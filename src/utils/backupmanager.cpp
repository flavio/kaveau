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
#include "common.h"
#include "processlistener.h"

#include <kdebug.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kprocess.h>
#include <kshell.h>
#include <QtCore/QtAlgorithms>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMap>
#include <QtCore/QVector>

#define DATE_FORMAT "yyyy-MM-ddThh:mm:ss"

/*!
  Grouping options for the QDateTime objects
*/
enum DATETIME_GROUP_BY {
  WEEK,
  WEEKDAY,
  HOUR
};

/*!
  Function used for grouping a set of datetimes object by a certain field.
  \param times vector containing the datetimes object to group
  \param group_by field used during the grouping operation
  \return a map with week number as key and a vector of datetime objects as value
  \sa DATETIME_GROUP_BY
*/
const QMap< int, QVector<QDateTime> > groupDateTime(QVector<QDateTime>& times, DATETIME_GROUP_BY group_by)
{
  QMap<int, QVector< QDateTime> > result;

  foreach (QDateTime dateTime, times) {
    int key;

    switch (group_by) {
      case WEEK:
        key = dateTime.date().weekNumber();
        break;
      case WEEKDAY:
        key = dateTime.date().dayOfWeek();
        break;
      case HOUR:
        key = dateTime.time().hour();
        break;
    }

    if (result.contains(key)) {
      result[key].push_back(dateTime);
    } else {
      QVector<QDateTime> v;
      v.push_back(dateTime);
      result.insert(key, v);
    }
  }
  return result;
}

/*!
  Function used for finding the backups that have to be removed.
  \param weekDates vector containing the datetimes objects to inspect
  \return a QStringList containing the name of the backup directories to remove
*/
const QStringList findOldBackups(QVector<QDateTime>& dates)
{
  qSort(dates.begin(), dates.end());
  if (dates.size() > 1) {
    dates.pop_back();
  }

  QStringList directories;
  foreach(QDateTime dateTime, dates) {
    directories << dateTime.toString(DATE_FORMAT);
  }

  return directories;
}

BackupManager::BackupManager(Backup* backup, QObject* parent)
    : QObject(parent),
      m_backup (backup)
{
}

bool BackupManager::doBackup()
{
  QString backupTimestamp = QDateTime::currentDateTime().toString(DATE_FORMAT);
  bool updateCurrent = false;
  m_error.clear();

  QString current = QFile::encodeName(KShell::quoteArg(QDir::cleanPath(m_backup->dest() + QDir::separator() + "current")));
  QString destination = QFile::encodeName(KShell::quoteArg(QDir::cleanPath(m_backup->dest()+ QDir::separator() + backupTimestamp)));

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

  // Ensure the destination directory exists
  QDir destDir(m_backup->dest());
  if (!destDir.exists()) {
    destDir.mkpath(m_backup->dest());
  }

  QFileInfo currLinkInfo (current);
  if (currLinkInfo.exists()) {
    if (!currLinkInfo.isSymLink()) {
      kDebug() << current << "is not a symbolic link, going to delete it!";
      if (!KIO::NetAccess::del(KUrl(current), 0)) {
        m_error = i18n("Error during deletiong of current directory.");
        kDebug() << m_error;
        return false;
      }
    } else {
      proc << QString("--link-dest=%1").arg(current);
      updateCurrent = true;
    }
  }

  proc << QFile::encodeName(KShell::quoteArg(m_backup->source()));
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
    QDir::setCurrent(m_backup->dest());
    // create a symlink called current pointing to the latest backup
    kDebug() << "timestamp" << backupTimestamp;
    if (!QFile::link(backupTimestamp, "current")) {
      m_error = i18n("Error during symbolic link creation");
      kDebug() << m_error;
      return false;
    }
  }
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

void BackupManager::purgeOldBackups()
{
  QDir backupRoot (m_backup->dest());
  QStringList backups = backupRoot.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
  QStringList directoriesToRemove;

  QDateTime now = QDateTime::currentDateTime();

  QVector<QDateTime> dates;
  foreach(QString backup, backups) {
    QDateTime backupTime = QDateTime::fromString(backup, DATE_FORMAT);
    if (backupTime.isValid())
      dates.push_back(backupTime);
  }

  QMap<int, QVector<QDateTime> > groupedByWeek = groupDateTime(dates, WEEK);
  QMap<int, QVector<QDateTime> >::const_iterator weekIter;

  for (weekIter = groupedByWeek.begin(); weekIter != groupedByWeek.end(); weekIter++) {
    QVector<QDateTime> weekDates = weekIter.value();
    if (weekIter.key() != now.date().weekNumber())
      directoriesToRemove << findOldBackups(weekDates);
    else {
      // will keep on backup per day of the current week
      QMap<int, QVector<QDateTime> > groupedByWeekDay = groupDateTime(weekDates, WEEKDAY);
      QMap<int, QVector<QDateTime> >::const_iterator weekDayIter;

      for (weekDayIter = groupedByWeekDay.begin(); weekDayIter != groupedByWeekDay.end(); weekDayIter++) {
        QVector<QDateTime> weekDayDates = weekDayIter.value();
        if (weekDayIter.key() != now.date().dayOfWeek())
          directoriesToRemove << findOldBackups(weekDayDates);
        else {
          // will keep on backup per hour of the current day
          QMap<int, QVector<QDateTime> > groupedByHour = groupDateTime(weekDates, HOUR);
          QMap<int, QVector<QDateTime> >::const_iterator hourIter;

          for (hourIter = groupedByHour.begin(); hourIter != groupedByHour.end(); hourIter++) {
            QVector<QDateTime> hourDates = hourIter.value();
            directoriesToRemove << findOldBackups(hourDates);
          }
        }
      }
    }
  }

  if (directoriesToRemove.isEmpty()) {
    qDebug() << "nothing to delete";
  } else {
    qDebug() << "going to delete" << directoriesToRemove;
  }
}

