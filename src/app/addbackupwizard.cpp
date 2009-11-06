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

#include "addbackupwizard.h"

#include "addbackupwizardpage1.h"
#include "addbackupwizardpage2.h"
#include "addbackupwizardpage3.h"

#include <KIconLoader>
#include <KLocale>
#include <KUser>
#include <QtGui/QPushButton>

AddBackupWizard::AddBackupWizard(QWidget *parent):
    QWizard( parent )
{
  m_completed = false;
  initView();

  setWindowTitle( i18n("Setup backup") );

  connect( QWizard::button(QWizard::FinishButton), SIGNAL( clicked()), this, SLOT( slotFinishClicked() ) );
}

void AddBackupWizard::initView()
{
  KIconLoader* loader = KIconLoader::global();
  setWindowIcon( loader->loadIcon( "tools-wizard", KIconLoader::Small ));

  page1 = new AddBackupWizardPage1(this);
  addPage(page1);

  page2 = new AddBackupWizardPage2(this);
  addPage(page2);

  page3 = new AddBackupWizardPage3(this);
  addPage(page3);
}

void AddBackupWizard::slotFinishClicked()
{
  m_completed = true;
}

void AddBackupWizard::updateGlobalSettings()
{
  Settings* settings = Settings::global();
  KUser user;

  settings->setSource(user.homeDir());
  settings->setDiskUdi(page2->deviceUDI());
  settings->updateRelativeDesc();
  settings->setExcludeList(page3->excludedDirs());
  settings->setMount( page2->mountPoint());
}

bool AddBackupWizard::deleteDestination() const
{
  return page2->eraseDestination();
}

