/* This file is part of the kaveau project
 * The base of this file has been taken from the Keep project
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

#include "rdiffmanager.h"

#include <KProcess>
#include <KDebug>
#include <QtCore/QFile>

#include <kshell.h>

#include "backup.h"

RdiffManager::RdiffManager()
{
}

RdiffManager::~RdiffManager()
{
}

QString RdiffManager::compareAtTime(Backup* backup, QDateTime date)
{
  // Gets the rdiff-backup process
  KProcess *proc = createRdiffProcess();

  // Adds the options
  *proc << "--compare-at-time";
  *proc << QString::number(date.toTime_t()); // increment date

  // Adds source and dest
  *proc << QFile::encodeName(KShell::quoteArg(backup->source()));
  *proc << QFile::encodeName(KShell::quoteArg(backup->dest()));

  // Adds the listener
  RdiffListener *listen = new RdiffListener(proc);

  // Starts the process
  startRdiffProcess(proc);

  QStringList output = listen->stdOut();
  QStringList::iterator it = output.begin();

  delete listen;
  delete proc;

  return (*it);
}

QString RdiffManager::listChangedSince(Backup* backup, QDateTime date)
{
  // Gets the rdiff-backup process
  KProcess* proc = createRdiffProcess();

  // Adds the options
  *proc << "--list-changed-since";
  *proc << QString::number(date.toTime_t()); // increment date

  // Adds the dest
  *proc << QFile::encodeName(KShell::quoteArg(backup->dest()));

  // Adds a listener
  RdiffListener* listen = new RdiffListener(proc);

  // Starts the process
  startRdiffProcess(proc);

  QStringList output = listen->stdOut();
  QStringList::iterator it = output.begin();

  delete listen;
  delete proc;

  return (*it);
}

QString RdiffManager::listAtTime(Backup* backup, QDateTime date)
{
  // Gets the rdiff-backup process
  KProcess *proc = createRdiffProcess();

  // Adds the options
  *proc << "--list-at-time";
  *proc << QString::number(date.toTime_t()); // increment date

  // Adds the dest
  *proc << QFile::encodeName(KShell::quoteArg(backup->dest()));

  // Adds a listener
  RdiffListener *listen = new RdiffListener(proc);


  // Starts the process
  startRdiffProcess(proc);

  QStringList output = listen->stdOut();
  QStringList::iterator it = output.begin();

  delete listen;
  delete proc;

  return (*it);
}

QList<QDateTime> RdiffManager::incrementList(Backup* backup)
{
  // Gets the rdiff-backup process
  KProcess *proc = createRdiffProcess();

  // Adds the options
  *proc << "--list-increments";
  *proc << "-v2";
  *proc << "--parsable-output";

  // Adds the dest
  *proc << QFile::encodeName(KShell::quoteArg(backup->dest()));

  // Adds a listener (for output recording)
  RdiffListener *listen = new RdiffListener(proc);

  // Starts the process
  startRdiffProcess(proc);

  QStringList output = listen->stdOut();

  if(output.empty()) {
    kDebug() << "The List of Backups returned by rdiff-backup is empty." << endl;
    delete listen;
    delete proc;
    return QList<QDateTime>();
  }

  QList<QDateTime> dateList;

  for (QStringList::iterator it2 = output.begin(); it2 != output.end(); ++it2)
  {
    kDebug() << *it2 << "\n";
    QStringList field = it2->split(" ");
    QStringList::iterator dateStr = field.begin();

    //Check if we really have somthing in it.
    if(!(*dateStr).isEmpty()) {
      long timestamp = (*dateStr).toUInt();
      QDateTime datetime;
      datetime.setTime_t(timestamp);
      dateList.append(datetime);
    }
  }

  delete listen;
  delete proc;

  return dateList;
}

QDateTime RdiffManager::lastIncrement(Backup* backup)
{
  QList<QDateTime> increments = incrementList(backup);
  if(increments.empty()) {
    return QDateTime();
  } else {
    QDateTime last = increments.last();
    return last;
  }
}

bool RdiffManager::restoreBackup(Backup* backup,QDateTime time)
{
  bool ok;

  // Gets the rdiff-backup process
  KProcess *proc = createRdiffProcess();

  // Adds the options
  *proc << "--force";
  *proc << "--restore-as-of";
  *proc << QString::number(time.toTime_t()); // Date of the increment to restore

  // Adds source and dest
  *proc << QFile::encodeName(KShell::quoteArg(backup->dest()));
  *proc << QFile::encodeName(KShell::quoteArg(backup->source()));

  // Adds a listener (for output recording)
  RdiffListener *listen = new RdiffListener(proc);

  // Starts the process
  startRdiffProcess(proc);

  if ( !listen->isOk() )
  {
    kDebug() << "Error message: " << listen->errorMessage() << endl;
    m_error = listen->errorMessage();
    ok = true;
  } else {
    m_error = "";
    ok = false;
  }

  delete listen;
  delete proc;

  return ok;
}

bool RdiffManager::isRdiffAvailable()
{
  // Gets the rdiff-backup process
  KProcess *proc = createRdiffProcess();
  *proc << "-V";

  // Adds a listener (for output recording)
  RdiffListener *listen = new RdiffListener(proc);

  // Starts the process
  startRdiffProcess(proc);

  QStringList outList = listen->stdOut();
  QStringListIterator out(outList);

  delete listen;
  delete proc;

  if ( !out.hasNext() ) {
    kDebug() << "I can't find rdiff-backup" << endl;
    return false;
  }
  kDebug() << "I have found rdiff-backup" << endl;
  return true;
}

QString RdiffManager::rdiffVersion()
{
  // Gets the rdiff-backup process
  KProcess *proc = createRdiffProcess();
  *proc << "-V";

  // Adds a listener (for output recording)
  RdiffListener *listen = new RdiffListener(proc);

  // Starts the process
  startRdiffProcess(proc);

  QStringList outList = listen->stdOut();
  QStringListIterator out(outList);

  kDebug() << "Executed process: " << proc->program().join(" ") << endl;

  delete listen;
  delete proc;

  return out.next().mid(13,6);
}

bool RdiffManager::doBackup(Backup* backup)
{
  bool ok;

  // Gets the rdiff-backup process
  KProcess *proc = createRdiffProcess();

  // Adds include and exclude
  foreach (QString item, backup->excludeList()) {
    *proc << "--exclude";
    *proc << QFile::encodeName(KShell::quoteArg(item));
  }

  // Common options used by default
  *proc << "--no-compression";
  *proc << "--exclude-special-files";

  //TODO: remove me!
  *proc << "-v9";

  // Adds source and dest
  *proc << QFile::encodeName(KShell::quoteArg(backup->source()));
  *proc << QFile::encodeName(KShell::quoteArg(backup->dest()));

  // Adds a listener (for output recording)
  RdiffListener *listen = new RdiffListener(proc);

  // Starts the process
  if (!startRdiffProcess(proc)) {
    kDebug() << "Error message: " << listen->errorMessage() << endl;
    ok = false;
    m_error = listen->errorMessage();
  } else {
    kDebug() << "Backup successfull";
    ok = true;
    m_error = "";
  }

  delete listen;
  delete proc;

  return ok;
}

void RdiffManager::removeOldIncrements(Backup* backup)
{
  // Gets the rdiff-backup process
  KProcess* proc = createRdiffProcess();

  //TODO :fixme
  // Adds the options
//  *proc << "--remove-older-than" << QString("%1").arg(backup->deleteAfter()) + "D";

  // Adds dest
  *proc << backup->dest();

  startRdiffProcess(proc);

  delete proc;
}

KProcess* RdiffManager::createRdiffProcess()
{
  KProcess* proc = new KProcess();
  *proc << "rdiff-backup";
  proc->setOutputChannelMode(KProcess::SeparateChannels);

  return proc;
}

bool RdiffManager::startRdiffProcess(KProcess* proc)
{
  if ( proc->execute() != 0 ) {
    kDebug() << "Error executing rdiff-backup" << endl;
    return false;
  }

  kDebug() << "Executed process: " << proc->program().join(" ") << endl;
  return true;
}

QString RdiffManager::errorString() const
{
  return m_error;
}

