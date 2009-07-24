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

#ifndef _BACKUP_H_
#define _BACKUP_H_

#include <QtCore/QDateTime>
#include <QtCore/QStringList>

class Backup
{
  public:
    Backup( QString source, QString diskUdi, QString dest,
            QStringList excludeList,
            QDateTime lastBackupTime = QDateTime());
    Backup();
    ~Backup();

    QString source() const;
    void setSource(QString source);

    QString diskUdi() const;
    void setDiskUdi(QString uid);

    QString dest() const;
    void setDest(QString dest);

    QStringList excludeList() const;
    void setExcludeList(QStringList& excludeList);

    QDateTime lastBackupTime() const;
    void setLastBackupTime(const QDateTime&);

  private:
    QString m_source;
    QString m_dest;
    QString m_diskUdi;
    QStringList m_excludeList;
    QDateTime m_lastBackupTime;
};

#endif
