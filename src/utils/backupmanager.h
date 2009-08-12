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

#ifndef BACKUPMANAGER_H
#define BACKUPMANAGER_H

class Backup;
class KJob;

#include <QtCore/QString>
#include <QtCore/QObject>

class BackupManager : public QObject
{
  Q_OBJECT

  public:
    BackupManager(Backup* backup, QObject* parent = 0);

    bool doBackup();
    QString error() const;
    static bool isBackupProgramAvailable();

  private:
    Backup* m_backup;
    bool m_running;
    bool m_ok;
    QString m_error;
};

#endif // BACKUPMANAGER_H
