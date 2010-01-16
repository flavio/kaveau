#include "kaveaudaemon.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>

#include <kdebug.h>
#include <klocale.h>
#include <knotification.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../utils/common.h"
#include "../utils/backupdevice.h"
#include "../utils/backupmanager.h"
#include "../utils/settings.h"

int KaveauDaemon::sighupFd[2]  = {0.0};
int KaveauDaemon::sigtermFd[2] = {0.0};

//TODO move to kconfig xt?
#define BACKUP_INTERVAL 3600 // backup every hour

KaveauDaemon::KaveauDaemon(QObject *parent)
  : QObject(parent)
{
  if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sighupFd))
    qFatal("Couldn't create HUP socketpair");

  if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd))
    qFatal("Couldn't create TERM socketpair");
       snHup = new QSocketNotifier(sighupFd[1], QSocketNotifier::Read, this);
       connect(snHup, SIGNAL(activated(int)), this, SLOT(handleSigHup()));
       snTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
       connect(snTerm, SIGNAL(activated(int)), this, SLOT(handleSigTerm()));

  m_backupDevice = new BackupDevice(this);
  m_backupManager = new BackupManager(this);

  setupConnections();

  if (!BackupManager::isBackupProgramAvailable())
    qFatal("rync is not installed");

  Settings* settings = Settings::global();
  if (settings->isBackupDeviceConfigured() && (m_backupDevice->isAvailable()))
   m_backupDevice->setup();
}

KaveauDaemon::~KaveauDaemon()
{
}

void KaveauDaemon::hupSignalHandler(int)
{
    char a = 1;
    ::write(sighupFd[0], &a, sizeof(a));
}

void KaveauDaemon::termSignalHandler(int)
{
    char a = 1;
    ::write(sigtermFd[0], &a, sizeof(a));
}

void KaveauDaemon::handleSigTerm()
{
   snTerm->setEnabled(false);
   char tmp;
   ::read(sigtermFd[1], &tmp, sizeof(tmp));

   // do Qt stuff
   qDebug() << "KaveauDaemon::handleSigTerm()";
   QCoreApplication::instance()->quit();
   snTerm->setEnabled(true);
}

void KaveauDaemon::handleSigHup()
{
   snHup->setEnabled(false);
   char tmp;
   ::read(sighupFd[1], &tmp, sizeof(tmp));

   // do Qt stuff
   qDebug() << "KaveauDaemon::handleSigHup()";

   snHup->setEnabled(true);
}

void KaveauDaemon::setupConnections()
{
  connect (m_backupManager, SIGNAL(backupFinished(bool,QString)),
           this, SLOT(slotBackupFinished(bool,QString)));

  //TODO create a new slot
//  connect (m_mainWidget->btnBackup, SIGNAL(clicked()),
//           this, SLOT(slotStartBackup()));

  connect (m_backupDevice, SIGNAL(accessibilityChanged(bool)),
           this, SLOT(slotBackupDeviceAccessibilityChanged(bool)));
  connect (m_backupDevice, SIGNAL(newDeviceAttached()),
           this, SLOT(slotNewDeviceAttached()));
  connect (m_backupDevice, SIGNAL(setupDone(bool,QString)),
           this, SLOT(slotBackupDeviceSetupDone(bool,QString)));
  connect (m_backupDevice, SIGNAL(backupDirectoriesRemoved(bool,QString)),
           this, SLOT(slotOldBackupDirectoriesRemoved(bool,QString)));
}

void KaveauDaemon::backupIfNeeded()
{
  if (!m_backupDevice->isAvailable()) {
    return;
  } else if (!m_backupDevice->isAccesible()) {
    m_backupDevice->setup();
    return;
  }

  QDateTime lastBackup = Settings::global()->lastBackupTime();
  QDateTime now = QDateTime::currentDateTime();

  if ((!lastBackup.isValid()) or (lastBackup.secsTo(now) > BACKUP_INTERVAL)) {
    // perform the backup immediately

    // ensure the destination directory exists
    m_backupDevice->createBackupDirectory();

    slotStartBackup();
  } else {
    // schedule the backup
    scheduleNextBackup(BACKUP_INTERVAL - lastBackup.secsTo(now));
  }
}

void KaveauDaemon::slotStartBackup() {
  if ((!m_backupDevice->isAccesible()) || (m_backupManager->isBackupRunning()))
    return;

  m_backupManager->doBackup();

  KNotification* notification= new KNotification ( "backupStarted" );
  notification->setText( i18n("Backup started"));
  notification->setFlags( KNotification::RaiseWidgetOnActivation);
  notification->sendEvent();
}

void KaveauDaemon::slotBackupFinished(bool ok, QString message)
{
  KNotification* notification;

  if (ok) {
    kDebug() << "backup completed successfully";

    notification= new KNotification ( "backupSuccess" );
    notification->setText( i18n("Backup successfully completed"));
    notification->setFlags( KNotification::RaiseWidgetOnActivation);

    QDateTime now = QDateTime::currentDateTime();
    Settings::global()->setLastBackupTime(now);
  }
  else {
    kDebug() << "error during backup:" << message;
    m_lastError = message;

    notification= new KNotification ( "backupError" );
    notification->setText( i18n("Backup failed"));
    notification->setFlags( KNotification::RaiseWidgetOnActivation);
  }

  notification->sendEvent();

  // schedule next backup
  scheduleNextBackup( BACKUP_INTERVAL );
}

void KaveauDaemon::scheduleNextBackup(int whithinSeconds)
{
  QTimer::singleShot( whithinSeconds*1000, this, SLOT( slotStartBackup()));
//  QDateTime nextRun = QDateTime::currentDateTime().addSecs(whithinSeconds);
}

void KaveauDaemon::slotPurgeOldBackups()
{
  if (!m_backupDevice->isAvailable()) {
    scheduleNextPurgeOperation ( BACKUP_INTERVAL*3);
    return;
  }
  else if (!m_backupDevice->isAccesible()) {
    m_backupDevice->setup();
    return;
  }

  QStringList oldBackups = m_backupManager->oldBackups();
  if (!oldBackups.isEmpty())
    m_backupDevice->removeBackupDirectories(oldBackups);
  else
    slotOldBackupDirectoriesRemoved(true, "");
}

void KaveauDaemon::slotOldBackupDirectoriesRemoved(bool ok, QString message)
{
  if (!ok)
    m_lastError = message;

  // schedule delete operation
  scheduleNextPurgeOperation ( BACKUP_INTERVAL*3 );
}

void KaveauDaemon::scheduleNextPurgeOperation(int whithinSeconds)
{
  QTimer::singleShot(whithinSeconds * 1000, this, SLOT( slotPurgeOldBackups()));
}

void KaveauDaemon::slotNewDeviceAttached()
{
  Settings* settings = Settings::global();
  if (settings->isBackupDeviceConfigured())
    return;

  // we don't have a backup disk, maybe we can use this one
  KNotification *notify = new KNotification( "storageDeviceAttached");
  notify->setText( QString( "An external storage device has been attached."));
  notify->setActions( i18n( "Use as backup device" ).split( ',' ));
  // TODO fix
//  connect( notify, SIGNAL( action1Activated() ),
//           this , SLOT( slotStartBackupWizard()));
  notify->sendEvent();
  QTimer::singleShot( 10*1000, notify, SLOT( close()));
}

void KaveauDaemon::slotBackupDeviceAccessibilityChanged(bool accessible)
{
  if (accessible)
    backupIfNeeded();
  else if (m_backupDevice->isAvailable())
    m_backupDevice->setup(); // remount the backup device
}

void KaveauDaemon::slotBackupDeviceSetupDone(bool ok, QString message)
{
  if (!ok) {
    //TODO: do something else
//    showGenericError(message);
  } else {
    backupIfNeeded();
    slotPurgeOldBackups();
  }
}
