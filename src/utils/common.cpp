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
  if (!dates.isEmpty()) {
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
  qSort(dates.begin(), dates.end());

  // find backups performed in the last 24 hours
  QList<QDateTime> last_24_hours_backups;
  QList<QDateTime>::iterator last_24_hours_start = qLowerBound(dates.begin(), dates.end(), now.addSecs(-24*60*60));
  QList<QDateTime>::iterator last_24_hours_end = qUpperBound(dates.begin(), dates.end(), now);

  for(QList<QDateTime>::iterator iter = last_24_hours_start; iter != last_24_hours_end; iter++)
    last_24_hours_backups.push_back(*iter);

  if (last_24_hours_end != dates.end())
    last_24_hours_backups.push_back(*last_24_hours_end);

  //keep only one backup per hour
  QMap<int, QList<QDateTime> > groupedByHour = groupDateTime(last_24_hours_backups, HOUR);
  QMap<int, QList<QDateTime> >::const_iterator hourIter;

  for (hourIter = groupedByHour.begin(); hourIter != groupedByHour.end(); hourIter++) {
    QList<QDateTime> hourlyBackups = hourIter.value();
    directoriesToRemove << findOldBackups(hourlyBackups);
  }

  // remove the processed dates
  if (last_24_hours_end != dates.end())
    last_24_hours_end++;
  dates.erase(last_24_hours_start, last_24_hours_end);

  // find backups performed in the last month
  QList<QDateTime> last_month_backups;
  QList<QDateTime>::iterator last_month_start = qLowerBound(dates.begin(), dates.end(), now.addMonths(-1));
  QList<QDateTime>::iterator last_month_end = qUpperBound(dates.begin(), dates.end(), now);

  for(QList<QDateTime>::iterator iter = last_month_start; iter != last_month_end; iter++)
    last_month_backups.push_back(*iter);

  if (last_month_end != dates.end())
    last_month_backups.push_back(*last_month_end);

  //keep only one backup per day
  QMap<int, QList<QDateTime> > groupedByDay = groupDateTime(last_month_backups, DAY);
  QMap<int, QList<QDateTime> >::const_iterator dayIter;

  for (dayIter = groupedByDay.begin(); dayIter != groupedByDay.end(); dayIter++) {
    QList<QDateTime> dailyBackups = dayIter.value();
    directoriesToRemove << findOldBackups(dailyBackups);
  }

  // remove the processed dates
  if (last_month_end != dates.end())
    last_month_end++;
  dates.erase(last_month_start, last_month_end);

  // keep only weekly backups of the remaining dates
  QMap<int, QList<QDateTime> > groupedByWeek = groupDateTime(dates, WEEK);
  QMap<int, QList<QDateTime> >::const_iterator weekIter;

  for (weekIter = groupedByWeek.begin(); weekIter != groupedByWeek.end(); weekIter++) {
    QList<QDateTime> weeklyBackups = weekIter.value();
    directoriesToRemove << findOldBackups(weeklyBackups);
  }

  return directoriesToRemove;
}

