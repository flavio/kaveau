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

#include <QtCore/QDebug>
#include <QtTest/QtTest>

#include "common.h"


class TestCommon: public QObject
{
  Q_OBJECT
  private slots:
    void testFindBackupDirectoriesToDelete();
    void testFindBackupDirectoriesToDelete_data();
};

void TestCommon::testFindBackupDirectoriesToDelete()
{
  QFETCH(QStringList, dirs);
  QFETCH(QStringList, expected);

  QStringList result = findBackupDirectoriesToDelete(dirs);
  qSort(result.begin(), result.end());
  QCOMPARE(result, expected);
}

void TestCommon::testFindBackupDirectoriesToDelete_data()
{
  QTest::addColumn<QStringList>( "dirs" );
  QTest::addColumn<QStringList>( "expected" );

  QStringList input;
  QStringList output;
  QDateTime now = QDateTime::currentDateTime();
  QDateTime dateTime;

  // empty lists
  input.clear();
  output.clear();
  QTest::newRow("emtpy list") << input << output;

  // just today
  input.clear();
  output.clear();
  dateTime = now;

  dateTime.setTime(QTime(10,20,0,0));
  input << dateTime.toString(DATE_FORMAT);

  dateTime.setTime(QTime(10,10,0,0)); // 10 minutes before
  input << dateTime.toString(DATE_FORMAT);
  output << dateTime.toString(DATE_FORMAT);

  qSort(output.begin(), output.end());
  QTest::newRow("just today") << input << output;

  // today + some days ago
  input.clear();
  output.clear();
  dateTime = now;

  dateTime = now;

  dateTime.setTime(QTime(10,20,0,0));
  input << dateTime.toString(DATE_FORMAT);
  input << dateTime.addDays(-1).toString(DATE_FORMAT); // 1 day ago
  dateTime.setTime(QTime(10,10,0,0)); // 10 minutes before
  input << dateTime.toString(DATE_FORMAT);
  input << dateTime.addDays(-1).toString(DATE_FORMAT); // 1 day ago
  output << dateTime.toString(DATE_FORMAT);
  output << dateTime.addDays(-1).toString(DATE_FORMAT); // 1 day ago

  qSort(output.begin(), output.end());
  QTest::newRow("today + some days ago") << input << output;
}


QTEST_MAIN(TestCommon)
#include "moc_testcommon.cxx"
