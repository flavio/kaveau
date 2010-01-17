/* This file is part of the kaveau project
 * The base of this file has been taken from the Keep project
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

#ifndef _PROCESSLISTENER_H_
#define _PROCESSLISTENER_H_

#include <QtCore/QStringList>
#include <QtCore/QObject>

class KProcess;

/*!
  Class used for reading rsync output
*/
class ProcessListener: public QObject
{
  Q_OBJECT
  public:
    ProcessListener(KProcess*);

    QStringList stdOut();
    QStringList stdErr();
    QString errorMessage();
    bool isOk();

  private slots:
    void slot_readyReadStandardOutput();
    void slot_readyReadStandardError();

  private:
    KProcess* m_proc;
    QStringList m_stdOut;
    QStringList m_stdErr;
};

#endif
