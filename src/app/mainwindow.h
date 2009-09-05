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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <KXmlGuiWindow>

#include <solid/solidnamespace.h>

namespace Ui {
  class MainWidgetBase;
}

class BackupThread;
class BackupRemoverThread;
class KSystemTrayIcon;
class QCloseEvent;

class MainWindow : public KXmlGuiWindow
{
  Q_OBJECT

  public:
    MainWindow(QWidget *parent=0);
    virtual ~MainWindow();

  protected:
    void closeEvent(QCloseEvent *event);

  private:
    void setupActions();
    void setupConnections();
    void setupTrayIcon();

    void updateBackupView();
    void updateDiskUsage(const QString&);
    void showGenericError(const QString& message, bool disableBackup = true);

    bool isBackupDiskPlugged();
    void createBackupDirectory();
    void mountBackupPartition();
    bool isRdiffAvailable();

    void backupIfNeeded();
    void scheduleNextBackup(int);

  private slots:
    void slotPurgeOldBackups();
    void slotStartBackupWizard();
    void slotStartBackup();
    void slotShowLog();
    void slotEditFilters();
    void slotBackupFinished(bool, QString);
    void slotDeviceAdded(QString);
    void slotDeviceRemoved(QString);
    void slotBackupPartitionMounted(Solid::ErrorType error,QVariant message,QString udi);
    void slotDeleteDestinationDone();

  private:
    Ui::MainWidgetBase* m_mainWidget;
    KSystemTrayIcon* m_trayIcon;
    BackupThread* m_backupThread;
    BackupRemoverThread* m_backupRemoverThread;
    bool m_backupDiskPlugged;
    QString m_lastError;
    QString m_mount;
};

#endif
