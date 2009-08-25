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

#ifndef BACKUPTHREAD_H
#define BACKUPTHREAD_H

#include <QtCore/QThread>

/*!
  Thread class used for performing the backup operations and not freezing the ui.
*/
class BackupThread : public QThread
{
  Q_OBJECT

  public:
    void run();

  signals:
    /*! signal emitted when the backup process is completed
      \param successfull true if the backup completed without errors, otherwise is set to false
      \param message contains an error description if successfull is false
    */
    void backupFinished(bool successfull, QString message);
};

#endif // BACKUPTHREAD_H
