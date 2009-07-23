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

#include "addbackupdialog.h"

#include "ui_addbackupwidget.h"

#include <KLocale>
#include <KMessageBox>

#include "backup.h"

AddBackupDialog::AddBackupDialog (Backup* backup, QWidget *parent)
  : KDialog(parent)
{
  setCaption(i18n("Add backup"));
  setButtons(KDialog::Cancel | KDialog::Ok);
  setDefaultButton(KDialog::Ok);
  setModal(true);

  QWidget* widget = new QWidget( this );
  m_backupWidget = new Ui::AddBackupWidget();
  m_backupWidget->setupUi(widget);
  setMainWidget( widget );

  m_backupWidget->sourceURL->setMode(KFile::Directory | KFile::ExistingOnly);
  m_backupWidget->destURL->setMode(KFile::Directory);
  m_backupWidget->excludeURL->setMode(KFile::Directory | KFile::ExistingOnly);

  m_backup = backup;
  loadBackupSettings();

  setupConnections();
}

AddBackupDialog::~AddBackupDialog()
{
  delete m_backupWidget;
}

void AddBackupDialog::setupConnections()
{
  connect (m_backupWidget->btnExclude, SIGNAL(clicked()), this, SLOT(slotBtnExcludeClicked()));
  connect (m_backupWidget->btnRemove, SIGNAL(clicked()), this, SLOT(slotBtnRemoveClicked()));

  connect (m_backupWidget->excludeURL, SIGNAL(textChanged(QString)), this, SLOT(slotExcludeChanged()));
  connect (m_backupWidget->excludeURL, SIGNAL(urlSelected(KUrl)), this, SLOT(slotExcludeChanged()));

  connect (m_backupWidget->excludedItems, SIGNAL( itemSelectionChanged()), this, SLOT(sloExcludedItemsSelectionChanged()));
}

void AddBackupDialog::loadBackupSettings()
{
  if (m_backup == 0)
    return;

  m_backupWidget->sourceURL->setUrl(m_backup->source());
  m_backupWidget->destURL->setUrl(m_backup->dest());
  m_backupWidget->excludedItems->addItems(m_backup->excludeList());
}


void AddBackupDialog::slotBtnExcludeClicked()
{
  QString item = m_backupWidget->excludeURL->text();

  if (m_backupWidget->excludedItems->findItems(item, Qt::MatchExactly).isEmpty())
    m_backupWidget->excludedItems->addItem(item);

  m_backupWidget->btnExclude->setEnabled(false);
  m_backupWidget->excludeURL->clear();
}

void AddBackupDialog::slotBtnRemoveClicked()
{
  QList<QListWidgetItem *> selectedItems = m_backupWidget->excludedItems->selectedItems ();

  foreach (QListWidgetItem* item, selectedItems) {
    m_backupWidget->excludedItems->takeItem (m_backupWidget->excludedItems->row(item));
  }

  m_backupWidget->btnRemove->setEnabled(false);
}


void AddBackupDialog::accept()
{
  if ((m_backupWidget->sourceURL->text().isEmpty()) || (m_backupWidget->destURL->text().isEmpty())) {
    KMessageBox::error(this, i18n("Error"), i18n("You have to select both source and destination directories!"));
  } else {
    if (m_backup == 0)
      m_backup = new Backup();

    m_backup->setSource(m_backupWidget->sourceURL->text());
    m_backup->setDest(m_backupWidget->destURL->text());

    QStringList items;
    for (int i = 0; i < m_backupWidget->excludedItems->count(); i++)
      items << m_backupWidget->excludedItems->item(i)->data(Qt::DisplayRole).toString();
    m_backup->setExcludeList(items);

    KDialog::accept();
  }
}

Backup* AddBackupDialog::backup()
{
  return m_backup;
}

void AddBackupDialog::slotExcludeChanged()
{
  m_backupWidget->btnExclude->setEnabled(!m_backupWidget->excludeURL->text().isEmpty());
}

void AddBackupDialog::sloExcludedItemsSelectionChanged()
{
  QList<QListWidgetItem *> selectedItems = m_backupWidget->excludedItems->selectedItems ();
  m_backupWidget->btnRemove->setEnabled(!selectedItems.isEmpty());
}

