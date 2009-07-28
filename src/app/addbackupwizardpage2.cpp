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

#include <kuser.h>

#include "addbackupwizardpage2.h"

#include "ui_addbackupwizardpage2view.h"

AddBackupWizardPage2::AddBackupWizardPage2(QWidget* parent)
  : QWizardPage (parent)
{
  KUser user;
  QWidget *widget = new QWidget(this);
  m_view = new Ui::AddBackupWizardPage2View();
  m_view->setupUi(widget);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(widget);
  setLayout(layout);

  m_view->excludeURL->setMode(KFile::Directory | KFile::ExistingOnly);
  m_view->excludeURL->setPath(user.homeDir());

  setupConnections();
}

AddBackupWizardPage2::~AddBackupWizardPage2()
{
  delete m_view;
}

void AddBackupWizardPage2::setupConnections()
{
  connect (m_view->btnExclude, SIGNAL(clicked()), this, SLOT(slotBtnExcludeClicked()));
  connect (m_view->btnRemove, SIGNAL(clicked()), this, SLOT(slotBtnRemoveClicked()));

  connect (m_view->excludeURL, SIGNAL(textChanged(QString)), this, SLOT(slotExcludeChanged()));
  connect (m_view->excludeURL, SIGNAL(urlSelected(KUrl)), this, SLOT(slotExcludeChanged()));

  connect (m_view->excludedItems, SIGNAL( itemSelectionChanged()), this, SLOT(sloExcludedItemsSelectionChanged()));
}

void AddBackupWizardPage2::slotBtnExcludeClicked()
{
  QString item = m_view->excludeURL->text();

  if (m_view->excludedItems->findItems(item, Qt::MatchExactly).isEmpty())
    m_view->excludedItems->addItem(item);

  m_view->btnExclude->setEnabled(false);
  m_view->excludeURL->clear();
}

void AddBackupWizardPage2::slotBtnRemoveClicked()
{
  QList<QListWidgetItem *> selectedItems = m_view->excludedItems->selectedItems ();

  foreach (QListWidgetItem* item, selectedItems) {
    m_view->excludedItems->takeItem (m_view->excludedItems->row(item));
  }

  m_view->btnRemove->setEnabled(!m_view->excludedItems->selectedItems().isEmpty());
}

void AddBackupWizardPage2::slotExcludeChanged()
{
  m_view->btnExclude->setEnabled(!m_view->excludeURL->text().isEmpty());
}

void AddBackupWizardPage2::sloExcludedItemsSelectionChanged()
{
  QList<QListWidgetItem *> selectedItems = m_view->excludedItems->selectedItems ();
  m_view->btnRemove->setEnabled(!selectedItems.isEmpty());
}

QStringList AddBackupWizardPage2::excludedDirs()
{
  QStringList items;
  for (int i = 0; i < m_view->excludedItems->count(); i++) {
    QListWidgetItem* item = m_view->excludedItems->item(i);
    items << item->data(Qt::DisplayRole).toString();
  }

  return items;
}

