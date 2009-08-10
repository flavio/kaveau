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

#ifndef ADDBACKUPWIZARDPAGE3_H
#define ADDBACKUPWIZARDPAGE3_H

#include <QtGui/QWizardPage>

#include <solid/solidnamespace.h>

namespace Ui {
  class AddBackupWizardPage3View;
}

class AddBackupWizardPage3 : public QWizardPage
{
  Q_OBJECT

  public:
    AddBackupWizardPage3(QWidget* parent = 0);
    virtual ~AddBackupWizardPage3();

    bool isComplete () const;
    void initializePage();
    QString destination() const;
    QString deviceUDI() const;

  signals:
    void completeChanged();

  private slots:
    void slotDestChanged();
    void slotSetupDone(Solid::ErrorType,QVariant,QString);

  private:
    void setupConnections();
    void checkDeviceStatus();

    Ui::AddBackupWizardPage3View* m_view;
};

#endif // ADDBACKUPWIZARDPAGE3_H
