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

#include "backup.h"
#include "processlistener.h"

#include <kdebug.h>
#include <kio/deletejob.h>
#include <klocale.h>
#include <kprocess.h>
#include <kshell.h>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>

BackupManager::BackupManager(Backup* backup, QObject* parent)
    : QObject(parent),
      m_backup (backup)
{
}

void BackupManager::doBackup()
{
  m_backupTimestamp = QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss");
  bool updateCurrent = false;

  QString current = QFile::encodeName(KShell::quoteArg(QDir::cleanPath(m_backup->dest() + QDir::separator() + "current")));
  QString destination = QFile::encodeName(KShell::quoteArg(QDir::cleanPath(m_backup->dest()+ QDir::separator() + m_backupTimestamp)));

  KProcess proc;
  ProcessListener listener(&proc);
  proc << "rsync";
  proc.setOutputChannelMode(KProcess::SeparateChannels);

  // Adds include and exclude
  foreach (QString item, m_backup->excludeList()) {
    proc << "--exclude";
    proc << QFile::encodeName(KShell::quoteArg(item));
  }

  // Common options used by default
  proc << "-xaP";
  proc << "-v"; //TODO: remove
  proc << "--delete";
  proc << "--delete-excluded";

  // Adds source and dest

  QDir currDir (current);
  if (currDir.exists()) {
    proc << QString("--link-dest=%1").arg(current);
    updateCurrent = true;
  }

  proc << QFile::encodeName(KShell::quoteArg(m_backup->source()));
  proc << QFile::encodeName(KShell::quoteArg(destination));

//  proc << m_backup->source();
//  proc << destination;

  kDebug() << "Starting process: " << proc.program().join(" ") << endl;

  // Starts the process
  if ( proc.execute() != 0 ) {
    emit backupDone( false, i18n("Error starting rsync\n%1").arg(listener.stdErr().join("\n")));
  } else {
    if (updateCurrent) {
      KIO::DeleteJob* deleteJob =  KIO::del(current);
      connect (deleteJob, SIGNAL (finished(KJob*)), this, SLOT (slotDeleteDestinationDone()));
    } else
      slotDeleteDestinationDone();
  }
}

void BackupManager::slotDeleteDestinationDone()
{
  QDir::setCurrent(m_backup->dest());
  // create a symlink called current pointing to the latest backup
  if (!QFile::link(m_backupTimestamp, "current"))
    emit backupDone(false, i18n("Error during symbolic link creation"));
  else
    emit backupDone(true, "");
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

