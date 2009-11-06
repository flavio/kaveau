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

#include "backupdevice.h"
#include "settings.h"

#include <klocale.h>
#include <kio/deletejob.h>
#include <kurl.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QList>

//solid specific includes
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/storagedrive.h>
#include <solid/storageaccess.h>

BackupDevice::BackupDevice(QObject* parent)
  : QObject(parent)
{
  Solid::DeviceNotifier* notifier = Solid::DeviceNotifier::instance();
  connect (notifier, SIGNAL(deviceAdded(QString)),
           this, SLOT(slotDeviceAdded(QString)));
  connect (notifier, SIGNAL(deviceRemoved(QString)),
           this, SLOT(slotDeviceRemoved(QString)));
}

bool BackupDevice::isAvailable()
{
  Settings* settings = Settings::global();

  if (settings->diskUdi().isEmpty())
    return false;

  Solid::Device device (settings->diskUdi());
  if (device.isValid())
    return true;
  else
    return false;
}

bool BackupDevice::isAccesible()
{
  Settings* settings = Settings::global();

  if (!isAvailable())
    return false;

  Solid::Device device (settings->diskUdi());
  Solid::StorageAccess* storageAccess = (Solid::StorageAccess*) device.asDeviceInterface(Solid::DeviceInterface::StorageAccess);
  bool accesible =  storageAccess->isAccessible();

  if (!accesible)
    connect(storageAccess, SIGNAL(accessibilityChanged(bool,QString)), this, SLOT(slotDeviceAccessibilityChanged(bool,QString)));
  else {
    QFileInfo info (storageAccess->filePath());
    if (!info.isWritable())
      return false;

    settings->setMount(storageAccess->filePath());
  }

  return accesible;
}

void BackupDevice::setup()
{
  if (!isAvailable()) {
    emit setupDone(false, i18n("External disk not plugged"));
    return;
  } else if (isAccesible()) {
    emit setupDone(true, i18n("External disk already mounted"));
    return;
  }

  Solid::Device device (Settings::global()->diskUdi());
  Solid::StorageAccess* storageAccess = (Solid::StorageAccess*) device.asDeviceInterface(Solid::DeviceInterface::StorageAccess);

  if (storageAccess->isAccessible()) {
    slotDiskMounted(Solid::NoError, QVariant(), Settings::global()->diskUdi());
    return;
  }

  connect(storageAccess, SIGNAL(setupDone(Solid::ErrorType,QVariant,QString)), this, SLOT(slotDiskMounted(Solid::ErrorType,QVariant,QString)));
  storageAccess->setup();
}

void BackupDevice::slotDiskMounted(Solid::ErrorType error,QVariant message,QString udi)
{
  Q_UNUSED(message)
  Q_UNUSED(udi)

  if (error == Solid::NoError) {
    Settings* settings = Settings::global();
    Solid::Device device (Settings::global()->diskUdi());
    Solid::StorageAccess* storageAccess = (Solid::StorageAccess*) device.asDeviceInterface(Solid::DeviceInterface::StorageAccess);
    QFileInfo info (storageAccess->filePath());

    if (!info.isWritable())
      emit setupDone ( false, "No write permissions on backup device");
    else {
      // update the mount point
      settings->setMount(storageAccess->filePath());
      emit setupDone ( true, "");
    }
  } else
    emit setupDone ( false, i18n("unable to mount backup partition"));
}

void BackupDevice::slotDeviceAdded(QString udi)
{
  Settings* settings = Settings::global();
  if (settings->diskUdi() == udi) {
    setup();
  } else {
    emit newDeviceAttached();
  }
}

void BackupDevice::slotDeviceRemoved(QString udi)
{
  Settings* settings = Settings::global();
  if (settings->diskUdi() == udi) {
    emit accessibilityChanged(false);
  }
}

void BackupDevice::slotDeviceAccessibilityChanged(bool accessible, QString udi)
{
  Settings* settings = Settings::global();
  if (settings->diskUdi().compare(udi,Qt::CaseSensitive) == 0) {
    if (accessible) {
      Solid::Device device (udi);
      Solid::StorageAccess* storageAccess = (Solid::StorageAccess*) device.asDeviceInterface(Solid::DeviceInterface::StorageAccess);

      QFileInfo info (storageAccess->filePath());

      if (info.isWritable()) {
        // update backup dest
        settings->setMount(storageAccess->filePath());
        emit accessibilityChanged(true);
      } else {
        accessibilityChanged(false);
      }
    } else {
      settings->setMount("");
      emit accessibilityChanged(false);
    }
  }
}

bool BackupDevice::createBackupDirectory()
{
  Settings* settings = Settings::global();
  QDir dir;
  return dir.mkpath(settings->dest());
}

void BackupDevice::removeBackupDirectories(QStringList& dirs)
{
  Settings* settings = Settings::global();
  KUrl::List directoriesToRemove;

  foreach(QString dir, dirs) {
    QDir dirToRemove;
    QString path = dirToRemove.cleanPath(settings->dest() + QDir::separator() + dir );
    if (QFile::exists(path)) {
      directoriesToRemove << KUrl(path);
    }
  }

  KIO::DeleteJob* deleteJob = KIO::del(directoriesToRemove);
  connect (deleteJob, SIGNAL(finished(KJob*)), this, SLOT( slotBackupDirectoriesRemoved(KJob*)));
}

void BackupDevice::slotBackupDirectoriesRemoved(KJob* job)
{
  if (job->error())
    emit backupDirectoriesRemoved(false, job->errorString());
  else
    emit backupDirectoriesRemoved(true, "");
}
