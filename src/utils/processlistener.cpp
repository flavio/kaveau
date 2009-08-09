/* This file is part of kaveau
 * The base of this file has been taken from the Keep project
 *
 * Copyright (C) 2009 Flavio Castelli <flavio@castelli.name>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with kaveau; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "processlistener.h"

#include <kdebug.h>
#include <kprocess.h>

ProcessListener::ProcessListener(KProcess* proc)
  : QObject(),
    m_proc(proc)
{
  connect(m_proc,SIGNAL(readyReadStandardOutput()), this, SLOT(slot_readyReadStandardOutput()));
  connect(m_proc,SIGNAL(readyReadStandardError()), this, SLOT(slot_readyReadStandardError()));
}

void ProcessListener::slot_readyReadStandardOutput()
{
  m_proc->setReadChannel(QProcess::StandardOutput);
  QByteArray output;
  while(!(output = m_proc->readLine()).isEmpty()) {
    kDebug() << output;
    m_stdOut.append(output);
  }
}

void ProcessListener::slot_readyReadStandardError()
{
  m_proc->setReadChannel(QProcess::StandardError);
  QByteArray error;
  while(!(error = m_proc->readLine()).isEmpty()) {
    kDebug() << "error:" << error;
    m_stdErr.append(error);
  }
}

QString ProcessListener::errorMessage()
{
  return m_stdErr.join("").simplified();
}

QStringList ProcessListener::stdErr()
{
  return m_stdErr;
}

QStringList ProcessListener::stdOut()
{
  return m_stdOut;
}

bool ProcessListener::isOk()
{
  return !m_stdErr.isEmpty();
}
