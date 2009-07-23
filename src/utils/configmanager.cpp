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

#include "configmanager.h"

#include <KConfig>
#include <KConfigGroup>
#include <KGlobal>

#include "backup.h"

K_GLOBAL_STATIC (ConfigManager, globalConfigManager)

ConfigManager::ConfigManager()
{
  m_backup = 0;
  m_config = new KConfig( "kaveaubackup.rc" );
  loadConfigFile();
}

ConfigManager::~ConfigManager()
{
  saveConfigFile();

  delete m_backup;
  m_backup = 0;

  delete m_config;
}

ConfigManager* ConfigManager::global()
{
  return globalConfigManager;
}

void ConfigManager::setBackup(Backup* backup)
{
  if (m_backup != backup) {
    delete m_backup;
    m_backup = backup;
  }
}

Backup* ConfigManager::backup()
{
  return m_backup;
}

void ConfigManager::loadConfigFile()
{
  QStringList groupList = m_config->groupList();
  QStringList backupNameList = groupList.filter("Backup");
  for ( QStringList::Iterator it = backupNameList.begin(); it != backupNameList.end(); ++it ) {
    KConfigGroup group = m_config->group(*it);
    QString source = group.readEntry("Source");
    QString dest = group.readEntry("Dest");
    QStringList excludeList = group.readEntry("ExcludeList", QStringList());
    QDateTime lastBackupTime = group.readEntry("LastBackupTime", QDateTime());

    setBackup(new Backup(source, dest, excludeList, lastBackupTime));
  }
}


void ConfigManager::clearConfigFile()
{
  QStringList groupList = m_config->groupList();
  QStringList backupNameList = groupList.filter("Backup_");
  for ( QStringList::Iterator it = backupNameList.begin(); it != backupNameList.end(); ++it ) {
    m_config->deleteGroup(*it);
  }
  m_config->sync();
}

void ConfigManager::saveConfigFile()
{
  clearConfigFile();

  if (m_backup) {
    KConfigGroup configGroup = m_config->group("Backup");
    configGroup.writeEntry( "Source", m_backup->source());
    configGroup.writeEntry( "Dest", m_backup->dest());
    configGroup.writeEntry( "ExcludeList", m_backup->excludeList());
    configGroup.writeEntry( "LastBackupTime", m_backup->lastBackupTime());
    configGroup.sync();
  }
}

