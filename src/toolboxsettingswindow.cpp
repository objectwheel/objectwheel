#include <toolboxsettingswindow.h>
#include <ui_toolboxsettingswindow.h>
#include <css.h>
#include <toolsbackend.h>
#include <filemanager.h>
#include <zipper.h>
#include <savebackend.h>

#include <QScrollBar>
#include <QBuffer>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QMessageBox>
#include <designerwidget.h>

//!
//! *************************** [global] ***************************
//!

static QString handleImports(const QStringList& fileNames)
{
    QString msg;
    for (auto fileName : fileNames) {
        QTemporaryDir dir;
        if (dir.isValid()) {
            if (Zipper::extractZip(rdfile(fileName), dir.path())) {
                if (SaveBackend::isOwctrl(dir.path())) {
                    if (ToolsBackend::instance()->addTool(dir.path(), true)) {
                        msg = "Tool import has successfully done.";
                    } else {
                        msg = "An unknown error occurred.";
                    }
                } else {
                    msg = "Tool is not valid or doesn't meet Owctrl™ requirements.";
                }
            } else {
                msg = "Extraction failed, zip file is not valid.";
            }
        } else {
            msg = "Temporary directory creation failed.";
        }
    }
    return msg;
}

//!
//! ********************** [ToolboxSettingsWindow] **********************
//!

ToolboxSettingsWindow::ToolboxSettingsWindow(QWidget *parent) : QWidget(parent)
    , ui(new Ui::ToolboxSettingsWindow)
{
    ui->setupUi(this);
    ToolsBackend::instance()->addToolboxTree(ui->treeWidget);

    ui->scrollArea->verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    ui->scrollArea->horizontalScrollBar()->setStyleSheet(CSS::ScrollBarH);

    connect(ui->treeWidget, &ToolboxTree::itemSelectionChanged, this, [=] {
        const bool hasValidSelection = ui->treeWidget->currentItem() &&
          ui->treeWidget->currentItem()->parent();
        ui->btnRemove->setEnabled(hasValidSelection);
        ui->btnExport->setEnabled(hasValidSelection);
        ui->btnSave->setEnabled(hasValidSelection);
        ui->txtCategory->setEnabled(hasValidSelection);
        ui->txtIcon->setEnabled(hasValidSelection);
        ui->txtName->setEnabled(hasValidSelection);
        ui->btnFileDialog->setEnabled(hasValidSelection);
        if (hasValidSelection) {
            const auto dir = dname(ui->treeWidget->urls(ui->treeWidget->currentItem()).first().toLocalFile());
            QPixmap icon;
            icon.loadFromData(dlfile(dir + separator() + "icon.png"));
            ui->lblIconPreview->setPixmap(icon);
            ui->txtIcon->setText("icon.png");
            ui->txtCategory->setText(ui->treeWidget->currentItem()->parent()->text(0));
            ui->txtName->setText(ui->treeWidget->currentItem()->text(0));
        }
    });

    auto fnDisableBtnSave = [=] {
        const bool hasValidSelection = ui->treeWidget->currentItem() &&
          ui->treeWidget->currentItem()->parent();
        ui->btnSave->setDisabled(ui->txtCategory->text().isEmpty() ||
                                 ui->txtIcon->text().isEmpty() ||
                                 ui->txtName->text().isEmpty() ||
                                 ui->lblIconPreview->pixmap()->isNull() ||
                                 !hasValidSelection);
    };

    connect(ui->btnOk, SIGNAL(clicked(bool)), SIGNAL(done()));
    connect(ui->txtCategory, &QLineEdit::textChanged, this, fnDisableBtnSave);
    connect(ui->txtIcon, &QLineEdit::textChanged, this, fnDisableBtnSave);
    connect(ui->txtName, &QLineEdit::textChanged, this, fnDisableBtnSave);
    connect(ui->txtIcon, &QLineEdit::editingFinished, this, [=] {
        ui->txtIcon->setEnabled(false);
        ui->lblLoading->setPixmap(QPixmap(":/resources/images/preloader.gif"));
        const auto dirctrl = dname(ui->treeWidget->urls(ui->treeWidget->currentItem()).first().toLocalFile());
        const auto remoteTry = dlfile(ui->txtIcon->text());
        QPixmap pixmap;
        pixmap.loadFromData(!remoteTry.isEmpty() ?
          remoteTry : dlfile(dirctrl + separator() + ui->txtIcon->text()));
        ui->lblIconPreview->setPixmap(pixmap);
        fnDisableBtnSave();
        ui->lblLoading->setPixmap(QPixmap());
        ui->txtIcon->setEnabled(true);
    });
}

ToolboxSettingsWindow::~ToolboxSettingsWindow()
{
    delete ui;
}

void ToolboxSettingsWindow::on_btnReset_clicked()
{
    if (QMessageBox::Yes == QMessageBox::question(this, "Confirm Reset",
     "This will reset tool library to factory defaults and remove"
      " all custom tools. Are you sure?")) {
        bool obstacle = false;
        for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
            auto tli = ui->treeWidget->topLevelItem(i);
            for (int j = 0; j < tli->childCount(); j++) {
                auto ci = tli->child(j);
                auto currentDir = dname(dname(ui->treeWidget->
                  urls(ci).first().toLocalFile()));//FIXME: Do same for Control GUI Editor /Tool Editor
                // TODO: Check same for selected control's child controls
                if (DesignerWidget::qmlEditorView()->isOpen(currentDir)) {
                    obstacle = true;
                    break;
                }
            }
            if (obstacle)
                break;
        }

        if (!obstacle) {
            ToolsBackend::instance()->resetTools();
        } else { //FIXME: Do same for Control GUI Editor /Tool Editor
            // TODO: Check same for selected control's child controls
            QMessageBox::information(this, "Oops",
              "Some tool documents are open within QML Editor or "
              "Tool Editor, please close them first.");
        }
    }
}

void ToolboxSettingsWindow::on_btnRemove_clicked()
{
    if (QMessageBox::Yes == QMessageBox::question(this, "Confirm Removal",
      "This will remove selected tool from tool library. Are you sure?")) {
        auto currentDir = dname(dname(ui->treeWidget->urls
          (ui->treeWidget->currentItem()).first().toLocalFile()));
        if (!DesignerWidget::qmlEditorView()->isOpen(currentDir)) {
            ToolsBackend::instance()->removeTool(dname(dname(ui->treeWidget->urls
              (ui->treeWidget->currentItem()).first().toLocalFile())));
        } else { //FIXME: Do same for Control GUI Editor /Tool Editor
            // TODO: Check same for selected control's child controls
            QMessageBox::information(this, "Oops",
              "Some documents belongs to this tool is open within QML "
              "Editor or Tool Editor, please close them first.");
        }
    }
}

void ToolboxSettingsWindow::on_btnAdd_clicked()
{
    ToolsBackend::instance()->newTool();
}

void ToolboxSettingsWindow::on_btnImport_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(tr("Zip files (*.zip)"));
    dialog.setViewMode(QFileDialog::Detail);
    if (dialog.exec()) {
        auto msg = handleImports(dialog.selectedFiles());
        QMessageBox::information(this, "Finished",
          msg.contains("successfully") ? msg :
            "One or more files contain errors: " + msg);
    }
}

void ToolboxSettingsWindow::on_btnExport_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    if (dialog.exec()) {
        QString msg;
        auto dir = dname(dname(ui->treeWidget->urls(ui->treeWidget->currentItem()).first().toLocalFile()));
        auto toolName = ui->treeWidget->currentItem()->text(0);
        if (rm(dialog.selectedFiles().first() + separator() + toolName + ".zip")) {
            if (Zipper::compressDir(dir, dialog.selectedFiles().at(0) + separator() + toolName + ".zip")) {
                msg = "Tool export has successfully done.";
            } else {
                msg = "An unknown error occurred while making zip file.";
            }
        } else {
            msg = "An error occurred while removing existing zip file within selected directory.";
        }
        QMessageBox::information(this, "Finished", msg);
    }
}

void ToolboxSettingsWindow::on_btnFileDialog_clicked()
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open Image"),
      QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first(),
      tr("Image Files (*.png *.jpg *.bmp)"));
    if (!fileName.isEmpty()) {
        ui->txtIcon->setText(fileName);
        emit ui->txtIcon->editingFinished();
    }
}

void ToolboxSettingsWindow::on_btnSave_clicked()
{
    ToolsBackend::ChangeSet changeSet;
    changeSet.category = ui->txtCategory->text();
    changeSet.iconPath = ui->txtIcon->text();
    changeSet.name = ui->txtName->text();
    changeSet.toolPath = dname(dname(ui->treeWidget->urls
      (ui->treeWidget->currentItem()).first().toLocalFile()));
    ToolsBackend::instance()->changeTool(changeSet);
}
