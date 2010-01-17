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

#include "kaveau_utils_export.h"

namespace Kaveau {

  #define DATE_FORMAT "yyyy-MM-ddThh:mm:ss"

  //! Converts bytes to human format (like 1Gb or 700Mb)
  KAVEAU_UTILS_EXPORT const QString bytesToHuman(qulonglong bytes);

  //! Calculates the relative backup path
  KAVEAU_UTILS_EXPORT const QString calculateRelativeBackupPath();

  //! Calculates the final backup destination
  KAVEAU_UTILS_EXPORT const QString calculateBackupDestination(const QString& mount,
                                                               const QString& relative);

  //! Calculates the final backup destination, convenience method
  KAVEAU_UTILS_EXPORT const QString calculateBackupDestination(const QString& mount);

  /*!
    Function used to find the old backup directories to remove.
    Kaveau keeps:
    \li hourly backups for the past 24 hours
    \li daily backups for the past month
    \li weekly backups until the external disk is full
  */
  KAVEAU_UTILS_EXPORT const QStringList findBackupDirectoriesToDelete(const QStringList& dirs);


  /*!
    Function used to check if a certain device can be used by kaveau.
    \param udi the udi of the device to check
  */
  KAVEAU_UTILS_EXPORT bool isDeviceInteresting(const QString& udi);
}
#endif // COMMON_H
