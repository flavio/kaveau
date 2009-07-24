/* This file is part of the Keep project
   Copyright (C) 2005 Jean-Rémy Falleri <jr.falleri@laposte.net>

   Keep is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Keep is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Keep; if not, write to the
   Free Software Foundation, Inc.,
   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.           */

#ifndef _ADDBACKUPWIZARD_H_
#define _ADDBACKUPWIZARD_H_

#include <QtGui/QWizard>
#include <QtGui/QWizardPage>

class AddBackupWizardPage1;
class AddBackupWizardPage2;

class AddBackupWizard: public QWizard
{
Q_OBJECT
public:
	AddBackupWizard(QWidget *parent = 0);

private:
	//Init GUI
	void initView();

  AddBackupWizardPage1* page1;
  AddBackupWizardPage2* page2;
};

#endif
