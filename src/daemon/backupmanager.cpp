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

using namespace Kaveau;

BackupManager::BackupManager(QObject* parent)
    : QObject(parent)
{
  m_backupRunning = false;
}

void BackupManager::doBackup()
{
  Settings* settings = Settings::global();
  m_backupRunning = true;
  m_updateCurrent = false;
  QStringList args;
  QString program = "rsync";

  m_backupTimestamp = QDateTime::currentDateTime().toString(DATE_FORMAT);
  m_destination = QFile::encodeName(KShell::quoteArg(QDir::cleanPath(settings->dest()+ QDir::separator() + m_backupTimestamp)));
  QString current = QFile::encodeName(KShell::quoteArg(QDir::cleanPath(settings->dest() + QDir::separator() + "current")));

  m_backupProcess = new KProcess(this);
  ProcessListener listener(m_backupProcess);
  m_backupProcess->setOutputChannelMode(KProcess::SeparateChannels);

  // Adds include and exclude
  foreach (QString item, settings->excludeList()) {
    args << "--exclude";
    args << QFile::encodeName(KShell::quoteArg(item));
  }

  // Common options used by default
  args << "-xaP";
  args << "-v"; //TODO: remove
  args << "--delete";
  args << "--delete-excluded";

  // Adds source and dest

  // Ensure the destination directory exists
  QDir destDir(settings->dest());
  if (!destDir.exists()) {
    destDir.mkpath(settings->dest());
  }

  QFileInfo currLinkInfo (current);
  if (currLinkInfo.exists() && !currLinkInfo.isSymLink()) {
    // it's a directory
    kDebug() << current << "is not a symbolic link, going to delete it!";
    if (!KIO::NetAccess::del(KUrl(current), 0)) {
      emit backupFinished(false, i18n("Error during deletiong of current directory."));
      return;
    }
  } else if (currLinkInfo.exists() && currLinkInfo.isSymLink()) {
    // it's a non-broken symlink
    args << QString("--link-dest=%1").arg(current);
    m_updateCurrent = true;
  } else if (!currLinkInfo.exists() && currLinkInfo.isSymLink()) {
    // it's a broken symlink

    // remove the broken symlink
    if (!QFile::remove(current)) {
      emit backupFinished(false, i18n("current is a broken symlink, error while trying to remove it."));
      return;
    }

    QString latestBackup = findLatestBackup();
    if (!latestBackup.isEmpty()) {
      QDir::setCurrent(settings->dest());
      // create a symlink called current pointing to the latest backup

      if (QFile::link(latestBackup, "current")) {
        args << QString("--link-dest=%1").arg(current);
        m_updateCurrent = true;
      }
    }
  }

  args << QFile::encodeName(KShell::quoteArg(settings->source()));
  args << QFile::encodeName(KShell::quoteArg(m_destination));

  m_backupProcess->setProgram(program, args);
  kDebug() << "Starting m_backupProcessess: " << m_backupProcess->program().join(" ") << endl;

  // Starts the m_backupProcessess
  connect (m_backupProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotBackupFinished(int,QProcess::ExitStatus)));
  m_backupProcess->start();
}

void BackupManager::slotBackupFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  Settings* settings = Settings::global();
  m_backupRunning = false;
  kDebug() << "exitCode ==" << exitCode;
  kDebug() << "output ==" << m_backupProcess->readAllStandardOutput();
  kDebug() << "error ==" << m_backupProcess->readAllStandardError();

  if (exitCode != 0) {
    emit backupFinished( false, m_backupProcess->errorString());
    return;
  } else if (exitStatus == QProcess::CrashExit) {
    emit backupFinished( false, i18n("The m_backupProcessess crashed"));
    return;
  }

  QString current = QFile::encodeName(KShell::quoteArg(QDir::cleanPath(settings->dest() + QDir::separator() + "current")));

  kDebug() << "rsync m_backupProcessess completed";
  if (m_updateCurrent) {
    kDebug() << "going to delete current" << current;
    QFile currLink (current);
    if (! currLink.remove()) {
      emit backupFinished( false, i18n("Error during deletion of current directory."));
      return;
    }
  }

  QDir::setCurrent(settings->dest());

  // create a symlink called current pointing to the latest backup
  if (!QFile::link(m_backupTimestamp, "current"))
    emit backupFinished(false, i18n("Error during symbolic link creation"));
  else
    emit backupFinished(true, "");

  // reset current directory, otherwise it will be impossible to umount the
  // external device
  QDir::setCurrent(QDir::home().path());

  delete m_backupProcess;
  m_backupProcess = 0;
}

bool BackupManager::isBackupRunning() const
{
  return m_backupRunning;
}

void BackupManager::terminateBackup()
{
  if (m_backupProcess)
    m_backupProcess->terminate();
}

bool BackupManager::isBackupProgramAvailable()
{
  KProcess m_backupProcess;
  m_backupProcess << "rsync";
  m_backupProcess << "--version";

  if ( m_backupProcess.execute() != 0 ) {
    return false;
  } else if (m_backupProcess.exitCode() == 0)
    return true;
  else
    return false;
}

QString BackupManager::findLatestBackup() const
{
  Settings* settings = Settings::global();
  QList<QDateTime> dates;
  QDir backupRoot (settings->dest());
  QStringList backups = backupRoot.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

  foreach (QString backup, backups)
    dates << QDateTime::fromString( backup, DATE_FORMAT);

  qSort(dates.begin(), dates.end());

  if (dates.isEmpty())
    return "";
  else
    return dates.last().toString(DATE_FORMAT);
}

QStringList BackupManager::oldBackups() const
{
  Settings* settings = Settings::global();
  QDir backupRoot (settings->dest());
  QStringList backups = backupRoot.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

  QFileInfo current (QDir::cleanPath(settings->dest() + QDir::separator() + "current"));

  if (current.isSymLink()) {
    QDir currentTarget (current.symLinkTarget());
    backups.removeAll(currentTarget.dirName());
  }

  return findBackupDirectoriesToDelete(backups);
}

#include "backupmanager.moc"
