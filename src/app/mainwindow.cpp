/* This file is part of the kaveau project
 *
 * Copyright (C) 2009 Flavio Castelli <flavio@castelli.name>
 *
 * kaveau is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * kaveau is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Keep; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "mainwindow.h"

#include <kapplication.h>
#include <kaction.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <kdiskfreespaceinfo.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <knotification.h>
#include <kstandardaction.h>
#include <ksystemtrayicon.h>

#include <QtGui/QCloseEvent>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>

#include "ui_mainwidgetbase.h"
#include "excludeditemsdialog.h"
#include "addbackupwizard.h"
#include "logdialog.h"

#include "common.h"
#include "backupdevice.h"
#include "backupmanager.h"
#include "settings.h"

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

  m_backupDevice = new BackupDevice(this);
  m_backupManager = new BackupManager(this);

  setupActions();
  setupTrayIcon();
  setupConnections();

  if (!BackupManager::isBackupProgramAvailable()) {
    showGenericError(i18n("rdiff-backup is not installed"));
  } else if (m_backupDevice->isAvailable() && !m_backupDevice->isAccesible()) {
    m_backupDevice->setup();
  } else if (m_backupDevice->isAvailable() && m_backupDevice->isAccesible())
    backupIfNeeded();

  updateBackupView();
  updateDiskUsage("");
}

MainWindow::~MainWindow()
{
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
  connect (m_trayIcon, SIGNAL (quitSelected()), this, SLOT (close()));
}

void MainWindow::setupConnections()
{
  connect (m_backupManager, SIGNAL(backupFinished(bool,QString)), this, SLOT(slotBackupFinished(bool,QString)));

  connect (m_mainWidget->btnConfig, SIGNAL(clicked()), this, SLOT(slotStartBackupWizard()));
  connect (m_mainWidget->btnBackup, SIGNAL(clicked()), this, SLOT(slotStartBackup()));
  connect (m_mainWidget->btnFilter, SIGNAL(clicked()), this, SLOT(slotEditFilters()));
  connect (m_mainWidget->btnDetails, SIGNAL(clicked()), this, SLOT(slotShowLog()));

  connect (m_mainWidget->btnPurge, SIGNAL(clicked()), this, SLOT(slotPurgeOldBackups()));

  connect (m_backupDevice, SIGNAL(accessibilityChanged(bool)), this, SLOT(slotBackupDeviceAccessibilityChanged(bool)));
  connect (m_backupDevice, SIGNAL(newDeviceAttached()), this, SLOT(slotNewDeviceAttached()));
  connect (m_backupDevice, SIGNAL(setupDone(bool,QString)), this, SLOT(slotBackupDeviceSetupDone(bool,QString)));
  connect (m_backupDevice, SIGNAL(backupDirectoriesRemoved(bool,QString)), this, SLOT(slotOldBackupDirectoriesRemoved(bool,QString)));
}

void MainWindow::slotStartBackupWizard()
{
  AddBackupWizard wizard(this);
  wizard.exec();
  if (wizard.completed()) {
    *Settings::global() = wizard.settings();

    updateBackupView();

    if (wizard.deleteDestination()) {
      kDebug() << "Going to erase" << Settings::global()->dest();
      m_mainWidget->btnBackup->setEnabled(false);
      if (KIO::NetAccess::del(KUrl(Settings::global()->dest()), 0))
        m_backupDevice->createBackupDirectory();
      else {
        showGenericError(i18n("Unable to delete") + Settings::global()->dest(), true);
        m_mainWidget->btnBackup->setEnabled(false);
        return;
      }
    }

    backupIfNeeded();
  }
}

void MainWindow::slotEditFilters()
{
  Settings* settings = Settings::global();
  ExcludedItemsDialog dialog(settings->excludeList(), this);
  if (dialog.exec() == QDialog::Accepted) {
    QStringList excludedItems = dialog.excludedItems();
    settings->setExcludeList(excludedItems);
  }
}

bool MainWindow::queryClose()
{
  if (this->isVisible()) {
    hide();
    return false;
  }

  if (m_backupManager->isBackupRunning()) {
    KGuiItem continueBtn (KStandardGuiItem::yes());
    continueBtn.setText(i18n("Continue backup"));
    continueBtn.setToolTip(i18n("Continue the backup operation"));

    KGuiItem quitBtn (KStandardGuiItem::no());
    quitBtn.setText(i18n("Quit kaveau"));
    quitBtn.setToolTip(i18n("Abort the backup operation and exit"));

    if (KMessageBox::warningYesNo(this,
        i18n("A backup is in progress."),
        i18n("WARNING - backup running"),
        quitBtn, continueBtn) ==   KMessageBox::Yes)
    {
      return true;
    } else {
      return false;
    }
  } else {
    return true;
  }
}

void MainWindow::updateBackupView()
{
  Settings* settings = Settings::global();

  if (settings == 0) {
    m_mainWidget->stackedWidget->setCurrentIndex(CONFIGURE_PAGE);
    m_mainWidget->btnBackup->setEnabled(false);
    m_mainWidget->btnFilter->setEnabled(false);
    return;
  }

  m_mainWidget->labelSource->setText( settings->source());
  m_mainWidget->labelDest->setText( settings->dest());
  m_mainWidget->btnFilter->setEnabled(true);

  KIconLoader* iconLoader = KIconLoader::global();

  if (settings->lastBackupTime().isValid()) {
    QDateTime now = QDateTime::currentDateTime();
    int daysTo = settings->lastBackupTime().daysTo(now);

    if (daysTo > 7) {
      m_mainWidget->labelTime->setText(i18n("more than one week ago"));
      m_mainWidget->labelStatusIcon->setPixmap(iconLoader->loadIcon("security-low", KIconLoader::Small));
    } else if (daysTo > 0) {
      m_mainWidget->labelTime->setText(i18n("%1 day(s) ago").arg(daysTo));
      m_mainWidget->labelStatusIcon->setPixmap(iconLoader->loadIcon("security-medium", KIconLoader::Small));
    } else {
      m_mainWidget->labelTime->setText(i18n("Today at %1").arg(settings->lastBackupTime().toString("hh:mm")));
      m_mainWidget->labelStatusIcon->setPixmap(iconLoader->loadIcon("security-high", KIconLoader::Small));
    }
  } else {
    m_mainWidget->labelTime->setText(i18n("never"));
    m_mainWidget->labelStatusIcon->setPixmap(iconLoader->loadIcon("security-low", KIconLoader::Small));
  }

  if (m_backupDevice->isAvailable()) {
    if (m_backupDevice->isAccesible()) {
      m_mainWidget->labelDevice->setText (i18n("Connected"));
      m_mainWidget->btnBackup->setEnabled(true);
    } else {
      m_mainWidget->labelDevice->setText (i18n ("Not Accesible"));
      m_mainWidget->btnBackup->setEnabled(false);
      updateDiskUsage("");
    }
  } else {
    m_mainWidget->labelDevice->setText (i18n("Not connected"));
    m_mainWidget->btnBackup->setEnabled(false);
    updateDiskUsage("");
  }
}

void MainWindow::updateDiskUsage(const QString& mount)
{
  KDiskFreeSpaceInfo freeSpaceInfo = KDiskFreeSpaceInfo::freeSpaceInfo(mount);

  if (!freeSpaceInfo.isValid()) {
    m_mainWidget->diskSpaceBar->hide();
    m_mainWidget->diskSpaceLabel->hide();
  } else {
    m_mainWidget->diskSpaceBar->setMinimum(0);
    m_mainWidget->diskSpaceBar->setMaximum(freeSpaceInfo.size());
    m_mainWidget->diskSpaceBar->setValue(freeSpaceInfo.used());
    m_mainWidget->diskSpaceBar->show();
    m_mainWidget->diskSpaceLabel->setText(i18n("%1 over %2").arg(bytesToHuman(freeSpaceInfo.used())).arg(bytesToHuman(freeSpaceInfo.size())));
    m_mainWidget->diskSpaceLabel->show();
  }
}

void MainWindow::slotShowLog()
{
  LogDialog* dialog = new LogDialog(m_lastError, this);
  dialog->show();
}

void MainWindow::showGenericError(const QString& message, bool disableBackup)
{
  m_mainWidget->stackedWidget->setCurrentIndex(GENERIC_ERROR_PAGE);
  m_mainWidget->labelGenericError->setText(message);
  m_mainWidget->btnBackup->setEnabled(!disableBackup);
}

void MainWindow::backupIfNeeded()
{
  if (!m_backupDevice->isAvailable()) {
    m_mainWidget->labelNextBackup->setText(i18n("next time the backup disk will be plugged"));
    return;
  } else if (!m_backupDevice->isAccesible()) {
    m_backupDevice->setup();
    return;
  }

  QDateTime lastBackup = Settings::global()->lastBackupTime();
  QDateTime now = QDateTime::currentDateTime();

  if ((!lastBackup.isValid()) or (lastBackup.secsTo(now) > BACKUP_INTERVAL)) {
    // perform the backup immediately
    slotStartBackup();
  } else {
    // schedule the backup
    scheduleNextBackup(BACKUP_INTERVAL - lastBackup.secsTo(now));
  }
}

void MainWindow::slotStartBackup() {
  if ((!m_backupDevice->isAccesible()) || (m_backupManager->isBackupRunning()))
    return;

  m_mainWidget->stackedWidget->setCurrentIndex(DOING_BACKUP_PAGE);
  m_mainWidget->labelNextBackup->setText("-");
  m_mainWidget->btnBackup->setEnabled(false);

  m_backupManager->doBackup();

  KNotification* notification= new KNotification ( "backupStarted", this );
  notification->setText( i18n("Backup started"));
  notification->setFlags( KNotification::RaiseWidgetOnActivation);
  notification->sendEvent();
}

void MainWindow::slotBackupFinished(bool ok, QString message)
{
  KNotification* notification;

  if (ok) {
    kDebug() << "backup completed successfully";
    m_mainWidget->stackedWidget->setCurrentIndex(SUCCESS_PAGE);

    notification= new KNotification ( "backupSuccess", this );
    notification->setText( i18n("Backup successfully completed"));
    notification->setFlags( KNotification::RaiseWidgetOnActivation);

    QDateTime now = QDateTime::currentDateTime();
    Settings::global()->setLastBackupTime(now);
  }
  else {
    kDebug() << "error during backup:" << message;
    m_lastError = message;

    m_mainWidget->stackedWidget->setCurrentIndex(FAILURE_PAGE);

    notification= new KNotification ( "backupError", this );
    notification->setText( i18n("Backup failed"));
    notification->setFlags( KNotification::RaiseWidgetOnActivation);
  }

  notification->sendEvent();

  // schedule next backup
  scheduleNextBackup( BACKUP_INTERVAL );
  updateBackupView();
  updateDiskUsage(Settings::global()->mount());
}

void MainWindow::scheduleNextBackup(int whithinSeconds)
{
  QTimer::singleShot( whithinSeconds*1000, this, SLOT( slotStartBackup()));

  QDateTime nextRun = QDateTime::currentDateTime().addSecs(whithinSeconds);
  m_mainWidget->labelNextBackup->setText(nextRun.toString("hh:mm"));
}

void MainWindow::slotPurgeOldBackups()
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

void MainWindow::slotOldBackupDirectoriesRemoved(bool ok, QString message)
{
  if (!ok) {
    m_lastError = message;
    m_mainWidget->stackedWidget->setCurrentIndex(FAILURE_PAGE);
  }

  // schedule delete operation
  scheduleNextPurgeOperation ( BACKUP_INTERVAL*3);
  updateDiskUsage(Settings::global()->mount());
}

void MainWindow::scheduleNextPurgeOperation(int whithinSeconds)
{
  QTimer::singleShot(whithinSeconds * 1000, this, SLOT( slotPurgeOldBackups()));
}

void MainWindow::slotNewDeviceAttached()
{
  // we don't have a backup disk, maybe we can use this one
  KNotification *notify = new KNotification( "storageDeviceAttached", parentWidget() );
  notify->setText( QString( "An external storage device has been attached." ) );
  notify->setActions( i18n( "Use as backup device" ).split( ',' ) );
  connect( notify, SIGNAL( action1Activated() ), this , SLOT( slotStartBackupWizard()));
  notify->sendEvent();
  QTimer::singleShot( 10*1000, notify, SLOT( close() ) );
}

void MainWindow::slotBackupDeviceAccessibilityChanged(bool accessible)
{
  if (accessible)
    backupIfNeeded();

  updateBackupView();
}

void MainWindow::slotBackupDeviceSetupDone(bool ok, QString message)
{
  if (!ok)
    showGenericError(message);
  else
    updateDiskUsage(Settings::global()->mount());

  updateBackupView();
  backupIfNeeded();
  slotPurgeOldBackups();
}
