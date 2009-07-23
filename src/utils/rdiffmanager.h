/* This file is part of kaveau
 * The base of this file has been taken from the Keep project
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

#ifndef _RDIFFMANAGER_H_
#define _RDIFFMANAGER_H_

#include <QtCore/QList>
#include <QtCore/QDateTime>

#include "rdifflistener.h"

class Backup;
class KProcess;

class RdiffManager
{
  public:
    RdiffManager();
    ~RdiffManager();

    QList<QDateTime> incrementList(Backup* backup);
    QDateTime lastIncrement(Backup* backup);

    QString compareAtTime(Backup* backup, QDateTime date);
    QString listChangedSince(Backup* backup, QDateTime date);
    QString listAtTime(Backup* backup, QDateTime date);

    bool isRdiffAvailable();
    QString rdiffVersion();

    QString errorString() const;

    bool doBackup(Backup* backup);
    bool restoreBackup(Backup* backup,QDateTime time);

  private:
    void removeOldIncrements(Backup* backup);
    KProcess* createRdiffProcess();
    bool startRdiffProcess(KProcess* proc);

    QString m_error;
};

#endif
