#include <signaleditor.h>
#include <ui_signaleditor.h>
#include <QPushButton>

SignalEditor::SignalEditor(QWidget* parent) : QDialog(parent)
  , ui(new Ui::SignalEditor)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Go"));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setCursor(Qt::PointingHandCursor);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &SignalEditor::filterList);
    connect(ui->signalList->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=] {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
                    ui->signalList->selectedItems().size() == 1
                    && !ui->signalList->selectedItems().first()->isHidden());
    });
}

SignalEditor::~SignalEditor()
{
    delete ui;
}

void SignalEditor::setSignalList(const QStringList& signalss)
{
    ui->searchEdit->clear();
    ui->signalList->clear();
    ui->signalList->addItems(signalss);
    ui->searchEdit->setFocus();
    filterList();
}

QString SignalEditor::currentSignal() const
{
    return ui->signalList->currentItem()->text();
}

void SignalEditor::clear()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->searchEdit->clear();
    ui->signalList->clear();
}

void SignalEditor::filterList()
{
    const QString& filterText = ui->searchEdit->text();
    for (int i = 0; i < ui->signalList->count(); i++) {
        QListWidgetItem* item = ui->signalList->item(i);
        item->setHidden(!item->text().contains(filterText, Qt::CaseInsensitive));
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
                ui->signalList->selectedItems().size() == 1
                && !ui->signalList->selectedItems().first()->isHidden());
}
