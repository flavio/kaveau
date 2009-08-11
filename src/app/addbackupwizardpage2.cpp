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

#include "addbackupwizardpage2.h"

#include "ui_addbackupwizardpage2view.h"
#include "common.h"

#include <kdiskfreespaceinfo.h>

//solid specific includes
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/storageaccess.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#define MOUNTING_PAGE 0
#define CHOICE_PAGE 1
#define ERROR_PAGE 2
#define INFO_PAGE 3

AddBackupWizardPage2::AddBackupWizardPage2(QWidget* parent)
  : QWizardPage (parent)
{
  QWidget *widget = new QWidget(this);
  m_view = new Ui::AddBackupWizardPage2View();
  m_view->setupUi(widget);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(widget);
  setLayout(layout);

  m_view->labelIcon->setPixmap(KIconLoader::global()->loadIcon("dialog-error", KIconLoader::Desktop));

  // just for being sure
  m_view->btnErase->setChecked(false);

  setupConnections();
}

AddBackupWizardPage2::~AddBackupWizardPage2()
{
  delete m_view;
}

void AddBackupWizardPage2::initializePage()
{
  m_destination.clear();
  checkDeviceStatus();
}

void AddBackupWizardPage2::setupConnections()
{
  connect(m_view->btnGroup, SIGNAL(changed(int)), this, SLOT(slotBtnClicked()));
}

void AddBackupWizardPage2::slotBtnClicked()
{
  emit completeChanged();
}

bool AddBackupWizardPage2::isComplete () const
{
  if (m_view->stackedWidget->currentIndex() == CHOICE_PAGE)
    return (m_view->btnErase->isChecked()  || m_view->btnKeep->isChecked());
  else
    return true;
}

void AddBackupWizardPage2::slotSetupDone(Solid::ErrorType error,QVariant message,QString udi)
{
  if (error == Solid::NoError) {
    Solid::Device device (udi);
    Solid::StorageAccess* storageAccess = (Solid::StorageAccess*) device.asDeviceInterface(Solid::DeviceInterface::StorageAccess);

    QFileInfo info (storageAccess->filePath());

    if (info.isWritable()) {
      m_mount = storageAccess->filePath();
      m_destination = calculateBackupDestination(m_mount);
      verifyDestination();
    } else {
      m_view->stackedWidget->setCurrentIndex(ERROR_PAGE);
      m_view->labelMessage->setText(i18n("%1 is not writable").arg(storageAccess->filePath()));
    }
  }
  else {
    m_view->stackedWidget->setCurrentIndex(ERROR_PAGE);
    m_view->labelMessage->setText(message.toString());
    m_view->labelIcon->setPixmap(KIconLoader::global()->loadIcon("security-low", KIconLoader::Small));
  }
}

void AddBackupWizardPage2::verifyDestination()
{
  QDir destDir (m_destination);
  if (destDir.exists())
    m_view->stackedWidget->setCurrentIndex(CHOICE_PAGE);
  else {
    m_view->stackedWidget->setCurrentIndex(INFO_PAGE);
    m_view->labelInfo->setText(m_destination);

    KDiskFreeSpaceInfo info = KDiskFreeSpaceInfo::freeSpaceInfo(m_destination);
    m_view->labelDiskSpace->setText(QString("%1 / %2").arg(bytesToHuman(info.used())).arg(bytesToHuman(info.size())));
    m_view->diskSpaceBar->setMaximum(info.size());
    m_view->diskSpaceBar->setValue(info.used());
    emit completeChanged();
  }
}

void AddBackupWizardPage2::checkDeviceStatus()
{
  QString deviceUDI = field("deviceUDI").toString();

  Solid::Device device (deviceUDI);

  Solid::StorageAccess* storageAccess = (Solid::StorageAccess*) device.asDeviceInterface(Solid::DeviceInterface::StorageAccess);

  if (!device.isValid()) {
    m_view->stackedWidget->setCurrentIndex(ERROR_PAGE);
    if (deviceUDI.isEmpty())
      m_view->labelMessage->setText(i18n("Something went wrong during the wizard!"));
    else
      m_view->labelMessage->setText(i18n("%1 is not a valid storage device!").arg(deviceUDI));
  } else {
    if (storageAccess->isAccessible()) {
      QFileInfo info (storageAccess->filePath());

      if (info.isWritable()) {
        m_mount = storageAccess->filePath();
        m_destination = calculateBackupDestination(m_mount);
        verifyDestination();
      } else {
        m_view->stackedWidget->setCurrentIndex(ERROR_PAGE);
        m_view->labelMessage->setText(i18n("%1 is not writable").arg(storageAccess->filePath()));
      }
    } else {
      m_view->stackedWidget->setCurrentIndex(MOUNTING_PAGE);
      connect (storageAccess, SIGNAL(setupDone(Solid::ErrorType,QVariant,QString)), this, SLOT (slotSetupDone(Solid::ErrorType,QVariant,QString)));
      if (!storageAccess->setup()) {
        // mount operation is not permitted
        m_view->stackedWidget->setCurrentIndex(ERROR_PAGE);
        m_view->labelMessage->setText(i18n("Mount operation not permitted"));
      }
    }
  }
}

QString AddBackupWizardPage2::destination() const
{
  return m_destination;
}

QString AddBackupWizardPage2::relativeDestination() const
{
  return m_relative;
}

QString AddBackupWizardPage2::deviceUDI() const
{
  return field("deviceUDI").toString();
}

bool AddBackupWizardPage2::eraseDestination() const
{
  return m_view->btnErase->isChecked();
}
