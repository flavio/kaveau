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

#include "excludeditemsdialog.h"

#include "ui_excludeditemsview.h"

#include <KLocale>
#include <KMessageBox>
#include <KUser>

ExcludedItemsDialog::ExcludedItemsDialog (const QStringList& items, QWidget *parent)
  : KDialog(parent)
{
  KUser user;

  setCaption(i18n("Add backup"));
  setButtons(KDialog::Cancel | KDialog::Ok);
  setDefaultButton(KDialog::Ok);
  setModal(true);

  QWidget* widget = new QWidget( this );
  m_view = new Ui::ExcludedItemsView();
  m_view->setupUi(widget);
  setMainWidget( widget );

  m_view->excludeURL->setMode(KFile::Directory | KFile::ExistingOnly);
  m_view->excludeURL->setPath(user.homeDir());
  m_view->excludedItems->addItems(items);

  setupConnections();
}

ExcludedItemsDialog::~ExcludedItemsDialog()
{
  delete m_view;
}

void ExcludedItemsDialog::setupConnections()
{
  connect (m_view->btnExclude, SIGNAL(clicked()), this, SLOT(slotBtnExcludeClicked()));
  connect (m_view->btnRemove, SIGNAL(clicked()), this, SLOT(slotBtnRemoveClicked()));

  connect (m_view->excludeURL, SIGNAL(textChanged(QString)), this, SLOT(slotExcludeChanged()));
  connect (m_view->excludeURL, SIGNAL(urlSelected(KUrl)), this, SLOT(slotExcludeChanged()));

  connect (m_view->excludedItems, SIGNAL( itemSelectionChanged()), this, SLOT(sloExcludedItemsSelectionChanged()));
}

void ExcludedItemsDialog::slotBtnExcludeClicked()
{
  QString item = m_view->excludeURL->text();

  if (m_view->excludedItems->findItems(item, Qt::MatchExactly).isEmpty())
    m_view->excludedItems->addItem(item);

  m_view->btnExclude->setEnabled(false);
  m_view->excludeURL->clear();
}

void ExcludedItemsDialog::slotBtnRemoveClicked()
{
  QList<QListWidgetItem *> selectedItems = m_view->excludedItems->selectedItems ();

  foreach (QListWidgetItem* item, selectedItems) {
    m_view->excludedItems->takeItem (m_view->excludedItems->row(item));
  }

  m_view->btnRemove->setEnabled(!m_view->excludedItems->selectedItems().isEmpty());
}



QStringList ExcludedItemsDialog::excludedItems() const
{
  QStringList items;
  for (int i = 0; i < m_view->excludedItems->count(); i++)
    items << m_view->excludedItems->item(i)->data(Qt::DisplayRole).toString();
  return items;
}

void ExcludedItemsDialog::slotExcludeChanged()
{
  m_view->btnExclude->setEnabled(!m_view->excludeURL->text().isEmpty());
}

void ExcludedItemsDialog::sloExcludedItemsSelectionChanged()
{
  QList<QListWidgetItem *> selectedItems = m_view->excludedItems->selectedItems ();
  m_view->btnRemove->setEnabled(!selectedItems.isEmpty());
}

