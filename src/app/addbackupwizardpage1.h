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

#ifndef ADDBACKUPWIZARDPAGE1_H
#define ADDBACKUPWIZARDPAGE1_H

#include <QtGui/QWizardPage>

class DeviceWidget;

namespace Ui {
  class AddBackupWizardPage1View;
}

class AddBackupWizardPage1 : public QWizardPage
{
  Q_OBJECT

  public:
    AddBackupWizardPage1(QWidget* parent = 0);
    virtual ~AddBackupWizardPage1();

    bool isComplete () const;

  private slots:
    void slotDeviceItemSelectionChanged();

  signals:
    void completeChanged();

  private:
    Ui::AddBackupWizardPage1View* m_view;

    DeviceWidget* m_deviceWidget;
};

#endif // ADDBACKUPWIZARDPAGE1_H
