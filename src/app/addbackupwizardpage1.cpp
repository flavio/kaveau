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
  populateDeviceView();
}

AddBackupWizardPage1::~AddBackupWizardPage1()
{
  delete m_view;
}

void AddBackupWizardPage1::slotRefresh() {
  populateDeviceView();
}

QString bytesToHuman(qlonglong value) const
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

        items.append(new QTreeWidgetItem(deviceItem, columns));
      }
    }
  }

  m_view->devicesWidget->insertTopLevelItems(0, items);

}

