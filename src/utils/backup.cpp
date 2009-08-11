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

#include "backup.h"
#include "common.h"

Backup::Backup()
{
}

Backup::Backup( QString source, QString diskUdi, QString relativeDest,
                QStringList excludeList,
                QDateTime lastBackupTime)
{
  m_source = source;
  m_relativeDest = relativeDest;
  m_dest = "";
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

void Backup::setSource(const QString& source)
{
  m_source = source;
}

void Backup::setMount(const QString& mount)
{
  m_mount = mount;
  updateDest();
}

QString Backup::mount() const
{
  return m_mount;
}

void Backup::updateDest()
{
  m_dest = calculateBackupDestination(m_mount, m_relativeDest);
}

QString Backup::dest() const
{
  return m_dest;
}

void Backup::setDest(const QString& dest)
{
  m_dest = dest;
}

QString Backup::relativeDest() const
{
  return m_relativeDest;
}

void Backup::setRelativeDest(const QString& relativeDest)
{
  m_relativeDest = relativeDest;
}

QString Backup::diskUdi() const
{
  return m_diskUdi;
}

void Backup::setDiskUdi(const QString& uid)
{
  m_diskUdi = uid;
}

QStringList Backup::excludeList() const
{
  return m_excludeList;
}

void Backup::setExcludeList(const QStringList& excludeList)
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
