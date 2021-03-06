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

#include <kuser.h>

#include "addbackupwizardpage3.h"

#include "ui_addbackupwizardpage3view.h"

AddBackupWizardPage3::AddBackupWizardPage3(QWidget* parent)
  : QWizardPage (parent)
{
  KUser user;
  QWidget *widget = new QWidget(this);
  m_view = new Ui::AddBackupWizardPage3View();
  m_view->setupUi(widget);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(widget);
  setLayout(layout);

  m_view->excludeURL->setMode(KFile::Directory | KFile::ExistingOnly);
  m_view->excludeURL->setPath(user.homeDir());

  setupConnections();
}

AddBackupWizardPage3::~AddBackupWizardPage3()
{
  delete m_view;
}

void AddBackupWizardPage3::setupConnections()
{
  connect (m_view->btnExclude, SIGNAL(clicked()), this, SLOT(slotBtnExcludeClicked()));
  connect (m_view->btnRemove, SIGNAL(clicked()), this, SLOT(slotBtnRemoveClicked()));

  connect (m_view->excludeURL, SIGNAL(textChanged(QString)), this, SLOT(slotExcludeChanged()));
  connect (m_view->excludeURL, SIGNAL(urlSelected(KUrl)), this, SLOT(slotExcludeChanged()));

  connect (m_view->excludedItems, SIGNAL( itemSelectionChanged()), this, SLOT(sloExcludedItemsSelectionChanged()));
}

void AddBackupWizardPage3::slotBtnExcludeClicked()
{
  QString item = m_view->excludeURL->text();

  if (m_view->excludedItems->findItems(item, Qt::MatchExactly).isEmpty())
    m_view->excludedItems->addItem(item);

  m_view->btnExclude->setEnabled(false);
  m_view->excludeURL->clear();
}

void AddBackupWizardPage3::slotBtnRemoveClicked()
{
  QList<QListWidgetItem *> selectedItems = m_view->excludedItems->selectedItems ();

  foreach (QListWidgetItem* item, selectedItems) {
    m_view->excludedItems->takeItem (m_view->excludedItems->row(item));
  }

  m_view->btnRemove->setEnabled(!m_view->excludedItems->selectedItems().isEmpty());
}

void AddBackupWizardPage3::slotExcludeChanged()
{
  m_view->btnExclude->setEnabled(!m_view->excludeURL->text().isEmpty());
}

void AddBackupWizardPage3::sloExcludedItemsSelectionChanged()
{
  QList<QListWidgetItem *> selectedItems = m_view->excludedItems->selectedItems ();
  m_view->btnRemove->setEnabled(!selectedItems.isEmpty());
}

QStringList AddBackupWizardPage3::excludedDirs()
{
  QStringList items;
  for (int i = 0; i < m_view->excludedItems->count(); i++) {
    QListWidgetItem* item = m_view->excludedItems->item(i);
    items << item->data(Qt::DisplayRole).toString();
  }

  return items;
}

