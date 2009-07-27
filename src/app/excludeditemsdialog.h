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

#ifndef EXCLUDEDITEMSDIALOG_H
#define EXCLUDEDITEMSDIALOG_H

#include <KDialog>

namespace Ui {
  class ExcludedItemsView;
}

class ExcludedItemsDialog : public KDialog
{
  Q_OBJECT

  public:
    ExcludedItemsDialog(const QStringList& excludedItems, QWidget* parent = 0);
    ~ExcludedItemsDialog();

    QStringList excludedItems() const;

  private slots:
    void slotBtnExcludeClicked();
    void slotBtnRemoveClicked();
    void slotExcludeChanged();
    void sloExcludedItemsSelectionChanged();

  private:
    void setupConnections();

    Ui::ExcludedItemsView* m_view;
};

#endif // EXCLUDEDITEMSDIALOG_H
