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

#ifndef COMMON_H
#define COMMON_H

#include <QtCore/QString>
#include <QtCore/QStringList>

#define DATE_FORMAT "yyyy-MM-ddThh:mm:ss"

//! Converts bytes to human format (like 1Gb or 700Mb)
const QString bytesToHuman(qulonglong bytes);

//! Calculates the relative backup path
const QString calculateRelativeBackupPath();

//! Calculates the final backup destination
const QString calculateBackupDestination(const QString& mount, const QString& relative);

//! Calculates the final backup destination, convenience method
const QString calculateBackupDestination(const QString& mount);

/*!
  Function used for finding the old backup directories to remove.
  Kaveau keeps:
  \li hourly backups for the past 24 hours
  \li daily backups for the past month
  \li weekly backups untill the external disk is full
*/
const QStringList findBackupDirectoriesToDelete(const QStringList& dirs);

#endif // COMMON_H
