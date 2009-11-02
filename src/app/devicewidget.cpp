#include "devicewidget.h"

#include "common.h"

//solid specific includes
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>

using namespace Solid;

DeviceWidget::DeviceWidget(QWidget *parent) :
    QWidget(parent){
    setupUi(this);

    slotRefresh();

    DeviceNotifier* notifier = DeviceNotifier::instance();
    connect (notifier, SIGNAL(deviceAdded(QString)),
             this, SLOT(slotDeviceAdded(QString)));
    connect (notifier, SIGNAL(deviceRemoved(QString)),
             this, SLOT(slotDeviceRemoved(QString)));

    connect (devicesWidget,
             SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
             this,
             SLOT(slotCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

    connect (devicesWidget,
             SIGNAL(itemSelectionChanged()),
             this,
             SLOT(slotItemSelectionChanged()));
}

QList<QTreeWidgetItem*> DeviceWidget::selectedItems() {
  return devicesWidget->selectedItems();
}

void DeviceWidget::slotRefresh()
{
  devicesWidget->clear();

  foreach (const Device &device, Device::listFromType(DeviceInterface::StorageDrive, QString()))
  {
    StorageDrive* storage = (StorageDrive*) device.asDeviceInterface(DeviceInterface::StorageDrive);
    if (isStorageDeviceValid(storage))
      addDevice(&device);
  }
}

void DeviceWidget::slotDeviceAdded(QString udi)
{
  Q_UNUSED(udi)
  slotRefresh();
}

void DeviceWidget::slotDeviceRemoved(QString udi)
{
  Q_UNUSED(udi)
  slotRefresh();
}

bool DeviceWidget::isStorageDeviceValid(StorageDrive* storage)
{
  if (storage == 0) {
    return false;
  } else {
    return (storage->driveType() == StorageDrive::HardDisk) &&
              ((storage->bus() == StorageDrive::Usb) ||
               (storage->bus() == StorageDrive::Ieee1394));
  }
}

void DeviceWidget::slotCurrentItemChanged(QTreeWidgetItem* curr,QTreeWidgetItem* prev)
{
  Q_UNUSED(prev)
  if (curr != 0)
    m_selectedUdi = curr->data(4, Qt::DisplayRole).toString();
}

void DeviceWidget::slotItemSelectionChanged()
{
  emit itemSelectionChanged();
}

void DeviceWidget::addDevice(const Device* device)
{
  QList<QTreeWidgetItem *> items;
  QTreeWidgetItem* itemToSelect = 0;
  QTreeWidgetItem* deviceItem = new QTreeWidgetItem((QTreeWidget*)0,
                                                QStringList(device->product()));
  items.append(deviceItem);

  foreach (const Device &volumeDevice, Device::listFromType(DeviceInterface::StorageVolume, device->udi())) {
    StorageVolume* storage = (StorageVolume*) volumeDevice.asDeviceInterface(DeviceInterface::StorageVolume);

    if (storage == 0)
      continue;
    
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

    if (volumeDevice.udi() == m_selectedUdi)
      itemToSelect = item;

    if ((storage->fsType() == "vfat") || (storage->fsType() == "ntfs"))
      item->setIcon(1,KIcon("security-low"));
    else if (storage->fsType().startsWith("ext"))
      item->setIcon(1,KIcon("security-high"));

    items.append(item);
  }

  devicesWidget->insertTopLevelItems(0, items);
  if (itemToSelect != 0)
    devicesWidget->setCurrentItem(itemToSelect);
}
