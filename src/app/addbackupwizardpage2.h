#ifndef ADDBACKUPWIZARDPAGE2_H
#define ADDBACKUPWIZARDPAGE2_H

#include <QtGui/QWizardPage>

namespace Ui {
  class AddBackupWizardPage2View;
}

class AddBackupWizardPage2 : public QWizardPage
{
  Q_OBJECT

  public:
    AddBackupWizardPage2(QWidget* parent = 0);
    virtual ~AddBackupWizardPage2();

  signals:
    void completeChanged();

  private slots:
    void slotBtnExcludeClicked();
    void slotBtnRemoveClicked();
    void slotExcludeChanged();
    void sloExcludedItemsSelectionChanged();

  private:
    void populateDeviceView();
    void setupConnections();

    Ui::AddBackupWizardPage2View* m_view;
};

#endif // ADDBACKUPWIZARDPAGE2_H
