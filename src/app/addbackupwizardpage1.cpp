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

#include "addbackupwizardpage1.h"

#include "ui_addbackupwizardpage1view.h"

#include <QtCore/QList>
#include <QtGui/QTreeWidgetItem>

//solid specific includes
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>

AddBackupWizardPage1::AddBackupWizardPage1(QWidget* parent)
    : QWizardPage (parent)
{
  QWidget *widget = new QWidget(this);
  m_view = new Ui::AddBackupWizardPage1View();
  m_view->setupUi(widget);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(widget);
  setLayout(layout);

  connect (m_view->btnRefresh, SIGNAL(clicked()), this, SLOT(slotRefresh()));
  connect (m_view->devicesWidget, SIGNAL(itemSelectionChanged()), this, SLOT(slotDeviceItemSelectionChanged()));
  populateDeviceView();

  // this item is used just for accessing the selected UDI from other pages
  m_view->selectedUDI->hide();
  registerField("deviceUDI", m_view->selectedUDI);
}

AddBackupWizardPage1::~AddBackupWizardPage1()
{
  delete m_view;
}

void AddBackupWizardPage1::slotRefresh() {
  populateDeviceView();
}

QString bytesToHuman(qlonglong value)
{

}

void AddBackupWizardPage1::populateDeviceView()
{
  m_view->devicesWidget->clear();

  QList<QTreeWidgetItem *> items;
  foreach (const Solid::Device &device, Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive, QString()))
  {
    Solid::StorageDrive* storage = (Solid::StorageDrive*) device.asDeviceInterface(Solid::DeviceInterface::StorageDrive);
    if ((storage->driveType() == Solid::StorageDrive::HardDisk) && ((storage->bus() == Solid::StorageDrive::Usb) || (storage->bus() == Solid::StorageDrive::Ieee1394))) {
      QTreeWidgetItem* deviceItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(device.product()));
      items.append(deviceItem);

      foreach (const Solid::Device &volumeDevice, Solid::Device::listFromType(Solid::DeviceInterface::StorageVolume, device.udi())) {
        Solid::StorageVolume* storage = (Solid::StorageVolume*) volumeDevice.asDeviceInterface(Solid::DeviceInterface::StorageVolume);
        QStringList columns;
        columns << QString() << storage->fsType();
        if (storage->label().isEmpty())
          columns << i18n("Volume");
        else
          columns << storage->label();

        QString size;
        size.setNum(storage->size());
        columns << size;

        // this column is not displayed
        columns << volumeDevice.udi();

        items.append(new QTreeWidgetItem(deviceItem, columns));
      }
    }
  }

  m_view->devicesWidget->insertTopLevelItems(0, items);

}

bool AddBackupWizardPage1::isComplete () const
{
  QList<QTreeWidgetItem*> items = m_view->devicesWidget->selectedItems();
  if (items.isEmpty()) {
    m_view->selectedUDI->clear();
    return false;
  } else {
    foreach (QTreeWidgetItem* item, items) {
      if (item->parent() == 0) {
        m_view->selectedUDI->clear();
        return false;
      }
    }
  }

  m_view->selectedUDI->setText(items[0]->data(4,Qt::DisplayRole).toString());
  return true;
}

void AddBackupWizardPage1::slotDeviceItemSelectionChanged()
{
  emit completeChanged();
}

