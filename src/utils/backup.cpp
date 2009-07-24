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

#include "backup.h"

Backup::Backup()
{
}

Backup::Backup( QString source, QString diskUdi, QString dest,
                QStringList excludeList,
                QDateTime lastBackupTime)
{
  m_source = source;
  m_dest = dest;
  m_diskUdi = diskUdi;
  m_excludeList = excludeList;
  m_lastBackupTime = lastBackupTime;
}

Backup::~Backup()
{
}

QString Backup::source() const
{
  return m_source;
}

void Backup::setSource(QString source)
{
  m_source = source;
}

QString Backup::dest() const
{
  return m_dest;
}

void Backup::setDest(QString dest)
{
  m_dest = dest;
}

QString Backup::diskUdi() const
{
  return m_diskUdi;
}

void Backup::setDiskUdi(QString uid)
{
  m_diskUdi = uid;
}

QStringList Backup::excludeList() const
{
  return m_excludeList;
}

void Backup::setExcludeList(QStringList& excludeList)
{
  m_excludeList = excludeList;
}

QDateTime Backup::lastBackupTime() const
{
  return m_lastBackupTime;
}

void Backup::setLastBackupTime(const QDateTime& time)
{
  m_lastBackupTime = time;
}
