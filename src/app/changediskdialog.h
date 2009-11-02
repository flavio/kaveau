#ifndef CHANGEDISKDIALOG_H
#define CHANGEDISKDIALOG_H

#include "ui_changediskdialog.h"

class DeviceWidget;

class ChangeDiskDialog : public QDialog, private Ui::ChangeDiskDialog {
  Q_OBJECT

  public:
    ChangeDiskDialog(QWidget *parent = 0);
    QString selectedUDI() const;

  private slots:
    void slotDeviceItemSelectionChanged();

  private:
    DeviceWidget* m_deviceWidget;
    QString m_selectedUDI;
};

#endif // CHANGEDISKDIALOG_H
