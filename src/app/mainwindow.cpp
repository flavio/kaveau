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

#include "ui_mainwidgetbase.h"
#include "addbackupdialog.h"
#include "addbackupwizard.h"
#include "backup.h"
#include "configmanager.h"
#include "backupthread.h"

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

  updateBackupView();

  m_mainWidget->progressBox->hide();

  m_backupThread = new BackupThread();
  connect (m_backupThread, SIGNAL(backupFinished(bool,QString)), this, SLOT(slotBackupFinished(bool, QString)));

  m_backupDiskPlugged = isBackupDiskPlugged();
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
  connect (m_mainWidget->btnConfig, SIGNAL(clicked()), this, SLOT(slotConfigBackup()));
  connect (m_mainWidget->btnBackup, SIGNAL(clicked()), this, SLOT(slotStartBackup()));

  Solid::DeviceNotifier* notifier = Solid::DeviceNotifier::instance();
  connect (notifier, SIGNAL(deviceAdded(QString)), this, SLOT(slotDeviceAdded(QString)));
  connect (notifier, SIGNAL(deviceRemoved(QString)), this, SLOT(slotDeviceRemoved(QString)));
}

void MainWindow::slotConfigBackup()
{
  AddBackupWizard wizard(this);
  wizard.exec();
  if (wizard.completed()) {
     ConfigManager::global()->setBackup(wizard.backup());
    updateBackupView();
  }

//  AddBackupDialog dialog(ConfigManager::global()->backup(), this);
//  if (dialog.exec() == QDialog::Accepted) {
//    ConfigManager::global()->setBackup(dialog.backup());
//    updateBackupView();
//  }
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

  if (backup == 0) {
    //TODO do something better
    slotConfigBackup();
    updateBackupView();
  } else {
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
  }
}

void MainWindow::slotStartBackup() {
  m_mainWidget->progressBox->show();
  m_backupThread->start();
  kDebug() << "backup started";

  KNotification* notification= new KNotification ( "backupStarted", this );
  notification->setText( i18n("Backup started"));
  notification->setFlags( KNotification::RaiseWidgetOnActivation);
  notification->sendEvent();
}

void MainWindow::slotBackupFinished(bool status, QString message)
{
  KNotification* notification;
  m_mainWidget->progressBox->hide();

  if (status) {
    kDebug() << "backup completed successfully";

    notification= new KNotification ( "backupSuccess", this );
    notification->setText( i18n("Backup successfully completed"));
    notification->setFlags( KNotification::RaiseWidgetOnActivation);
  }
  else {
    kDebug() << "error during backup:" << message;

    notification= new KNotification ( "backupError", this );
    notification->setText( i18n("Backup failed"));
    notification->setFlags( KNotification::RaiseWidgetOnActivation);
  }

  notification->sendEvent();
}

void MainWindow::slotDeviceAdded(QString udi)
{
  Solid::Device device(udi);

  Backup* backup = ConfigManager::global()->backup();

  if (!m_backupDiskPlugged && (device.isDeviceInterface(Solid::DeviceInterface::StorageDrive))) {
    Solid::StorageDrive* drive = (Solid::StorageDrive*) device.asDeviceInterface(Solid::DeviceInterface::StorageDrive);

    if ((drive->driveType() == Solid::StorageDrive::HardDisk) && ((drive->bus() == Solid::StorageDrive::Usb) || (drive->bus() == Solid::StorageDrive::Ieee1394))) {
      if (backup == 0) {
        KNotification *notify = new KNotification( "storageDeviceAttached", parentWidget() );
        notify->setText( QString( "An external storage device has been attached." ) );
        notify->setActions( i18n( "Use it with kaveau" ).split( ',' ) );
        connect( notify, SIGNAL( action1Activated() ), this , SLOT( slotConfigBackup()));
        notify->sendEvent();
        QTimer::singleShot( 10*1000, notify, SLOT( close() ) );
      } else if (backup->diskUdi() == udi) {
        // external disk used for backups has been attached
      }
    }
  }
}

bool MainWindow::isBackupDiskPlugged()
{
//  foreach (const Solid::Device &device, Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive, QString()))
//  {
//    Solid::StorageDrive storage = device.asDeviceInterface(Solid::DeviceInterface::StorageDrive);
//    if ((storage.bus() != Solid::StorageDrive::Usb) || (storage.bus() != Solid::StorageDrive::Ieee1394))
//      continue;
//    kDebug() << device.udi().toLatin1().constData() << device.vendor().toLatin1().constData() << device.product().toLatin1().constData();
//    kDebug() << storage.bus()
//  }

  return false;
}

void MainWindow::slotDeviceRemoved(QString udi)
{

}


