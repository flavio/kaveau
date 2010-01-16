#ifndef KAVEAUDAEMON_H
#define KAVEAUDAEMON_H

#include <QtCore/QObject>

class QSocketNotifier;

class BackupDevice;
class BackupManager;

class KaveauDaemon : public QObject
{
  Q_OBJECT

  public:
    KaveauDaemon(QObject *parent = 0);
    ~KaveauDaemon();

    // Unix signal handlers.
    static void hupSignalHandler(int unused);
    static void termSignalHandler(int unused);

    private:
      void setupConnections();

      void backupIfNeeded();
      void scheduleNextBackup(int);
      void scheduleNextPurgeOperation(int);

    public slots:
      // Qt signal handlers.
      void handleSigHup();
      void handleSigTerm();

    private slots:
      void slotPurgeOldBackups();
      void slotStartBackup();
      void slotBackupFinished(bool, QString);
      void slotOldBackupDirectoriesRemoved(bool, QString);

      void slotNewDeviceAttached();
      void slotBackupDeviceAccessibilityChanged(bool);
      void slotBackupDeviceSetupDone(bool, QString);

    private:
      static int sighupFd[2];
      static int sigtermFd[2];

      QSocketNotifier *snHup;
      QSocketNotifier *snTerm;

      BackupManager* m_backupManager;
      BackupDevice* m_backupDevice;

      QString m_lastError;
};

#endif // KAVEAUDAEMON_H
