#include "devicewidget.h"

DeviceWidget::DeviceWidget(QWidget *parent) :
    QWidget(parent){
    setupUi(this);
}

void DeviceWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi(this);
        break;
    default:
        break;
    }
}
