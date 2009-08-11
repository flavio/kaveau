#include "common.h"

#include <klocale.h>
#include <kuser.h>

#include <QtCore/QDir>
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

