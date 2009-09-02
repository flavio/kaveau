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

#include "common.h"

#include <klocale.h>
#include <kuser.h>

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtNetwork/QHostInfo>

const QString bytesToHuman(qulonglong bytes)
{
  QString size;

  if(bytes >= 1024ul*1024ul*1024ul*1024ul) {
    size = i18nc("units", "%1 TiB").arg(bytes / (1024ul*1024ul*1024ul*1024)); // the unit - terabytes
  } else if(bytes >= 1024ul*1024ul*1024ul) {
    size = i18nc("units", "%1 GiB").arg(bytes / (1024ul*1024ul*1024)); // the unit - gigabytes
  } else if(bytes > 1024ul*1024ul) {
    size = i18nc("units", "%1 MiB").arg(bytes / (1024ul*1024ul)); // the unit - megabytes
  } else {
    size = i18nc("units", "%1 KiB").arg(bytes / 1024ul); // the unit - kilobytes
  }

  return size;
}

const QString calculateRelativeBackupPath()
{
  KUser user;

  QString path = "kaveau";
  path += QDir::separator();
  path += QHostInfo::localHostName();
  path += QDir::separator();
  path += user.loginName();

  return path;
}

const QString calculateBackupDestination(const QString& mount)
{
  return calculateBackupDestination(mount, calculateRelativeBackupPath());
}

const QString calculateBackupDestination(const QString& mount, const QString& relative)
{
  return QDir::cleanPath( mount + QDir::separator() + relative);
}

/*!
  Grouping options for the QDateTime objects
*/
enum DATETIME_GROUP_BY {
  WEEK,
  WEEKDAY,
  HOUR,
  MONTH,
  DAY
};

/*!
  Function used for grouping a set of datetimes object by a certain field.
  \param times list containing the datetimes object to group
  \param group_by field used during the grouping operation
  \return a map with week number as key and a list of datetime objects as value
  \sa DATETIME_GROUP_BY
*/
const QMap< int, QList<QDateTime> > groupDateTime(QList<QDateTime>& times, DATETIME_GROUP_BY group_by)
{
  QMap<int, QList< QDateTime> > result;

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
      case MONTH:
        key = dateTime.date().month();
        break;
      case DAY:
        key = dateTime.date().day();
        break;
    }

    if (result.contains(key)) {
      result[key].push_back(dateTime);
    } else {
      QList<QDateTime> v;
      v.push_back(dateTime);
      result.insert(key, v);
    }
  }
  return result;
}

/*!
  Function used for finding the backups that have to be removed.
  \param weekDates list containing the datetimes objects to inspect
  \return a QStringList containing the name of the backup directories to remove
*/
const QStringList findOldBackups(QList<QDateTime>& dates)
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

const QStringList findBackupDirectoriesToDelete(const QStringList& dirs)
{
  QStringList directoriesToRemove;
  QList<QDateTime> dates;
  QDateTime now = QDateTime::currentDateTime();

  foreach(QString dir, dirs) {
    QDateTime backupTime = QDateTime::fromString(dir, DATE_FORMAT);
    if (backupTime.isValid())
      dates.push_back(backupTime);
  }

  QMap<int, QList<QDateTime> > groupedByMonth = groupDateTime(dates, MONTH);
  QMap<int, QList<QDateTime> >::const_iterator monthIter;

  for (monthIter = groupedByMonth.begin(); monthIter != groupedByMonth.end(); monthIter++) {
    QList<QDateTime> monthlyBackups = monthIter.value();
    if (monthIter.key() != now.date().month()) {
      // not current month, keep weekly backups
      QMap<int, QList<QDateTime> > groupedByWeek = groupDateTime(monthlyBackups, WEEK);
      QMap<int, QList<QDateTime> >::const_iterator weekIter;

      for (weekIter = groupedByWeek.begin(); weekIter != groupedByWeek.end(); weekIter++) {
        QList<QDateTime> weeklyBackups = weekIter.value();
        directoriesToRemove << findOldBackups(weeklyBackups);
      }
    }
    else {
      // current month, keep daily backups
      QMap<int, QList<QDateTime> > groupedByDay = groupDateTime(monthlyBackups, DAY);
      QMap<int, QList<QDateTime> >::const_iterator dayIter;

      for (dayIter = groupedByDay.begin(); dayIter != groupedByDay.end(); dayIter++) {
        QList<QDateTime> dailyBackups = dayIter.value();
        if (dayIter.key() == now.date().day()) {
          // current day, keep hourly backup
          QMap<int, QList<QDateTime> > groupedByHour = groupDateTime(dailyBackups, HOUR);
          QMap<int, QList<QDateTime> >::const_iterator hourIter;

          for (hourIter = groupedByHour.begin(); hourIter != groupedByHour.end(); hourIter++) {
            QList<QDateTime> hourBackups = hourIter.value();
            directoriesToRemove << findOldBackups(hourBackups);
          }
        } else {
          // not current day, keep daily backups
          directoriesToRemove << findOldBackups(dailyBackups);
        }
      }
    }
  }

  return directoriesToRemove;
}

