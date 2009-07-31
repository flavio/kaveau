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

#include "addbackupwizardpage3.h"

#include "ui_addbackupwizardpage3view.h"

#include <kuser.h>

//solid specific includes
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/storageaccess.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtNetwork/QHostInfo>

#define MOUNTING_PAGE 0
#define CHOICE_PAGE 1
#define ERROR_PAGE 2
#define INFO_PAGE 3

AddBackupWizardPage3::AddBackupWizardPage3(QWidget* parent)
  : QWizardPage (parent)
{
  QWidget *widget = new QWidget(this);
  m_view = new Ui::AddBackupWizardPage3View();
  m_view->setupUi(widget);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(widget);
  setLayout(layout);

  m_view->labelIcon->setPixmap(KIconLoader::global()->loadIcon("dialog-error", KIconLoader::Desktop));

  // just for being sure
  m_view->btnErase->setChecked(false);

  setupConnections();
}

AddBackupWizardPage3::~AddBackupWizardPage3()
{
  delete m_view;
}

void AddBackupWizardPage3::initializePage()
{
  m_destination.clear();
  checkDeviceStatus();
}

void AddBackupWizardPage3::setupConnections()
{
  connect(m_view->btnGroup, SIGNAL(changed(int)), this, SLOT(slotBtnClicked()));
}

void AddBackupWizardPage3::slotBtnClicked()
{
  emit completeChanged();
}

bool AddBackupWizardPage3::isComplete () const
{
  if (m_view->stackedWidget->currentIndex() == CHOICE_PAGE)
    return (m_view->btnErase->isChecked()  || m_view->btnKeep->isChecked());
  else
    return true;
}

void AddBackupWizardPage3::calculateDestination(const QString& mount)
{
  KUser user;

  m_destination = mount;
  m_destination += QDir::separator();
  m_destination += "kaveau";
  m_destination += QDir::separator();
  m_destination += QHostInfo::localHostName();
  m_destination += QDir::separator();
  m_destination += user.loginName();
  m_destination = QDir::cleanPath(m_destination);
}

void AddBackupWizardPage3::slotSetupDone(Solid::ErrorType error,QVariant message,QString udi)
{
  if (error == Solid::NoError) {
    Solid::Device device (udi);
    Solid::StorageAccess* storageAccess = (Solid::StorageAccess*) device.asDeviceInterface(Solid::DeviceInterface::StorageAccess);

    QFileInfo info (storageAccess->filePath());

    if (info.isWritable()) {
      calculateDestination(storageAccess->filePath());
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

void AddBackupWizardPage3::verifyDestination()
{
  QDir destDir (m_destination);
  if (destDir.exists())
    m_view->stackedWidget->setCurrentIndex(CHOICE_PAGE);
  else {
    m_view->stackedWidget->setCurrentIndex(INFO_PAGE);
    m_view->labelInfo->setText(m_destination);
    emit completeChanged();
  }
}

void AddBackupWizardPage3::checkDeviceStatus()
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
        calculateDestination(storageAccess->filePath());
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

QString AddBackupWizardPage3::destination() const
{
  return m_destination;
}

QString AddBackupWizardPage3::deviceUDI() const
{
  return field("deviceUDI").toString();
}

bool AddBackupWizardPage3::eraseDestination() const
{
  return m_view->btnErase->isChecked();
}
