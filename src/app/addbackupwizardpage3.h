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

#ifndef ADDBACKUPWIZARDPAGE3_H
#define ADDBACKUPWIZARDPAGE3_H

#include <QtGui/QWizardPage>

namespace Ui {
  class AddBackupWizardPage3View;
}

class AddBackupWizardPage3 : public QWizardPage
{
  Q_OBJECT

  public:
    AddBackupWizardPage3(QWidget* parent = 0);
    virtual ~AddBackupWizardPage3();

    QStringList excludedDirs();

  signals:
    void completeChanged();

  private slots:
    void slotBtnExcludeClicked();
    void slotBtnRemoveClicked();
    void slotExcludeChanged();
    void sloExcludedItemsSelectionChanged();

  private:
    void setupConnections();

    Ui::AddBackupWizardPage3View* m_view;
};

#endif // ADDBACKUPWIZARDPAGE3_H
