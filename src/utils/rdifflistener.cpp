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

#include "rdifflistener.h"

#include <KProcess>

RdiffListener::RdiffListener(KProcess* proc)
  : QObject(),
    m_proc(proc)
{
  connect(m_proc,SIGNAL(readyReadStandardOutput()), this, SLOT(slot_readyReadStandardOutput()));
  connect(m_proc,SIGNAL(readyReadStandardError()), this, SLOT(slot_readyReadStandardError()));
}

void RdiffListener::slot_readyReadStandardOutput()
{
  m_proc->setReadChannel(QProcess::StandardOutput);
  QByteArray output;
  while(!(output = m_proc->readLine()).isEmpty()) {
    m_stdOut.append(output);
  }
}

void RdiffListener::slot_readyReadStandardError()
{
  m_proc->setReadChannel(QProcess::StandardError);
  QByteArray error;
  while(!(error = m_proc->readLine()).isEmpty()) {
    m_stdErr.append(error);
  }
}

QString RdiffListener::errorMessage()
{
  return m_stdErr.join("").simplified();
}

QStringList RdiffListener::stdErr()
{
  return m_stdErr;
}

QStringList RdiffListener::stdOut()
{
  return m_stdOut;
}

bool RdiffListener::isOk()
{
  return !m_stdErr.isEmpty();
}
