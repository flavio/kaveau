#include "changediskdialog.h"

#include "devicewidget.h"

#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>

ChangeDiskDialog::ChangeDiskDialog(QWidget *parent) :
    QDialog(parent)
{
  setupUi(this);

  m_deviceWidget = new DeviceWidget(deviceFrame);

  connect (m_deviceWidget, SIGNAL(itemSelectionChanged()),
           this, SLOT(slotDeviceItemSelectionChanged()));

  m_deviceWidget->slotRefresh();
}

QString ChangeDiskDialog::selectedUDI() const
{
  return m_selectedUDI;
}

void ChangeDiskDialog::slotDeviceItemSelectionChanged()
{
  QPushButton* okButton = buttonBox->button(QDialogButtonBox::Ok);

  if (okButton == 0)
    return;

  QList<QTreeWidgetItem*> items = m_deviceWidget->selectedItems();
  if (items.isEmpty()) {
    okButton->setEnabled(false);
    m_selectedUDI.clear();
  } else {
    foreach (QTreeWidgetItem* item, items) {
      if (item->parent() == 0) {
        okButton->setEnabled(false);
        m_selectedUDI.clear();
        break;
      }
    }
  }

  QString filesystem = items[0]->data(1,Qt::DisplayRole).toString();

  if ((filesystem == "vfat") || (filesystem == "ntfs")) {
    okButton->setEnabled(false);
    m_selectedUDI.clear();
  } else {
    m_selectedUDI = items[0]->data(4,Qt::DisplayRole).toString();
    okButton->setEnabled(true);
  }
}
