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

#ifndef ADDBACKUPWIZARDPAGE2_H
#define ADDBACKUPWIZARDPAGE2_H

#include <QtGui/QWizardPage>

#include <solid/solidnamespace.h>

namespace Ui {
  class AddBackupWizardPage2View;
}

class AddBackupWizardPage2 : public QWizardPage
{
  Q_OBJECT

  public:
    AddBackupWizardPage2(QWidget* parent = 0);
    virtual ~AddBackupWizardPage2();

    bool isComplete () const;
    void initializePage();
    QString destination() const;
    QString deviceUDI() const;
    bool eraseDestination() const;

  signals:
    void completeChanged();

  private slots:
    void slotBtnClicked();
    void slotSetupDone(Solid::ErrorType,QVariant,QString);

  private:
    void calculateDestination(const QString& mount);
    void setupConnections();
    void checkDeviceStatus();
    void verifyDestination();

    Ui::AddBackupWizardPage2View* m_view;
    QString m_destination;
};

#endif // ADDBACKUPWIZARDPAGE2_H
