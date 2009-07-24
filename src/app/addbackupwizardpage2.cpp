#include "addbackupwizardpage2.h"

#include "ui_addbackupwizardpage2view.h"

AddBackupWizardPage2::AddBackupWizardPage2(QWidget* parent)
  : QWizardPage (parent)
{
  QWidget *widget = new QWidget(this);
  m_view = new Ui::AddBackupWizardPage2View();
  m_view->setupUi(widget);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(widget);
  setLayout(layout);

  m_view->excludeURL->setMode(KFile::Directory | KFile::ExistingOnly);

  setupConnections();
}

AddBackupWizardPage2::~AddBackupWizardPage2()
{
  delete m_view;
}

void AddBackupWizardPage2::setupConnections()
{
  connect (m_view->btnExclude, SIGNAL(clicked()), this, SLOT(slotBtnExcludeClicked()));
  connect (m_view->btnRemove, SIGNAL(clicked()), this, SLOT(slotBtnRemoveClicked()));

  connect (m_view->excludeURL, SIGNAL(textChanged(QString)), this, SLOT(slotExcludeChanged()));
  connect (m_view->excludeURL, SIGNAL(urlSelected(KUrl)), this, SLOT(slotExcludeChanged()));

  connect (m_view->excludedItems, SIGNAL( itemSelectionChanged()), this, SLOT(sloExcludedItemsSelectionChanged()));
}

void AddBackupWizardPage2::slotBtnExcludeClicked()
{
  QString item = m_view->excludeURL->text();

  if (m_view->excludedItems->findItems(item, Qt::MatchExactly).isEmpty())
    m_view->excludedItems->addItem(item);

  m_view->btnExclude->setEnabled(false);
  m_view->excludeURL->clear();
}

void AddBackupWizardPage2::slotBtnRemoveClicked()
{
  QList<QListWidgetItem *> selectedItems = m_view->excludedItems->selectedItems ();

  foreach (QListWidgetItem* item, selectedItems) {
    m_view->excludedItems->takeItem (m_view->excludedItems->row(item));
  }

  m_view->btnRemove->setEnabled(false);
}

void AddBackupWizardPage2::slotExcludeChanged()
{
  m_view->btnExclude->setEnabled(!m_view->excludeURL->text().isEmpty());
}

void AddBackupWizardPage2::sloExcludedItemsSelectionChanged()
{
  QList<QListWidgetItem *> selectedItems = m_view->excludedItems->selectedItems ();
  m_view->btnRemove->setEnabled(!selectedItems.isEmpty());
}

