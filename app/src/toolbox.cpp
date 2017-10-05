#include <toolbox.h>
#include <toolsmanager.h>
#include <zipper.h>
#include <filemanager.h>
#include <savemanager.h>

using namespace Fit;

#define COLOR_BACKGROUND (QColor("#52616D"))
#define DURATION 500

ToolBox::ToolBox(QWidget *parent) : QWidget(parent)
{
    setAutoFillBackground(true);
    QPalette p(palette());
    p.setColor(QPalette::Window, COLOR_BACKGROUND);
    setPalette(p);

    _toolboxList.indicatorButton()->setIcon(QIcon(":/resources/images/right-arrow.png"));
    _toolboxList.indicatorButton()->setColor(QColor("#0D74C8"));
    _toolboxList.indicatorButton()->setRadius(fit(7));
    _toolboxList.indicatorButton()->setIconSize(QSize(fit(10), fit(10)));
    _toolboxList.indicatorButton()->resize(fit(15), fit(15));
    _toolboxList.setIndicatorButtonVisible(true);

    _toolboxList.setIconSize(fit({30, 30}));
    _toolboxList.setObjectName(QStringLiteral("_toolboxList"));
    _toolboxList.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _toolboxList.setFocusPolicy(Qt::NoFocus);
    _toolboxList.setStyleSheet(QString("QListView {\n"
                                       "	border: none;\n"
                                       "	background:#52616D;\n"
                                       "	padding-right:%1px;\n"
                                       "	padding-left:%5px;\n"
                                       "}"
                                       "QListView::item {\n"
                                       "	color:white;\n"
                                       "    border: none;\n"
                                       "	padding:%2px;\n"
                                       "}"
                                       "QListView::item:selected {\n"
                                       "	color:black;\n"
                                       "    background: #e0e4e7;\n"
                                       "    border: none;\n"
                                       "    border-radius: %3px;\n"
                                       "	padding:%4px;\n"
                                       "    margin-right: %4px;\n"
                                       "}").arg(fit(2)).arg(fit(2)).arg(fit(3)).arg(fit(2)).arg(fit(6)));
    _toolboxList.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _toolboxList.setDragEnabled(true);
    _toolboxList.setDragDropMode(QAbstractItemView::InternalMove);
    _toolboxList.setSelectionBehavior(QAbstractItemView::SelectRows);
    _toolboxList.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _toolboxList.setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    _toolboxAddButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _toolboxAddButton.setColor("#6BB64B");
    _toolboxAddButton.setFixedSize(fit(20),fit(20));
    _toolboxAddButton.setRadius(fit(4));
    _toolboxAddButton.setIconSize(QSize(fit(15),fit(15)));
    _toolboxAddButton.setIcon(QIcon(":/resources/images/plus.png"));
    connect(&_toolboxAddButton, SIGNAL(clicked(bool)), SLOT(toolboxAddButtonClicked()) );

    _toolboxRemoveButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _toolboxRemoveButton.setColor("#C1131D");
    _toolboxRemoveButton.setFixedSize(fit(20),fit(20));
    _toolboxRemoveButton.setRadius(fit(4));
    _toolboxRemoveButton.setIconSize(QSize(fit(15),fit(15)));
    _toolboxRemoveButton.setIcon(QIcon(":/resources/images/minus.png"));
    _toolboxRemoveButton.setDisabled(true);
    connect(&_toolboxRemoveButton, SIGNAL(clicked(bool)), SLOT(toolboxRemoveButtonClicked()) );

    _toolboxEditButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _toolboxEditButton.setColor("#0D74C8");
    _toolboxEditButton.setFixedSize(fit(20),fit(20));
    _toolboxEditButton.setRadius(fit(4));
    _toolboxEditButton.setCheckedColor(QColor("#0D74C8").darker(    110));
    _toolboxEditButton.setCheckable(true);
    _toolboxEditButton.setIconSize(QSize(fit(13),fit(13)));
    _toolboxEditButton.setIcon(QIcon(":/resources/images/edit.png"));
    _toolboxEditButton.setDisabled(true);
    connect(&_toolboxEditButton, SIGNAL(toggled(bool)), SLOT(toolboxEditButtonToggled(bool)) );

    _toolboxImportButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _toolboxImportButton.setColor("#F28E24");
    _toolboxImportButton.setFixedSize(fit(20),fit(20));
    _toolboxImportButton.setRadius(fit(4));
    _toolboxImportButton.setIconSize(QSize(fit(15),fit(15)));
    _toolboxImportButton.setIcon(QIcon(QPixmap(":/resources/images/left-arrow.png").transformed(QTransform().rotate(-90))));
    connect(&_toolboxImportButton, SIGNAL(clicked(bool)), SLOT(toolboxImportButtonClicked()) );

    _toolboxExportButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _toolboxExportButton.setColor("#6BB64B");
    _toolboxExportButton.setFixedSize(fit(20),fit(20));
    _toolboxExportButton.setRadius(fit(4));
    _toolboxExportButton.setIconSize(QSize(fit(15),fit(15)));
    _toolboxExportButton.setIcon(QIcon(QPixmap(":/resources/images/left-arrow.png").transformed(QTransform().rotate(90))));
    _toolboxExportButton.setDisabled(true);
    connect(&_toolboxExportButton, SIGNAL(clicked(bool)), SLOT(toolboxExportButtonClicked()) );

    _toolboxAdderAreaButtonSideHLay.addStretch();
    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxAddButton);
    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxRemoveButton);
    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxEditButton);
    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxImportButton);
    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxExportButton);
    _toolboxAdderAreaButtonSideHLay.addStretch();

    _toolboxUrlBox.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _toolboxUrlBox.setFixedHeight(fit(30));
    _toolboxUrlBox.setPlaceholderText("Icon url");
    _toolboxUrlBox.setText(":/resources/images/item.png");
    _toolboxUrlBox.setDisabled(true);
    _toolboxUrlBox.setHidden(true);
    connect(&_toolboxUrlBox, SIGNAL(textChanged(QString)),
            SLOT(handleToolboxUrlboxChanges(QString)));

    _toolBoxNameBox.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _toolBoxNameBox.setFixedHeight(fit(30));
    _toolBoxNameBox.setPlaceholderText("Tool name");
    _toolBoxNameBox.setDisabled(true);
    _toolBoxNameBox.setHidden(true);
    connect(&_toolBoxNameBox, SIGNAL(textChanged(QString)),
            SLOT(handleToolboxNameboxChanges(QString)));

    _toolboxAdderAreaEditingLayout.addWidget(&_toolBoxNameBox);
    _toolboxAdderAreaEditingLayout.addWidget(&_toolboxUrlBox);
    _toolboxAdderAreaEditingLayout.setSpacing(0);
    _toolboxAdderAreaEditingLayout.setContentsMargins(0,0,0,0);

    _toolboxAdderAreaButtonSideHLay.setSpacing(fit(6));
    _toolboxAdderAreaButtonSideHLay.setContentsMargins(0,0,0,0);
    _toolboxAdderAreaVLay.addLayout(&_toolboxAdderAreaButtonSideHLay);
    _toolboxAdderAreaVLay.addLayout(&_toolboxAdderAreaEditingLayout);
    _toolboxAdderAreaVLay.addStretch();
    _toolboxAdderAreaVLay.setSpacing(fit(6));
    _toolboxAdderAreaVLay.setContentsMargins(fit(6),fit(6),fit(6),fit(6));
    _toolboxAdderAreaWidget.setLayout(&_toolboxAdderAreaVLay);
    _toolboxAdderAreaWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _toolboxAdderAreaWidget.setFixedHeight(fit(32));
    _toolboxAdderAreaWidget.setObjectName("_toolboxAdderAreaWidget");
    _toolboxAdderAreaWidget.setStyleSheet(QString("#_toolboxAdderAreaWidget{background: #4E5C68;border-top:1px solid #405060;}"));

    _toolboxVLay.addWidget(&_toolboxList);
    _toolboxVLay.addWidget(&_toolboxAdderAreaWidget);
    _toolboxVLay.setSpacing(fit(2));
    _toolboxVLay.setContentsMargins(0, fit(2), 0, 0);
    setLayout(&_toolboxVLay);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(&_toolboxList, (void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){
        if (i>=0) {
//            _toolboxUrlBox.setText(dname(_toolboxList.GetUrls(_toolboxList.currentItem())[0].toLocalFile()) + "/icon.png");
//            _toolBoxNameBox.setText(_toolboxList.currentItem()->text());
        }
        _toolBoxNameBox.setEnabled(i>=0);
        _toolboxUrlBox.setEnabled(i>=0);
        if (!_toolboxEditButton.isChecked()) {
            _toolboxEditButton.setEnabled(i>=0);
            _toolboxRemoveButton.setEnabled(i>=0);
            _toolboxExportButton.setEnabled(i>=0);
        }
    });
}

ListWidget* ToolBox::toolboxList()
{
    return &_toolboxList;
}

QSize ToolBox::sizeHint() const
{
    return QSize(fit(190), fit(400));
}

void ToolBox::showAdderArea()
{
    QPropertyAnimation *animation = new QPropertyAnimation(&_toolboxAdderAreaWidget, "minimumHeight");
    animation->setDuration(DURATION);
    animation->setStartValue(fit(32));
    animation->setEndValue(fit(    110));
    animation->setEasingCurve(QEasingCurve::OutExpo);
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));

    QPropertyAnimation *animation2 = new QPropertyAnimation(&_toolboxAdderAreaWidget, "maximumHeight");
    animation2->setDuration(DURATION);
    animation2->setStartValue(fit(32));
    animation2->setEndValue(fit(    110));
    animation2->setEasingCurve(QEasingCurve::OutExpo);
    connect(animation2, SIGNAL(finished()), animation2, SLOT(deleteLater()));

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(animation);
    group->addAnimation(animation2);
    connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));
    group->start();

    _toolboxAdderAreaEditingLayout.setSpacing(fit(6));
    _toolboxAdderAreaEditingLayout.setContentsMargins(0,0,0,0);
    _toolBoxNameBox.setHidden(false);
    _toolboxUrlBox.setHidden(false);
    _toolboxAddButton.setDisabled(true);
    _toolboxRemoveButton.setDisabled(true);
    _toolboxImportButton.setDisabled(true);
    _toolboxExportButton.setDisabled(true);
}

void ToolBox::hideAdderArea()
{
    QPropertyAnimation *animation = new QPropertyAnimation(&_toolboxAdderAreaWidget, "minimumHeight");
    animation->setDuration(DURATION);
    animation->setStartValue(fit(    110));
    animation->setEndValue(fit(32));
    animation->setEasingCurve(QEasingCurve::OutExpo);
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));

    QPropertyAnimation *animation2 = new QPropertyAnimation(&_toolboxAdderAreaWidget, "maximumHeight");
    animation2->setDuration(DURATION);
    animation2->setStartValue(fit(    110));
    animation2->setEndValue(fit(32));
    animation2->setEasingCurve(QEasingCurve::OutExpo);
    connect(animation2, SIGNAL(finished()), animation2, SLOT(deleteLater()));

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(animation);
    group->addAnimation(animation2);
    connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));
    group->start();

    _toolboxAdderAreaEditingLayout.setSpacing(0);
    _toolboxAdderAreaEditingLayout.setContentsMargins(0,0,0,0);
    _toolboxAddButton.setEnabled(true);
    _toolboxRemoveButton.setEnabled(true);
    _toolboxImportButton.setEnabled(true);
    _toolboxExportButton.setEnabled(true);
    _toolboxEditButton.setChecked(false);
    _toolBoxNameBox.setHidden(true);
    _toolboxUrlBox.setHidden(true);
}

void ToolBox::handleToolboxUrlboxChanges(const QString& text)
{
    QPixmap pixmap;
    pixmap.loadFromData(dlfile(text));
    if (pixmap.isNull()) return;
    auto icon = dname(_toolboxList.GetUrls(_toolboxList.currentItem())[0].toLocalFile()) + "/icon.png";
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    if (!pixmap.save(&buffer,"PNG")) return;
    buffer.close();
    if (!wrfile(icon, bArray)) return;
    _toolboxList.currentItem()->setIcon(QIcon(icon));
}

void ToolBox::handleToolboxNameboxChanges(QString /*name*/)
{
    //	if (name == _toolboxList.currentItem()->text() || name == "") return;

    //	int count = 1;
    //	for (int i = 0; i < _toolboxList.count(); i++) {
    //		if (_toolboxList.item(i)->text() == name) {
    //			if (count > 1) {
    //				name.remove(name.size() - 1, 1);
    //			}
    //			i = -1;
    //			count++;
    //			name += QString::number(count);
    //		}
    //	}

    //	auto from = ToolsManager::toolsDir() + "/" + _toolboxList.currentItem()->text();
    //	auto to = ToolsManager::toolsDir() + "/" + name;
    //	if (!rn(from, to)) qFatal("ToolBox : Error occurred");

    //	_toolboxList.currentItem()->setText(name);

    //	QList<QUrl> urls;
    //	urls << QUrl::fromLocalFile(to + "/main.qml");
    //	_toolboxList.RemoveUrls(_toolboxList.currentItem());
    //	_toolboxList.AddUrls(_toolboxList.currentItem(),urls);

    //	for (int i = 0; i < m_ItemUrls.size(); i++) {
    //		if (m_ItemUrls[i].toLocalFile() == (from+"/main.qml")) {
    //			m_ItemUrls[i] = QUrl::fromLocalFile(to+"/main.qml");
    //		}
    //	}

    //	qmlEditor->updateCacheForRenamedEntry(from, to, true);
}

void ToolBox::toolboxEditButtonToggled(bool checked)
{
    if (checked)
        showAdderArea();
    else
        hideAdderArea();
}

void ToolBox::toolboxRemoveButtonClicked()
{
    //	if (_toolboxList.currentRow() < 0) return;
    //	auto name = _toolboxList.currentItem()->text();
    //	QMessageBox msgBox;
    //	msgBox.setText(QString("<b>This will remove %1 from Tool Library and Dashboard.</b>").arg(name));
    //	msgBox.setInformativeText("Do you want to continue?");
    //	msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    //	msgBox.setDefaultButton(QMessageBox::No);
    //	msgBox.setIcon(QMessageBox::Warning);
    //	const int ret = msgBox.exec();
    //	switch (ret) {
    //		case QMessageBox::Yes: {
    //			qmlEditor->clearCacheFor(ToolsManager::toolsDir() + separator() + name, true);
    //			rm(ToolsManager::toolsDir() + separator() + name);
    //			_toolboxList.RemoveUrls(_toolboxList.currentItem());
    //			delete _toolboxList.takeItem(_toolboxList.currentRow());

    //			for (int i = 0; i < m_ItemUrls.size(); i++) {
    //				if (m_ItemUrls[i].toLocalFile() == (ToolsManager::toolsDir() + separator() + name + "/main.qml")) {
    //					auto items = GetAllChildren(m_Items[i]);
    //					for (auto item : items) {
    //						if (m_Items.contains(item)) {
    //							SaveManager::removeSave(designWidget.rootContext()->nameForObject(item));
    //							SaveManager::removeParentalRelationship(designWidget.rootContext()->nameForObject(item));
    //							designWidget.rootContext()->setContextProperty(
    //										designWidget.rootContext()->nameForObject(item), 0);
    //							int j = m_Items.indexOf(item);
    //							m_Items.removeOne(item);
    //							m_ItemUrls.removeAt(j);
    //							item->deleteLater();
    //						}
    //					}
    //				}
    //			}
    //			HideSelectionTools();
    //			break;
    //		} default: {
    //			// Do nothing
    //			break;
    //		}
    //	}
}

void ToolBox::toolboxAddButtonClicked()
{
    int count = 1;
    auto name = QString("Item%1").arg(count);
    for (int i = 0; i < _toolboxList.count(); i++) {
        if (_toolboxList.item(i)->text() == name) {
            name.remove(name.size() - 1, 1);
            i = -1;
            count++;
            name += QString::number(count);
        }
    }

    auto itemPath = ToolsManager::toolsDir() + separator() + name;
    auto iconPath = itemPath + separator() + DIR_THIS + separator() + "icon.png";
    auto qmlPath = itemPath + separator() + DIR_THIS + separator() + "main.png";

    if (!mkdir(itemPath) || !cp(DIR_QRC_ITEM, itemPath, true, true))
        return;

    SaveManager::refreshToolUid(itemPath);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(qmlPath);

    QListWidgetItem* item = new QListWidgetItem(QIcon(iconPath), name);
    _toolboxList.insertItem(0, item);
    _toolboxList.AddUrls(item, urls);
    _toolboxList.setCurrentRow(0);
    _toolboxEditButton.setChecked(true);
}

void ToolBox::toolboxExportButtonClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    if (dialog.exec()) {
        auto dir = dname(dname(_toolboxList.GetUrls(_toolboxList.currentItem())[0].toLocalFile()));
        auto toolName = _toolboxList.currentItem()->text();
        if (!rm(dialog.selectedFiles().at(0) + separator() + toolName + ".zip")) return;
        Zipper::compressDir(dir, dialog.selectedFiles().at(0) + separator() + toolName + ".zip");
        QMessageBox::information(this, "Done", "Tool export is done.");
    }
}

void ToolBox::toolboxImportButtonClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(tr("Zip files (*.zip)"));
    dialog.setViewMode(QFileDialog::Detail);
    if (dialog.exec()) {
        handleImports(dialog.selectedFiles());
        QMessageBox::information(this, "Done", "Tool import is done.");
    }
}

void ToolBox::handleImports(const QStringList& fileNames)
{
    for (auto fileName : fileNames) {
        auto name = fname(fileName.remove(fileName.size() - 4, 4));
        int count = 1;
        for (int i = 0; i < _toolboxList.count(); i++) {
            if (_toolboxList.item(i)->text() == name) {
                if (count > 1) {
                    name.remove(name.size() - 1, 1);
                }
                i = -1;
                count++;
                name += QString::number(count);
            }
        }

        auto itemPath = ToolsManager::toolsDir() + separator() + name;
        if (!mkdir(itemPath) || !Zipper::extractZip(rdfile(fileName + ".zip"), itemPath))
            return;

        if (!SaveManager::isOwctrl(itemPath)) {
            rm(itemPath);
            return;
        }

        SaveManager::refreshToolUid(itemPath);
        ToolsManager::addTool(name);
    }
}
