#ifndef ADDBACKUPWIZARDPAGE1_H
#define ADDBACKUPWIZARDPAGE1_H

#include <QtGui/QWizardPage>

namespace Ui {
  class AddBackupWizardPage1View;
}

class AddBackupWizardPage1 : public QWizardPage
{
  Q_OBJECT

  public:
    AddBackupWizardPage1(QWidget* parent = 0);
    virtual ~AddBackupWizardPage1();

  private slots:
    void slotRefresh();

  signals:
    void completeChanged();

  private:
    void populateDeviceView();

    Ui::AddBackupWizardPage1View* m_view;
};

#endif // ADDBACKUPWIZARDPAGE1_H
