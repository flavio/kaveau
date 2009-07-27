#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <KDialog>

namespace Ui {
  class LogView;
}


class LogDialog : public KDialog
{
  public:
    LogDialog(QString message, QWidget* parent = 0 );

  private:
    Ui::LogView* m_view;
};

#endif // LOGDIALOG_H
