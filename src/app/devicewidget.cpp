#include "devicewidget.h"

#include "common.h"

//solid specific includes
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>

DeviceWidget::DeviceWidget(QWidget *parent) :
    QWidget(parent){
    setupUi(this);

    slotRefresh();
}

QList<QTreeWidgetItem*> DeviceWidget::selectedItems() {
  return this->selectedItems();
}

void DeviceWidget::slotRefresh()
{
  devicesWidget->clear();

  QList<QTreeWidgetItem *> items;
  foreach (const Solid::Device &device, Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive, QString()))
  {
    Solid::StorageDrive* storage = (Solid::StorageDrive*) device.asDeviceInterface(Solid::DeviceInterface::StorageDrive);
    if ((storage->driveType() == Solid::StorageDrive::HardDisk) && ((storage->bus() == Solid::StorageDrive::Usb) || (storage->bus() == Solid::StorageDrive::Ieee1394))) {
      QTreeWidgetItem* deviceItem = new QTreeWidgetItem((QTreeWidget*)0,
                                                 QStringList(device.product()));
      items.append(deviceItem);

      foreach (const Solid::Device &volumeDevice, Solid::Device::listFromType(Solid::DeviceInterface::StorageVolume, device.udi())) {
        Solid::StorageVolume* storage = (Solid::StorageVolume*) volumeDevice.asDeviceInterface(Solid::DeviceInterface::StorageVolume);
        QStringList columns;
        columns << QString() << storage->fsType();
        if (storage->label().isEmpty())
          columns << i18n("Undefined");
        else
          columns << storage->label();

        columns << bytesToHuman(storage->size());

        // this column is not displayed
        columns << volumeDevice.udi();

        QTreeWidgetItem* item = new QTreeWidgetItem(deviceItem, columns);

        if ((storage->fsType() == "vfat") || (storage->fsType() == "ntfs"))
          item->setIcon(1,KIcon("security-low"));
        else if (storage->fsType().startsWith("ext"))
          item->setIcon(1,KIcon("security-high"));

        items.append(item);
      }
    }
  }

  devicesWidget->insertTopLevelItems(0, items);
}


