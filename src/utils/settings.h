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

#ifndef _BACKUPCONFIG_H_ 
#define _BACKUPCONFIG_H_

#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QStringList>

class KConfig;

/*!
  Class used for saving kaveau settings to disk
*/
class Settings
{
  public:
    Settings();

    /*!
      Constructor
      \param source path of the directory to backup
      \param diskUdi UDI of the external disk used for backups
      \param relative path of the backups, external disk mount point is the base path
      \param excludeList list of items to exclude from the backup
      \param latestBackupTime latest backup time
    */
    Settings( QString source, QString diskUdi, QString relativeDest,
            QStringList excludeList,
            QDateTime lastBackupTime = QDateTime());
    
    ~Settings();
    
    static Settings* global();

    QString source() const;
    void setSource(const QString& source);

    QString diskUdi() const;
    void setDiskUdi(const QString& uid);
    bool isBackupDeviceConfigured();

    QString dest() const;

    QString mount() const;
    void setMount(const QString& mount);

    QString relativeDest() const;
    void setRelativeDest(const QString& relativeDest);
    void updateRelativeDesc();

    QStringList excludeList() const;
    void setExcludeList(const QStringList& excludeList);

    QDateTime lastBackupTime() const;
    void setLastBackupTime(const QDateTime&);

  private:
    //! method used for calculating the final backup path, used when m_source or m_mount are changed
    void updateDest();

    QString m_source;
    QString m_mount;
    QString m_relativeDest;
    QString m_dest;
    QString m_diskUdi;
    QStringList m_excludeList;
    QDateTime m_lastBackupTime;
    
    KConfig *m_config;

    void clearConfigFile();
    void loadConfigFile();
    void saveConfigFile();
};

#endif
