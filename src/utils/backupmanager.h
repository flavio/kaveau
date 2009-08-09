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

#ifndef BACKUPMANAGER_H
#define BACKUPMANAGER_H

class Backup;

#include <QtCore/QString>
#include <QtCore/QObject>

class BackupManager : public QObject
{
  Q_OBJECT

  public:
    BackupManager(Backup* backup, QObject* parent = 0);

    void doBackup();
    static bool isBackupProgramAvailable();

  signals:
    void backupDone(bool ok, QString error);

  private slots:
    void slotDeleteDestinationDone();

  private:
    Backup* m_backup;
    bool m_running;
    bool m_ok;
    QString m_error;
    QString m_backupTimestamp;
};

#endif // BACKUPMANAGER_H
