/* This file is part of the kaveau project
 *
 * Copyright (C) 2009 Flavio Castelli <flavio@castelli.name>
 *
 * kaveau is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * kaveau is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Keep; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "addbackupwizardpage1.h"

#include "ui_addbackupwizardpage1view.h"
#include "common.h"

#include <kdiskfreespaceinfo.h>
#include <QtCore/QList>
#include <QtGui/QTreeWidgetItem>

#include "devicewidget.h"

AddBackupWizardPage1::AddBackupWizardPage1(QWidget* parent)
    : QWizardPage (parent)
{
  QWidget *widget = new QWidget(this);
  m_view = new Ui::AddBackupWizardPage1View();
  m_view->setupUi(widget);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(widget);
  setLayout(layout);

  m_deviceWidget = new DeviceWidget(m_view->deviceFrame);

  connect (m_deviceWidget, SIGNAL(itemSelectionChanged()),
           this, SLOT(slotDeviceItemSelectionChanged()));

  // this item is used just for accessing the selected UDI from other pages
  m_view->selectedUDI->hide();
  registerField("deviceUDI", m_view->selectedUDI);
}

AddBackupWizardPage1::~AddBackupWizardPage1()
{
  delete m_view;
}

bool AddBackupWizardPage1::isComplete () const
{
  QList<QTreeWidgetItem*> items = m_deviceWidget->selectedItems();
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

  QString filesystem = items[0]->data(1,Qt::DisplayRole).toString();

  if ((filesystem == "vfat") || (filesystem == "ntfs")) {
    m_view->selectedUDI->clear();
    return false;
  } else {
    m_view->selectedUDI->setText(items[0]->data(4,Qt::DisplayRole).toString());
    return true;
  }
}

void AddBackupWizardPage1::slotDeviceItemSelectionChanged()
{
  emit completeChanged();
}

