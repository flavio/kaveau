#ifndef COMMON_H
#define COMMON_H

#include <QtCore/QString>

const QString bytesToHuman(qulonglong bytes);

const QString calculateRelativeBackupPath();

const QString calculateBackupDestination(const QString& mount);
const QString calculateBackupDestination(const QString& mount, const QString& relative);

#endif // COMMON_H
