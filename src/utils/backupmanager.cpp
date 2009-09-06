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

#include "backupmanager.h"

#include "common.h"
#include "processlistener.h"
#include "settings.h"

#include <kdebug.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kprocess.h>
#include <kshell.h>
#include <QtCore/QtAlgorithms>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

BackupManager::BackupManager(Settings* settings, QObject* parent)
    : QObject(parent),
      m_settings (settings)
{
}

bool BackupManager::doBackup()
{
  QString backupTimestamp = QDateTime::currentDateTime().toString(DATE_FORMAT);
  bool updateCurrent = false;
  m_error.clear();

  QString current = QFile::encodeName(KShell::quoteArg(QDir::cleanPath(m_settings->dest() + QDir::separator() + "current")));
  QString destination = QFile::encodeName(KShell::quoteArg(QDir::cleanPath(m_settings->dest()+ QDir::separator() + backupTimestamp)));

  KProcess proc;
  ProcessListener listener(&proc);
  proc << "rsync";
  proc.setOutputChannelMode(KProcess::SeparateChannels);

  // Adds include and exclude
  foreach (QString item, m_settings->excludeList()) {
    proc << "--exclude";
    proc << QFile::encodeName(KShell::quoteArg(item));
  }

  // Common options used by default
  proc << "-xaP";
  proc << "-v"; //TODO: remove
  proc << "--delete";
  proc << "--delete-excluded";

  // Adds source and dest

  // Ensure the destination directory exists
  QDir destDir(m_settings->dest());
  if (!destDir.exists()) {
    destDir.mkpath(m_settings->dest());
  }

  QFileInfo currLinkInfo (current);
  if (currLinkInfo.exists() && !currLinkInfo.isSymLink()) {
    // it's a directory
    kDebug() << current << "is not a symbolic link, going to delete it!";
    if (!KIO::NetAccess::del(KUrl(current), 0)) {
      m_error = i18n("Error during deletiong of current directory.");
      kDebug() << m_error;
      return false;
    }
  } else if (currLinkInfo.exists() && currLinkInfo.isSymLink()) {
    // it's a non-broken symlink
    proc << QString("--link-dest=%1").arg(current);
    updateCurrent = true;
  } else if (!currLinkInfo.exists() && currLinkInfo.isSymLink()) {
    // it's a broken symlink

    // remove the broken symlink
    if (!QFile::remove(current)) {
      m_error = i18n("current is a broken symlink, error while trying to remove it.");
      kDebug() << m_error;
      return false;
    }

    QString latestBackup = findLatestBackup();
    if (!latestBackup.isEmpty()) {
      QDir::setCurrent(m_settings->dest());
      // create a symlink called current pointing to the latest backup

      if (QFile::link(latestBackup, "current")) {
        proc << QString("--link-dest=%1").arg(current);
        updateCurrent = true;
      }
    }
  }

  proc << QFile::encodeName(KShell::quoteArg(m_settings->source()));
  proc << QFile::encodeName(KShell::quoteArg(destination));

  kDebug() << "Starting process: " << proc.program().join(" ") << endl;

  // Starts the process
  if ( proc.execute() != 0 ) {
    m_error = i18n("Error starting rsync\n%1").arg(listener.stdErr().join("\n"));
    kDebug() << m_error;
    return false;
  } else {
    kDebug() << "rsync process completed";
    if (updateCurrent) {
      kDebug() << "going to delete current" << current;
      QFile currLink (current);
      if (! currLink.remove()) {
        m_error = i18n("Error during deletion of current directory.");
        kDebug() << m_error;
        return false;
      }
    }

    kDebug() << "going to create current";
    QDir::setCurrent(m_settings->dest());
    // create a symlink called current pointing to the latest backup
    kDebug() << "timestamp" << backupTimestamp;
    if (!QFile::link(backupTimestamp, "current")) {
      m_error = i18n("Error during symbolic link creation");
      kDebug() << m_error;

      // reset current directory, otherwise it will be impossible to umount the
      // external device
      QDir::setCurrent(QDir::home().path());

      return false;
    }
  }

  // reset current directory, otherwise it will be impossible to umount the
  // external device
  QDir::setCurrent(QDir::home().path());
  return true;
}

QString BackupManager::error() const
{
  return m_error;
}

bool BackupManager::isBackupProgramAvailable()
{
  KProcess proc;
  proc << "rsync";
  proc << "--version";

  if ( proc.execute() != 0 ) {
    return false;
  } else if (proc.exitCode() == 0)
    return true;
  else
    return false;
}

QString BackupManager::findLatestBackup() const
{
  QList<QDateTime> dates;
  QDir backupRoot (m_settings->dest());
  QStringList backups = backupRoot.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

  foreach (QString backup, backups)
    dates << QDateTime::fromString( backup, DATE_FORMAT);

  qSort(dates.begin(), dates.end());

  if (dates.isEmpty())
    return "";
  else
    return dates.last().toString(DATE_FORMAT);
}

void BackupManager::purgeOldBackups()
{
  QDir backupRoot (m_settings->dest());
  QStringList backups = backupRoot.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

  QStringList directoriesToRemove = findBackupDirectoriesToDelete(backups);

  foreach(QString dir, directoriesToRemove) {
    QString dirToRemove = QDir::cleanPath(m_settings->dest() + QDir::separator() + dir);
    if (QFile::exists(dirToRemove)) {
      kDebug() << "Going to remove old backup directory:" << dirToRemove;
      KIO::NetAccess::del(KUrl(dirToRemove), 0);
      kDebug() << dirToRemove << "removed";
    }
  }
}

