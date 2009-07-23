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

#include <KUniqueApplication>
#include <KAboutData>
#include <KCmdLineArgs>

#include "mainwindow.h"

#include <kdebug.h>

int main (int argc, char *argv[])
{
  KAboutData aboutData( "kaveau", 0, ki18n("kaveau"), "0.0.1",
                          ki18n("A simple backup tool"),
                          KAboutData::License_GPL,
                          ki18n("Copyright (c) 2009 Flavio Castelli") );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KUniqueApplication app;

  MainWindow* window = new MainWindow();
  window->show();

  return app.exec();
}
