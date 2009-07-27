#include "logdialog.h"

#include <KLocale>

#include "ui_logview.h"

LogDialog::LogDialog(QString message, QWidget* parent)
  : KDialog(parent)
{
  setCaption(i18n("Backup process output"));
  setButtons(KDialog::Ok);
  setDefaultButton(KDialog::Ok);
  setModal(true);

  QWidget* widget = new QWidget( this );
  m_view = new Ui::LogView();
  m_view->setupUi(widget);
  setMainWidget( widget );
  m_view->logEdit->setPlainText(message);
}
