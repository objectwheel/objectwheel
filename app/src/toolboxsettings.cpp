#include <toolboxsettings.h>
#include <ui_toolboxsettings.h>
#include <css.h>
#include <toolsmanager.h>
#include <filemanager.h>
#include <zipper.h>
#include <savemanager.h>

#include <QScrollBar>
#include <QBuffer>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QMessageBox>

ToolboxSettings::ToolboxSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ToolboxSettings)
{
    ui->setupUi(this);
    ToolsManager::instance()->addToolboxTree(ui->treeWidget);

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

        //        const auto icndir = dirctrl + separator() + "/icon.png";
        //        if (pixmap.isNull()) return;
        //        QByteArray bArray;
        //        QBuffer buffer(&bArray);
        //        buffer.open(QIODevice::WriteOnly);
        //        if (!pixmap.save(&buffer,"PNG")) return;
        //        buffer.close();
        //        if (!wrfile(icndir, bArray)) return;
        //        ui->lblIcon->setText("icon.png");
    });


//    _toolboxAddButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    _toolboxAddButton.setColor("#6BB64B");
//    _toolboxAddButton.setFixedSize(fit(17),fit(17));
//    _toolboxAddButton.setRadius(fit(8));
//    _toolboxAddButton.setIconSize(QSize(fit(13),fit(13)));
//    _toolboxAddButton.setIcon(QIcon(":/resources/images/plus.png"));
//    connect(&_toolboxAddButton, SIGNAL(clicked(bool)), SLOT(toolboxAddButtonClicked()) );

//    _toolboxRemoveButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    _toolboxRemoveButton.setColor("#C61717");
//    _toolboxRemoveButton.setFixedSize(fit(17),fit(17));
//    _toolboxRemoveButton.setRadius(fit(8));
//    _toolboxRemoveButton.setIconSize(QSize(fit(13),fit(13)));
//    _toolboxRemoveButton.setIcon(QIcon(":/resources/images/minus.png"));
//    _toolboxRemoveButton.setDisabled(true);
//    connect(&_toolboxRemoveButton, SIGNAL(clicked(bool)), SLOT(toolboxRemoveButtonClicked()) );

//    _toolboxEditButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    _toolboxEditButton.setColor("#697D8C");
//    _toolboxEditButton.setFixedSize(fit(17),fit(17));
//    _toolboxEditButton.setRadius(fit(8));
//    _toolboxEditButton.setCheckedColor(QColor("#6BB64B"));
//    _toolboxEditButton.setCheckable(true);
//    _toolboxEditButton.setIconSize(QSize(fit(13),fit(13)));
//    _toolboxEditButton.setIcon(QIcon(":/resources/images/edit.png"));
//    _toolboxEditButton.setDisabled(true);
//    connect(&_toolboxEditButton, SIGNAL(toggled(bool)), SLOT(toolboxEditButtonToggled(bool)) );

//    _toolboxImportButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    _toolboxImportButton.setColor("#3498DB");
//    _toolboxImportButton.setFixedSize(fit(17),fit(17));
//    _toolboxImportButton.setRadius(fit(8));
//    _toolboxImportButton.setIconSize(QSize(fit(13),fit(13)));
//    _toolboxImportButton.setIcon(QIcon(QPixmap(":/resources/images/left-arrow.png").transformed(QTransform().rotate(-90))));
//    connect(&_toolboxImportButton, SIGNAL(clicked(bool)), SLOT(toolboxImportButtonClicked()) );

//    _toolboxExportButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    _toolboxExportButton.setColor("#E8BC43");
//    _toolboxExportButton.setFixedSize(fit(17),fit(17));
//    _toolboxExportButton.setRadius(fit(8));
//    _toolboxExportButton.setIconSize(QSize(fit(13),fit(13)));
//    _toolboxExportButton.setIcon(QIcon(QPixmap(":/resources/images/left-arrow.png").transformed(QTransform().rotate(90))));
//    _toolboxExportButton.setDisabled(true);
//    connect(&_toolboxExportButton, SIGNAL(clicked(bool)), SLOT(toolboxExportButtonClicked()) );

//    _toolboxAdderAreaButtonSideHLay.setSpacing(0);
//    _toolboxAdderAreaButtonSideHLay.setContentsMargins(0, 0, 0, 0);
//    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxAddButton);
//    _toolboxAdderAreaButtonSideHLay.addStretch();
//    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxRemoveButton);
//    _toolboxAdderAreaButtonSideHLay.addStretch();
//    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxEditButton);
//    _toolboxAdderAreaButtonSideHLay.addStretch();
//    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxImportButton);
//    _toolboxAdderAreaButtonSideHLay.addStretch();
//    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxExportButton);

//    _toolboxUrlBox.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//    _toolboxUrlBox.setFixedHeight(fit(30));
//    _toolboxUrlBox.setPlaceholderText("Icon url");
//    _toolboxUrlBox.setText(":/resources/images/item.png");
//    _toolboxUrlBox.setDisabled(true);
//    _toolboxUrlBox.setHidden(true);
//    connect(&_toolboxUrlBox, SIGNAL(textChanged(QString)),
//            SLOT(handleToolboxUrlboxChanges(QString)));

//    _toolBoxNameBox.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//    _toolBoxNameBox.setFixedHeight(fit(30));
//    _toolBoxNameBox.setPlaceholderText("Tool name");
//    _toolBoxNameBox.setDisabled(true);
//    _toolBoxNameBox.setHidden(true);
//    connect(&_toolBoxNameBox, SIGNAL(textChanged(QString)),
//            SLOT(handleToolboxNameboxChanges(QString)));

//    _toolboxAdderAreaEditingLayout.addWidget(&_toolBoxNameBox);
//    _toolboxAdderAreaEditingLayout.addWidget(&_toolboxUrlBox);
//    _toolboxAdderAreaEditingLayout.setSpacing(0);
//    _toolboxAdderAreaEditingLayout.setContentsMargins(0, 0, 0, 0);

//    _toolboxAdderAreaVLay.addLayout(&_toolboxAdderAreaButtonSideHLay);
//    _toolboxAdderAreaVLay.addLayout(&_toolboxAdderAreaEditingLayout);
//    _toolboxAdderAreaVLay.addStretch();
//    _toolboxAdderAreaVLay.setSpacing(fit(2));
//    _toolboxAdderAreaVLay.setContentsMargins(0, 0, 0, 0);
//    _toolboxAdderAreaWidget.setLayout(&_toolboxAdderAreaVLay);
//    _toolboxAdderAreaWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//    _toolboxAdderAreaWidget.setFixedHeight(fit(17));

}

ToolboxSettings::~ToolboxSettings()
{
    delete ui;
}


//void ToolboxSettings::handleToolboxUrlboxChanges(const QString& text)
//{
//    QPixmap pixmap;
//    pixmap.loadFromData(dlfile(text));
//    if (pixmap.isNull()) return;
//    auto icon = dname(ui->treeWidget->urls(ui->treeWidget->currentItem())[0].toLocalFile()) + "/icon.png";
//    QByteArray bArray;
//    QBuffer buffer(&bArray);
//    buffer.open(QIODevice::WriteOnly);
//    if (!pixmap.save(&buffer,"PNG")) return;
//    buffer.close();
//    if (!wrfile(icon, bArray)) return;
//    ui->treeWidget->currentItem()->setIcon(0, QIcon(icon));
//}

//void ToolboxSettings::handleToolboxNameboxChanges(QString /*name*/)
//{
//    //	if (name == ui->treeWidget->currentItem()->text() || name == "") return;

//    //	int count = 1;
//    //	for (int i = 0; i < ui->treeWidget->count(); i++) {
//    //		if (ui->treeWidget->item(i)->text() == name) {
//    //			if (count > 1) {
//    //				name.remove(name.size() - 1, 1);
//    //			}
//    //			i = -1;
//    //			count++;
//    //			name += QString::number(count);
//    //		}
//    //	}

//    //	auto from = ToolsManager::toolsDir() + "/" + ui->treeWidget->currentItem()->text();
//    //	auto to = ToolsManager::toolsDir() + "/" + name;
//    //	if (!rn(from, to)) qFatal("ToolboxSettings : Error occurred");

//    //	ui->treeWidget->currentItem()->setText(name);

//    //	QList<QUrl> urls;
//    //	urls << QUrl::fromLocalFile(to + "/main.qml");
//    //	ui->treeWidget->RemoveUrls(ui->treeWidget->currentItem());
//    //	ui->treeWidget->addUrls(ui->treeWidget->currentItem(),urls);

//    //	for (int i = 0; i < m_ItemUrls.size(); i++) {
//    //		if (m_ItemUrls[i].toLocalFile() == (from+"/main.qml")) {
//    //			m_ItemUrls[i] = QUrl::fromLocalFile(to+"/main.qml");
//    //		}
//    //	}

//    //	qmlEditor->updateCacheForRenamedEntry(from, to, true);
//}

//void ToolboxSettings::toolboxRemoveButtonClicked()
//{
//    //	if (ui->treeWidget->currentRow() < 0) return;
//    //	auto name = ui->treeWidget->currentItem()->text();
//    //	QMessageBox msgBox;
//    //	msgBox.setText(QString("<b>This will remove %1 from Tool Library and Dashboard.</b>").arg(name));
//    //	msgBox.setInformativeText("Do you want to continue?");
//    //	msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
//    //	msgBox.setDefaultButton(QMessageBox::No);
//    //	msgBox.setIcon(QMessageBox::Warning);
//    //	const int ret = msgBox.exec();
//    //	switch (ret) {
//    //		case QMessageBox::Yes: {
//    //			qmlEditor->clearCacheFor(ToolsManager::toolsDir() + separator() + name, true);
//    //			rm(ToolsManager::toolsDir() + separator() + name);
//    //			ui->treeWidget->RemoveUrls(ui->treeWidget->currentItem());
//    //			delete ui->treeWidget->takeItem(ui->treeWidget->currentRow());

//    //			for (int i = 0; i < m_ItemUrls.size(); i++) {
//    //				if (m_ItemUrls[i].toLocalFile() == (ToolsManager::toolsDir() + separator() + name + "/main.qml")) {
//    //					auto items = GetAllChildren(m_Items[i]);
//    //					for (auto item : items) {
//    //						if (m_Items.contains(item)) {
//    //							SaveManager::removeSave(designWidget.rootContext()->nameForObject(item));
//    //							SaveManager::removeParentalRelationship(designWidget.rootContext()->nameForObject(item));
//    //							designWidget.rootContext()->setContextProperty(
//    //										designWidget.rootContext()->nameForObject(item), 0);
//    //							int j = m_Items.indexOf(item);
//    //							m_Items.removeOne(item);
//    //							m_ItemUrls.removeAt(j);
//    //							item->deleteLater();
//    //						}
//    //					}
//    //				}
//    //			}
//    //			HideSelectionTools();
//    //			break;
//    //		} default: {
//    //			// Do nothing
//    //			break;
//    //		}
//    //	}
//}

QString ToolboxSettings::handleImports(const QStringList& fileNames)
{
    QString msg;
    for (auto fileName : fileNames) {
        QTemporaryDir dir;
        if (dir.isValid()) {
            if (Zipper::extractZip(rdfile(fileName), dir.path())) {
                if (SaveManager::isOwctrl(dir.path())) {
                    if (ToolsManager::instance()->addTool(dir.path())) {
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

void ToolboxSettings::on_btnFileDialog_clicked()
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open Image"),
      QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first(),
      tr("Image Files (*.png *.jpg *.bmp)"));
    if (!fileName.isEmpty()) {
        ui->txtIcon->setText(fileName);
        emit ui->txtIcon->editingFinished();
    }
}

void ToolboxSettings::on_btnAdd_clicked()
{
    ToolsManager::instance()->createNewTool();
}

void ToolboxSettings::on_btnRemove_clicked()
{
    ToolsManager::instance()->removeTool(dname(dname(ui->treeWidget->urls
      (ui->treeWidget->currentItem()).first().toLocalFile())));
}

void ToolboxSettings::on_btnImport_clicked()
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

void ToolboxSettings::on_btnExport_clicked()
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
