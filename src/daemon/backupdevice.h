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

#ifndef BACKUPDEVICE_H
#define BACKUPDEVICE_H

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <solid/solidnamespace.h>

class KJob;

/*!
  Class used to interact with backup devices
*/
class BackupDevice : public QObject
{
  Q_OBJECT

  public:
    BackupDevice(QObject *parent=0);

    //! returns true if the device is available
    bool isAvailable();

    //! returns true if the device can be used for doing backups
    bool isAccesible();

    //! performs the operations needed to make the device accessible
    void setup();

    //! creates the directory where the back ups will be stored
    bool createBackupDirectory();

    //! remove a backup directory
    void removeBackupDirectories(QStringList&);

  private slots:
    void slotDeviceAdded(QString);
    void slotDeviceRemoved(QString);
    void slotDiskMounted(Solid::ErrorType error,QVariant message,QString udi);
    void slotDeviceAccessibilityChanged(bool,QString);
    void slotBackupDirectoriesRemoved(KJob*);

  signals:
    /*!
      Signal emitted when the setup operation is completed.
      \param ok is set to false if an error occurred during the setup operation
      \param message contains the error message
    */
    void setupDone(bool ok, QString message);

    /*!
      Signal emitted when an external device is attached and the user has not
      yet configured kaveau
    */
    void newDeviceAttached();

    /*!
      Signal emitted when accessibility of the backup device changes.
      Usually it's emitted when the disk is umounted or unplugged.
    */
    void accessibilityChanged(bool accessible);

    /*!
      Signal emitted when the old backup directories have been removed.
      \param ok is set to false if an error occurred during the operation
      \param message contains the error message
    */
    void backupDirectoriesRemoved(bool ok, QString message);
};

#endif // BACKUPDEVICE_H
