/* This file is part of kaveau
 *
 * Copyright (C) 2009 Flavio Castelli <flavio@castelli.name>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with kaveau; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "mainwindow.h"

#include <QtGui/QCloseEvent>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <KApplication>
#include <KAction>
#include <KDebug>
#include <KLocale>
#include <KActionCollection>
#include <KIconLoader>
#include <KNotification>
#include <KStandardAction>
#include <KSystemTrayIcon>

//solid specific includes
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/storagedrive.h>
#include <solid/storageaccess.h>

#include "ui_mainwidgetbase.h"
#include "addbackupdialog.h"
#include "addbackupwizard.h"
#include "backup.h"
#include "configmanager.h"
#include "rdiffmanager.h"
#include "backupthread.h"


#define BACKUP_INTERVAL 3600 // backup every hour

// stacked widget pages
#define CONFIGURE_PAGE 0
#define DOING_BACKUP_PAGE 1
#define SUCCESS_PAGE 2
#define FAILURE_PAGE 3
#define GENERIC_ERROR_PAGE 4

MainWindow::MainWindow(QWidget *parent)
  : KXmlGuiWindow(parent)
{
  QWidget* widget = new QWidget( this );
  m_mainWidget = new Ui::MainWidgetBase();
  m_mainWidget->setupUi(widget);

  setCentralWidget(widget);

  m_mainWidget->labelDiskIcon->setPixmap(KIcon("drive-removable-media").pixmap(128,128));

  setupActions();
  setupTrayIcon();
  setupConnections();

  m_backupThread = new BackupThread();
  connect (m_backupThread, SIGNAL(backupFinished(bool,QString)), this, SLOT(slotBackupFinished(bool, QString)));

  m_backupDiskPlugged = isBackupDiskPlugged();

  if (!isRdiffAvailable()) {
    showGenericError(i18n("rdiff-backup is not installed"));
  } else if (m_backupDiskPlugged)
    mountBackupPartition();

  updateBackupView();
}

MainWindow::~MainWindow()
{
  delete m_backupThread;
}

void MainWindow::setupActions()
{
  KStandardAction::quit(this, SLOT(close()), actionCollection());

  setupGUI();
}

void MainWindow::setupTrayIcon()
{
  m_trayIcon = new KSystemTrayIcon(KIcon("kaveau"), this);
  m_trayIcon->show();
}

void MainWindow::setupConnections()
{
  connect (m_mainWidget->btnConfig, SIGNAL(clicked()), this, SLOT(slotStartBackupWizard()));
  connect (m_mainWidget->btnBackup, SIGNAL(clicked()), this, SLOT(slotStartBackup()));

  Solid::DeviceNotifier* notifier = Solid::DeviceNotifier::instance();
  connect (notifier, SIGNAL(deviceAdded(QString)), this, SLOT(slotDeviceAdded(QString)));
  connect (notifier, SIGNAL(deviceRemoved(QString)), this, SLOT(slotDeviceRemoved(QString)));
}

void MainWindow::slotStartBackupWizard()
{
  AddBackupWizard wizard(this);
  wizard.exec();
  if (wizard.completed()) {
     ConfigManager::global()->setBackup(wizard.backup());
    updateBackupView();
  }
}

 void MainWindow::closeEvent(QCloseEvent *event)
 {
   //TODO: restore later
//   if (m_trayIcon->isVisible()) {
//     hide();
//     event->ignore();
//   }
   if (m_backupThread->isRunning()) {
     // TODO do something nicer!
     m_backupThread->terminate();
   }
 }

void MainWindow::updateBackupView()
{
  ConfigManager* backupManager = ConfigManager::global();
  Backup* backup = backupManager->backup();

  if (backup == 0)
    return;

  m_mainWidget->labelSource->setText( backup->source());
  m_mainWidget->labelDest->setText( backup->dest());
  KIconLoader* iconLoader = KIconLoader::global();

  if (backup->lastBackupTime().isValid()) {
    QDateTime now = QDateTime::currentDateTime();
    int daysTo = backup->lastBackupTime().daysTo(now);

    if (daysTo > 7) {
      m_mainWidget->labelTime->setText(i18n("more than one week ago"));
      m_mainWidget->labelStatusIcon->setPixmap(iconLoader->loadIcon("security-low", KIconLoader::Small));
    } else if (daysTo > 0) {
      m_mainWidget->labelTime->setText(i18n("%1 day(s) ago").arg(daysTo));
      m_mainWidget->labelStatusIcon->setPixmap(iconLoader->loadIcon("security-medium", KIconLoader::Small));
    } else {
      m_mainWidget->labelTime->setText(i18n("Today at %1").arg(backup->lastBackupTime().toString("hh:mm")));
      m_mainWidget->labelStatusIcon->setPixmap(iconLoader->loadIcon("security-high", KIconLoader::Small));
    }
  } else {
    m_mainWidget->labelTime->setText(i18n("never"));
    m_mainWidget->labelStatusIcon->setPixmap(iconLoader->loadIcon("security-low", KIconLoader::Small));
  }

  if (m_backupDiskPlugged)
    m_mainWidget->labelDevice->setText (i18n("Connected"));
  else
    m_mainWidget->labelDevice->setText (i18n("Not connected"));
}

void MainWindow::slotStartBackup() {
  if (!m_backupDiskPlugged)
    return;

  if (m_backupThread->isRunning()) {
    QTimer::singleShot( BACKUP_INTERVAL*1000, this, SLOT( slotStartBackup()));
    return;
  }

  m_mainWidget->stackedWidget->setCurrentIndex(DOING_BACKUP_PAGE);
  m_mainWidget->labelNextBackup->setText("-");

  m_backupThread->start();

  KNotification* notification= new KNotification ( "backupStarted", this );
  notification->setText( i18n("Backup started"));
  notification->setFlags( KNotification::RaiseWidgetOnActivation);
  notification->sendEvent();
}

void MainWindow::slotBackupFinished(bool status, QString message)
{
  KNotification* notification;

  if (status) {
    kDebug() << "backup completed successfully";
    m_mainWidget->stackedWidget->setCurrentIndex(SUCCESS_PAGE);

    notification= new KNotification ( "backupSuccess", this );
    notification->setText( i18n("Backup successfully completed"));
    notification->setFlags( KNotification::RaiseWidgetOnActivation);

    QDateTime now = QDateTime::currentDateTime();
    ConfigManager::global()->backup()->setLastBackupTime(now);
  }
  else {
    kDebug() << "error during backup:" << message;

    m_mainWidget->stackedWidget->setCurrentIndex(FAILURE_PAGE);

    notification= new KNotification ( "backupError", this );
    notification->setText( i18n("Backup failed"));
    notification->setFlags( KNotification::RaiseWidgetOnActivation);
  }

  notification->sendEvent();

  // schedule next backup
  scheduleNextBackup( BACKUP_INTERVAL );
  updateBackupView();
}

void MainWindow::slotDeviceAdded(QString udi)
{
  Solid::Device device(udi);

  Backup* backup = ConfigManager::global()->backup();

  if (m_backupDiskPlugged)
    return;

  if ((backup != 0) && (backup->diskUdi().compare(udi, Qt::CaseSensitive) == 0)) {
    m_mainWidget->labelDevice->setText (i18n ("Connected"));
    m_backupDiskPlugged = true;
    mountBackupPartition();
  } else if ((backup == 0)  && (device.isDeviceInterface(Solid::DeviceInterface::StorageDrive))) {

    Solid::StorageDrive* drive = (Solid::StorageDrive*) device.asDeviceInterface(Solid::DeviceInterface::StorageDrive);

    if ((drive->driveType() == Solid::StorageDrive::HardDisk) && ((drive->bus() == Solid::StorageDrive::Usb) || (drive->bus() == Solid::StorageDrive::Ieee1394))) {
      KNotification *notify = new KNotification( "storageDeviceAttached", parentWidget() );
      notify->setText( QString( "An external storage device has been attached." ) );
      notify->setActions( i18n( "Use it with kaveau" ).split( ',' ) );
      connect( notify, SIGNAL( action1Activated() ), this , SLOT( slotStartBackupWizard()));
      notify->sendEvent();
      QTimer::singleShot( 10*1000, notify, SLOT( close() ) );
    }
  }
}

void MainWindow::slotDeviceRemoved(QString udi)
{
  if (ConfigManager::global()->backup()->diskUdi().compare(udi,Qt::CaseSensitive) == 0) {
    m_mainWidget->labelDevice->setText (i18n ("Not Connected"));
    m_mainWidget->btnBackup->setEnabled(false);
    m_backupDiskPlugged = false;
  }
}


void MainWindow::backupIfNeeded()
{
  if (!m_backupDiskPlugged) {
    m_mainWidget->labelNextBackup->setText(i18n("next time the backup disk will be plugged"));
    return;
  }

  Backup* backup = ConfigManager::global()->backup();
  QDateTime lastBackup = backup->lastBackupTime();
  QDateTime now = QDateTime::currentDateTime();

  if ((!lastBackup.isValid()) or (lastBackup.secsTo(now) > BACKUP_INTERVAL)) {
    // perform the backup immediately
    slotStartBackup();
  } else {
    // schedule the backup
    scheduleNextBackup(BACKUP_INTERVAL - lastBackup.secsTo(now));
  }
}

void MainWindow::scheduleNextBackup(int whithinSeconds)
{
  QTimer::singleShot( whithinSeconds*1000, this, SLOT( slotStartBackup()));

  QDateTime nextRun = QDateTime::currentDateTime().addSecs(whithinSeconds);
  m_mainWidget->labelNextBackup->setText(nextRun.toString("hh:mm"));
}

bool MainWindow::isBackupDiskPlugged()
{
  Backup* backup = ConfigManager::global()->backup();

  if (backup == 0)
    return false;

  Solid::Device device (backup->diskUdi());
  if (device.isValid())
    return true;
  else
    return false;
}

void MainWindow::mountBackupPartition()
{
  Solid::Device device (ConfigManager::global()->backup()->diskUdi());
  Solid::StorageAccess* storageAccess = (Solid::StorageAccess*) device.asDeviceInterface(Solid::DeviceInterface::StorageAccess);

  if (storageAccess->isAccessible()) {
    slotBackupPartitionMounted(Solid::NoError, QVariant(), ConfigManager::global()->backup()->diskUdi());
    return;
  }

  connect(storageAccess, SIGNAL(setupDone(Solid::ErrorType,QVariant,QString)), this, SLOT(slotBackupPartitionMounted(Solid::ErrorType,QVariant,QString)));
  storageAccess->setup();
}

void MainWindow::slotBackupPartitionMounted(Solid::ErrorType error,QVariant message,QString udi)
{
  Q_UNUSED(message)

  if (error == Solid::NoError) {
    Solid::Device device (udi);
    Solid::StorageAccess* storageAccess = (Solid::StorageAccess*) device.asDeviceInterface(Solid::DeviceInterface::StorageAccess);

    QFileInfo info (storageAccess->filePath());

    if (info.isWritable()) {
      backupIfNeeded();
    } else {
      showGenericError(i18n("No write permission on the backup directory"));
    }
  }
  else {
    showGenericError(i18n("unable to mount backup partition"));
  }
}

bool MainWindow::isRdiffAvailable()
{
  RdiffManager manager;
  return manager.isRdiffAvailable();
}

void MainWindow::showGenericError(const QString& message, bool disableBackup)
{
  m_mainWidget->stackedWidget->setCurrentIndex(GENERIC_ERROR_PAGE);
  m_mainWidget->labelGenericError->setText(message);
  m_mainWidget->btnBackup->setEnabled(!disableBackup);
}

