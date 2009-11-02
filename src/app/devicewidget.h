#ifndef DEVICEWIDGET_H
#define DEVICEWIDGET_H

#include "ui_devicewidgetview.h"

namespace Solid {
  class Device;
  class StorageDrive;
}

class DeviceWidget : public QWidget, private Ui::DeviceWidget {
  Q_OBJECT
  public:
    DeviceWidget(QWidget *parent = 0);

    QList<QTreeWidgetItem*> selectedItems();

  public slots:
    void slotRefresh();

  private slots:
    void slotDeviceAdded(QString);
    void slotDeviceRemoved(QString);
    void slotCurrentItemChanged ( QTreeWidgetItem*, QTreeWidgetItem*);

  private:
    bool isStorageDeviceValid(Solid::StorageDrive* storage);
    void addDevice(const Solid::Device* device);

  signals:
    void itemSelectionChanged();

  private:
    QString m_selectedUdi;
};

#endif // DEVICEWIDGET_H
