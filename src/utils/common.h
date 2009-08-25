#ifndef COMMON_H
#define COMMON_H

#include <QtCore/QString>

//! Converts bytes to human format (like 1Gb or 700Mb)
const QString bytesToHuman(qulonglong bytes);

//! Calculates the relative backup path
const QString calculateRelativeBackupPath();

//! Calculates the final backup destination
const QString calculateBackupDestination(const QString& mount, const QString& relative);

//! Calculates the final backup destination, convenience method
const QString calculateBackupDestination(const QString& mount);

#endif // COMMON_H
