#ifndef DEVICEWIDGET_H
#define DEVICEWIDGET_H

#include "ui_devicewidgetview.h"

class DeviceWidget : public QWidget, private Ui::DeviceWidget {
  Q_OBJECT
  public:
    DeviceWidget(QWidget *parent = 0);

    QList<QTreeWidgetItem*> selectedItems();

  public slots:
    void slotRefresh();

  signals:
    void itemSelectionChanged();
};

#endif // DEVICEWIDGET_H
