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

#include "settings.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>

#include "common.h"

K_GLOBAL_STATIC (Settings, globalConfigManager)

Settings::Settings()
{
  m_config = new KConfig( "kaveaubackup.rc" );
  loadConfigFile();
}

Settings::Settings( QString source, QString diskUdi, QString relativeDest,
                QStringList excludeList,
                QDateTime lastBackupTime)
{
  m_config = new KConfig( "kaveaubackup.rc" );
  loadConfigFile();

  m_source = source;
  m_relativeDest = relativeDest;
  m_dest = "";
  m_diskUdi = diskUdi;
  m_excludeList = excludeList;
  m_lastBackupTime = lastBackupTime;
}

Settings::~Settings()
{
  saveConfigFile();
  delete m_config;
}

Settings* Settings::global()
{
  return globalConfigManager;
}

void Settings::loadConfigFile()
{
  QStringList groupList = m_config->groupList();
  QStringList backupNameList = groupList.filter("Backup");
  for ( QStringList::Iterator it = backupNameList.begin(); it != backupNameList.end(); ++it ) {
    KConfigGroup group = m_config->group(*it);
    setSource(group.readEntry("Source"));
    if ((group.hasKey("RelativeDest")) && (!group.readEntry("RelativeDest").isEmpty()))
      setRelativeDest(group.readEntry("RelativeDest"));
    else
      setRelativeDest(calculateRelativeBackupPath());
    setDiskUdi(group.readEntry("DiskUdi"));
    setExcludeList(group.readEntry("ExcludeList", QStringList()));
    setLastBackupTime(group.readEntry("LastBackupTime", QDateTime()));
  }
}

void Settings::clearConfigFile()
{
  QStringList groupList = m_config->groupList();
  QStringList backupNameList = groupList.filter("Backup_");
  for ( QStringList::Iterator it = backupNameList.begin(); it != backupNameList.end(); ++it ) {
    m_config->deleteGroup(*it);
  }
  m_config->sync();
}

void Settings::saveConfigFile()
{
  clearConfigFile();

  KConfigGroup configGroup = m_config->group("Backup");
  configGroup.writeEntry( "Source", m_source);
  configGroup.writeEntry( "RelativeDest", m_relativeDest);
  configGroup.writeEntry( "DiskUdi", m_diskUdi);
  configGroup.writeEntry( "ExcludeList", m_excludeList);
  configGroup.writeEntry( "LastBackupTime", m_lastBackupTime);
  configGroup.sync();
}

QString Settings::source() const
{
  return m_source;
}

void Settings::setSource(const QString& source)
{
  m_source = source;
}

void Settings::setMount(const QString& mount)
{
  m_mount = mount;

  if (mount.isEmpty())
    m_dest = "-";
  else
    updateDest();
}

QString Settings::mount() const
{
  return m_mount;
}

void Settings::updateDest()
{
  m_dest = calculateBackupDestination(m_mount);
}

QString Settings::dest() const
{
  return m_dest;
}

QString Settings::relativeDest() const
{
  return m_relativeDest;
}

void Settings::setRelativeDest(const QString& relativeDest)
{
  m_relativeDest = relativeDest;
}

void Settings::updateRelativeDesc() {
  m_relativeDest = calculateRelativeBackupPath();
}


QString Settings::diskUdi() const
{
  return m_diskUdi;
}

void Settings::setDiskUdi(const QString& uid)
{
  m_diskUdi = uid;
}

bool Settings::isBackupDeviceConfigured()
{
  return !m_diskUdi.isEmpty();
}

QStringList Settings::excludeList() const
{
  return m_excludeList;
}

void Settings::setExcludeList(const QStringList& excludeList)
{
  m_excludeList = excludeList;
}

QDateTime Settings::lastBackupTime() const
{
  return m_lastBackupTime;
}

void Settings::setLastBackupTime(const QDateTime& time)
{
  m_lastBackupTime = time;
}
