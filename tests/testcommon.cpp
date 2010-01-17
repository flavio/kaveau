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

void printDateArray(QStringList dates)
{
  foreach(QString string, dates)
    qDebug() << string;
}

void TestCommon::testFindBackupDirectoriesToDelete()
{
  QFETCH(QStringList, dirs);
  QFETCH(QStringList, expected);

  QStringList result = Kaveau::findBackupDirectoriesToDelete(dirs);
  qSort(result.begin(), result.end());
  qSort(expected.begin(), expected.end());

//  qDebug() << "dirs";
//  printDateArray(dirs);
//  qDebug() << "expected";
//  printDateArray(expected);
//  qDebug() << "result";
//  printDateArray(result);

  QCOMPARE(result, expected);
}

/*
  Function used for finding the old backup directories to remove.
  Kaveau keeps:
  - hourly backups for the past 24 hours
  - daily backups for the past month
  - weekly backups until the external disk is full
*/
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

  // 24h backups
  input.clear();
  output.clear();
  dateTime = now;
  dateTime.setTime(QTime(now.time().hour(), 10, 00));

  for (int i = 0; i < 5; i++) {
    QDateTime time_to_keep = dateTime.addSecs(-i*3600); // i hour(s) ago
    QDateTime time_to_delete = time_to_keep.addSecs(-60); // i hour(s) and 1m ago
    input << time_to_keep.toString(DATE_FORMAT);
    input << time_to_delete.toString(DATE_FORMAT);
    output << time_to_delete.toString(DATE_FORMAT);
  }

  QTest::newRow("last 24 hours backups") << input << output;
  
  // last month backups
  dateTime = now;
  dateTime.setTime(QTime(now.time().hour(), 10, 00));

  for (int i = 2; i < 5; i++) {
    QDateTime time_to_keep = dateTime.addDays(-i); // i days ago
    QDateTime time_to_delete = time_to_keep.addSecs(-60); // i day(s) and 1m ago
    input << time_to_keep.toString(DATE_FORMAT);
    input << time_to_delete.toString(DATE_FORMAT);
    output << time_to_delete.toString(DATE_FORMAT);
  }
  
  QTest::newRow("last 24 hours + last month backups") << input << output;
  
  // weekly backups
  dateTime = now;
  dateTime.setTime(QTime(now.time().hour(), 10, 00));
  dateTime = dateTime.addMonths(-2); // 2 months ago

  for (int i = 1; i < 5; i++) {
    QDateTime time_to_keep = dateTime.addDays(-i*7); // i week(s) ago
    QDateTime time_to_delete = time_to_keep.addSecs(-60); // i week(s) and 1m ago
    input << time_to_keep.toString(DATE_FORMAT);
    input << time_to_delete.toString(DATE_FORMAT);
    output << time_to_delete.toString(DATE_FORMAT);
  }

  QTest::newRow("last 24 hours + last month + some weeks backups") << input << output;

  // nothing to delete
  input.clear();
  output.clear();

  dateTime = now;
  dateTime.setTime(QTime(now.time().hour(), 10, 00));

  for (int i = 0; i < 5; i++) {
    QDateTime time_to_keep = dateTime.addSecs(-i*3600); // i hour(s) ago
    input << time_to_keep.toString(DATE_FORMAT);
  }

  for (int i = 2; i < 5; i++) {
    QDateTime time_to_keep = dateTime.addDays(-i); // i days ago
    input << time_to_keep.toString(DATE_FORMAT);
  }

  for (int i = 1; i < 5; i++) {
    QDateTime time_to_keep = dateTime.addDays(-i*7); // i week(s) ago
    input << time_to_keep.toString(DATE_FORMAT);
  }

  QTest::newRow("nothing to delete") << input << output;
}


QTEST_APPLESS_MAIN(TestCommon)
#include "moc_testcommon.cxx"
