#include "signalchooserdialog.h"
#include "ui_signalchooserdialog.h"

#include <QPushButton>

SignalChooserDialog::SignalChooserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignalChooserDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Go"));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setCursor(Qt::PointingHandCursor);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &SignalChooserDialog::filterList);
    connect(ui->signalList->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=] {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
                    ui->signalList->selectedItems().size() == 1
                    && !ui->signalList->selectedItems().first()->isHidden());
    });
}

SignalChooserDialog::~SignalChooserDialog()
{
    delete ui;
}

void SignalChooserDialog::setSignalList(const QStringList& signalss)
{
    ui->signalList->clear();
    ui->signalList->addItems(signalss);
    ui->searchEdit->setFocus();
    ui->searchEdit->selectAll();
    filterList();
}

QString SignalChooserDialog::currentSignal() const
{
    return ui->signalList->currentItem()->text();
}

void SignalChooserDialog::sweet()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
    ui->searchEdit->clear();
    ui->signalList->clear();
}

void SignalChooserDialog::filterList()
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
