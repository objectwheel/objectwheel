#include <toolbox.h>
#include <toolsmanager.h>
#include <zipper.h>
#include <filemanager.h>
#include <savemanager.h>
#include <css.h>

#include <QStyledItemDelegate>
#include <QApplication>
#include <QItemDelegate>
#include <QTreeView>

#define DURATION 500

using namespace Fit;

//!
//! *********************** [ToolboxDelegate] ***********************
//!

class ToolboxDelegate: public QStyledItemDelegate
{
        Q_OBJECT

    public:
        ToolboxDelegate(QTreeView* view, QWidget* parent);

        void paint(QPainter* painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

        QSize sizeHint(const QStyleOptionViewItem &opt,
                       const QModelIndex &index) const override;

    private:
        QTreeView* m_view;
};

ToolboxDelegate::ToolboxDelegate(QTreeView* view, QWidget* parent)
    : QStyledItemDelegate(parent)
    , m_view(view)
{
}

void ToolboxDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    const QAbstractItemModel* model = index.model();
    Q_ASSERT(model);

    if (!model->parent(index).isValid()) {
        // this is a top-level item.
        QStyleOptionButton buttonOption;

        buttonOption.state = option.state;
#ifdef Q_OS_MACOS
        buttonOption.state |= QStyle::State_Raised;
#endif
        buttonOption.state &= ~QStyle::State_HasFocus;

        buttonOption.rect = option.rect;
        buttonOption.palette = option.palette;
        buttonOption.features = QStyleOptionButton::None;

        painter->save();
        QColor buttonColor(230, 230, 230);
        QBrush buttonBrush = option.palette.button();
        if (!buttonBrush.gradient() && buttonBrush.texture().isNull())
            buttonColor = buttonBrush.color();
        QColor outlineColor = buttonColor.darker(150);
        QColor highlightColor = buttonColor.lighter(130);

        // Only draw topline if the previous item is expanded
        QModelIndex previousIndex = model->index(index.row() - 1, index.column());
        bool drawTopline = (index.row() > 0 && m_view->isExpanded(previousIndex));
        int highlightOffset = drawTopline ? 1 : 0;

        QLinearGradient gradient(option.rect.topLeft(), option.rect.bottomLeft());
        gradient.setColorAt(0, QColor("#EAEEF1"));
        gradient.setColorAt(1, QColor("#D0D4D7"));

        painter->setPen(Qt::NoPen);
        painter->setBrush(gradient);
        painter->drawRect(option.rect);
        QPen p(highlightColor);
        p.setWidthF(0.5);
        painter->setPen(p);
        painter->drawLine(option.rect.topLeft() + QPoint(0, highlightOffset),
                          option.rect.topRight() + QPoint(0, highlightOffset));
        p.setColor(outlineColor);
        painter->setPen(p);
        if (drawTopline)
            painter->drawLine(option.rect.topLeft(), option.rect.topRight());
        painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
        painter->restore();

        QStyleOption branchOption;
        static const int i = 9; // ### hardcoded in qcommonstyle.cpp
        QRect r = option.rect;
        branchOption.rect = QRect(r.left() + i/2, r.top() + (r.height() - i)/2, i, i);
        branchOption.state = QStyle::State_Children;

        if (m_view->isExpanded(index))
            branchOption.state |= QStyle::State_Open;

        qApp->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, painter, m_view);

        // draw text
        QRect textrect = QRect(r.left() + i*2, r.top(), r.width() - ((5*i)/2), r.height());
        QString text = elidedText(option.fontMetrics, textrect.width(), Qt::ElideMiddle,
                                  model->data(index, Qt::DisplayRole).toString());
        m_view->style()->drawItemText(painter, textrect, Qt::AlignCenter,
                                      option.palette, m_view->isEnabled(), text);

    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ToolboxDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;
    QSize sz = QStyledItemDelegate::sizeHint(opt, index) + QSize(2, 2);
    return sz;
}

//!
//! *********************** [ToolBox] ***********************
//!

ToolBox::ToolBox(QWidget* parent) : QWidget(parent)
{
    setAutoFillBackground(true);
    QPalette p(palette());
    p.setColor(QPalette::Window, QColor("#E0E4E7"));
    setPalette(p);

    QPalette p2(_toolboxTree.palette());
    p2.setColor(QPalette::Base, QColor("#F3F7FA"));
    p2.setColor(QPalette::Highlight, QColor("#d0d4d7"));
    p2.setColor(QPalette::Text, QColor("#202427"));
    _toolboxTree.setPalette(p2);

    _toolboxTree.setIndicatorButtonVisible(true);
    _toolboxTree.setIconSize(fit({24, 24}));
    _toolboxTree.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _toolboxTree.setFocusPolicy(Qt::NoFocus);
    _toolboxTree.setIndentation(0);
    _toolboxTree.setRootIsDecorated(false);
    _toolboxTree.setSortingEnabled(true);
    _toolboxTree.setColumnCount(1);
    _toolboxTree.header()->hide();
    _toolboxTree.header()->setSectionResizeMode(QHeaderView::Stretch);
    _toolboxTree.setTextElideMode(Qt::ElideMiddle);
    _toolboxTree.verticalScrollBar()->setStyleSheet(CSS::ScrollBar);

    _toolboxTree.setDragEnabled(true);
    _toolboxTree.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _toolboxTree.setDragDropMode(QAbstractItemView::DragOnly);
    _toolboxTree.setSelectionBehavior(QAbstractItemView::SelectRows);
    _toolboxTree.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _toolboxTree.setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    _toolboxTree.setItemDelegate(new ToolboxDelegate(&_toolboxTree, &_toolboxTree));

    _toolboxTree.indicatorButton()->setIcon(QIcon(":/resources/images/right-arrow.png"));
    _toolboxTree.indicatorButton()->setColor(QColor("#0D74C8"));
    _toolboxTree.indicatorButton()->setRadius(fit(7));
    _toolboxTree.indicatorButton()->setIconSize(QSize(fit(10), fit(10)));
    _toolboxTree.indicatorButton()->setFixedSize(fit(15), fit(15));
    connect(&_toolboxTree, &QTreeWidget::itemPressed, this, &ToolBox::handleMousePress);
    connect(&_toolboxTree, &QTreeWidget::itemSelectionChanged, this, &ToolBox::handleSelectionChange);

    _searchEdit.setPlaceholderText("Filter");
    _searchEdit.setClearButtonEnabled(true);
    connect(&_searchEdit, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));

    _toolboxAddButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _toolboxAddButton.setColor("#6BB64B");
    _toolboxAddButton.setFixedSize(fit(17),fit(17));
    _toolboxAddButton.setRadius(fit(8));
    _toolboxAddButton.setIconSize(QSize(fit(13),fit(13)));
    _toolboxAddButton.setIcon(QIcon(":/resources/images/plus.png"));
    connect(&_toolboxAddButton, SIGNAL(clicked(bool)), SLOT(toolboxAddButtonClicked()) );

    _toolboxRemoveButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _toolboxRemoveButton.setColor("#C61717");
    _toolboxRemoveButton.setFixedSize(fit(17),fit(17));
    _toolboxRemoveButton.setRadius(fit(8));
    _toolboxRemoveButton.setIconSize(QSize(fit(13),fit(13)));
    _toolboxRemoveButton.setIcon(QIcon(":/resources/images/minus.png"));
    _toolboxRemoveButton.setDisabled(true);
    connect(&_toolboxRemoveButton, SIGNAL(clicked(bool)), SLOT(toolboxRemoveButtonClicked()) );

    _toolboxEditButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _toolboxEditButton.setColor("#697D8C");
    _toolboxEditButton.setFixedSize(fit(17),fit(17));
    _toolboxEditButton.setRadius(fit(8));
    _toolboxEditButton.setCheckedColor(QColor("#6BB64B"));
    _toolboxEditButton.setCheckable(true);
    _toolboxEditButton.setIconSize(QSize(fit(13),fit(13)));
    _toolboxEditButton.setIcon(QIcon(":/resources/images/edit.png"));
    _toolboxEditButton.setDisabled(true);
    connect(&_toolboxEditButton, SIGNAL(toggled(bool)), SLOT(toolboxEditButtonToggled(bool)) );

    _toolboxImportButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _toolboxImportButton.setColor("#3498DB");
    _toolboxImportButton.setFixedSize(fit(17),fit(17));
    _toolboxImportButton.setRadius(fit(8));
    _toolboxImportButton.setIconSize(QSize(fit(13),fit(13)));
    _toolboxImportButton.setIcon(QIcon(QPixmap(":/resources/images/left-arrow.png").transformed(QTransform().rotate(-90))));
    connect(&_toolboxImportButton, SIGNAL(clicked(bool)), SLOT(toolboxImportButtonClicked()) );

    _toolboxExportButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _toolboxExportButton.setColor("#E8BC43");
    _toolboxExportButton.setFixedSize(fit(17),fit(17));
    _toolboxExportButton.setRadius(fit(8));
    _toolboxExportButton.setIconSize(QSize(fit(13),fit(13)));
    _toolboxExportButton.setIcon(QIcon(QPixmap(":/resources/images/left-arrow.png").transformed(QTransform().rotate(90))));
    _toolboxExportButton.setDisabled(true);
    connect(&_toolboxExportButton, SIGNAL(clicked(bool)), SLOT(toolboxExportButtonClicked()) );

    _toolboxAdderAreaButtonSideHLay.setSpacing(0);
    _toolboxAdderAreaButtonSideHLay.setContentsMargins(0, 0, 0, 0);
    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxAddButton);
    _toolboxAdderAreaButtonSideHLay.addStretch();
    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxRemoveButton);
    _toolboxAdderAreaButtonSideHLay.addStretch();
    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxEditButton);
    _toolboxAdderAreaButtonSideHLay.addStretch();
    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxImportButton);
    _toolboxAdderAreaButtonSideHLay.addStretch();
    _toolboxAdderAreaButtonSideHLay.addWidget(&_toolboxExportButton);

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
    _toolboxAdderAreaEditingLayout.setContentsMargins(0, 0, 0, 0);

    _toolboxAdderAreaVLay.addLayout(&_toolboxAdderAreaButtonSideHLay);
    _toolboxAdderAreaVLay.addLayout(&_toolboxAdderAreaEditingLayout);
    _toolboxAdderAreaVLay.addStretch();
    _toolboxAdderAreaVLay.setSpacing(fit(2));
    _toolboxAdderAreaVLay.setContentsMargins(0, 0, 0, 0);
    _toolboxAdderAreaWidget.setLayout(&_toolboxAdderAreaVLay);
    _toolboxAdderAreaWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _toolboxAdderAreaWidget.setFixedHeight(fit(17));

    _toolboxVLay.addWidget(&_searchEdit);
    _toolboxVLay.addWidget(&_toolboxTree);
    _toolboxVLay.addWidget(&_toolboxAdderAreaWidget);
    _toolboxVLay.setSpacing(fit(2));
    _toolboxVLay.setContentsMargins(fit(3), fit(3), fit(3), fit(3));

    setLayout(&_toolboxVLay);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ToolBox::handleMousePress(QTreeWidgetItem* item)
{
    if (item == 0)
        return;

    if (QApplication::mouseButtons() != Qt::LeftButton)
        return;

    if (item->parent() == 0) {
        _toolboxTree.setItemExpanded(item, !_toolboxTree.isItemExpanded(item));
        return;
    }
}

ToolboxTree* ToolBox::toolboxTree()
{
    return &_toolboxTree;
}

void ToolBox::filterList(const QString& filter)
{
    for (int i = 0; i < _toolboxTree.topLevelItemCount(); i++) {
        auto tli = _toolboxTree.topLevelItem(i);
        auto tlv = false;

        for (int j = 0; j < tli->childCount(); j++) {
            auto tci = tli->child(j);
            auto v = filter.isEmpty() ? true :
                tci->text(0).contains(filter, Qt::CaseInsensitive);

            tci->setHidden(!v);
            if (v)
                tlv = v;
        }

        auto v = filter.isEmpty() ? true : tlv;
        tli->setHidden(!v);
    }
}

void ToolBox::showAdderArea()
{
    QPropertyAnimation* animation = new QPropertyAnimation(&_toolboxAdderAreaWidget, "minimumHeight");
    animation->setDuration(DURATION);
    animation->setStartValue(fit(17));
    animation->setEndValue(fit(88));
    animation->setEasingCurve(QEasingCurve::OutExpo);
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));

    QPropertyAnimation* animation2 = new QPropertyAnimation(&_toolboxAdderAreaWidget, "maximumHeight");
    animation2->setDuration(DURATION);
    animation2->setStartValue(fit(17));
    animation2->setEndValue(fit(88));
    animation2->setEasingCurve(QEasingCurve::OutExpo);
    connect(animation2, SIGNAL(finished()), animation2, SLOT(deleteLater()));

    QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
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
    QPropertyAnimation* animation = new QPropertyAnimation(&_toolboxAdderAreaWidget, "minimumHeight");
    animation->setDuration(DURATION);
    animation->setStartValue(fit(88));
    animation->setEndValue(fit(17));
    animation->setEasingCurve(QEasingCurve::OutExpo);
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));

    QPropertyAnimation* animation2 = new QPropertyAnimation(&_toolboxAdderAreaWidget, "maximumHeight");
    animation2->setDuration(DURATION);
    animation2->setStartValue(fit(88));
    animation2->setEndValue(fit(17));
    animation2->setEasingCurve(QEasingCurve::OutExpo);
    connect(animation2, SIGNAL(finished()), animation2, SLOT(deleteLater()));

    QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
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

void ToolBox::handleSelectionChange()
{
    auto item = _toolboxTree.currentItem();
    bool enable = (_toolboxTree.selectedItems().size() == 1 && item->parent() != 0);

    if (enable) {
        _toolboxUrlBox.setText(dname(_toolboxTree.urls(_toolboxTree.currentItem())[0].toLocalFile()) + "/icon.png");
        _toolBoxNameBox.setText(_toolboxTree.currentItem()->text(0));
    }
    _toolBoxNameBox.setEnabled(enable);
    _toolboxUrlBox.setEnabled(enable);
    if (!_toolboxEditButton.isChecked()) {
        _toolboxEditButton.setEnabled(enable);
        _toolboxRemoveButton.setEnabled(enable);
        _toolboxExportButton.setEnabled(enable);
    }
}

void ToolBox::handleToolboxUrlboxChanges(const QString& text)
{
    QPixmap pixmap;
    pixmap.loadFromData(dlfile(text));
    if (pixmap.isNull()) return;
    auto icon = dname(_toolboxTree.urls(_toolboxTree.currentItem())[0].toLocalFile()) + "/icon.png";
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    if (!pixmap.save(&buffer,"PNG")) return;
    buffer.close();
    if (!wrfile(icon, bArray)) return;
    _toolboxTree.currentItem()->setIcon(0, QIcon(icon));
}

void ToolBox::handleToolboxNameboxChanges(QString /*name*/)
{
    //	if (name == _toolboxTree.currentItem()->text() || name == "") return;

    //	int count = 1;
    //	for (int i = 0; i < _toolboxTree.count(); i++) {
    //		if (_toolboxTree.item(i)->text() == name) {
    //			if (count > 1) {
    //				name.remove(name.size() - 1, 1);
    //			}
    //			i = -1;
    //			count++;
    //			name += QString::number(count);
    //		}
    //	}

    //	auto from = ToolsManager::toolsDir() + "/" + _toolboxTree.currentItem()->text();
    //	auto to = ToolsManager::toolsDir() + "/" + name;
    //	if (!rn(from, to)) qFatal("ToolBox : Error occurred");

    //	_toolboxTree.currentItem()->setText(name);

    //	QList<QUrl> urls;
    //	urls << QUrl::fromLocalFile(to + "/main.qml");
    //	_toolboxTree.RemoveUrls(_toolboxTree.currentItem());
    //	_toolboxTree.addUrls(_toolboxTree.currentItem(),urls);

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
    //	if (_toolboxTree.currentRow() < 0) return;
    //	auto name = _toolboxTree.currentItem()->text();
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
    //			_toolboxTree.RemoveUrls(_toolboxTree.currentItem());
    //			delete _toolboxTree.takeItem(_toolboxTree.currentRow());

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
    while(_toolboxTree.contains(name)) {
        name.remove(name.size() - 1, 1);
        count++;
        name += QString::number(count);
    }

    auto itemPath = ToolsManager::toolsDir() + separator() + name;
    auto iconPath = itemPath + separator() + DIR_THIS + separator() + "icon.png";
    auto qmlPath = itemPath + separator() + DIR_THIS + separator() + "main.png";

    if (!mkdir(itemPath) || !cp(DIR_QRC_ITEM, itemPath, true, true))
        return;

    SaveManager::refreshToolUid(itemPath);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(qmlPath);

    auto category = SaveManager::toolCategory(itemPath);
    auto topItem = _toolboxTree.categoryItem(category);
    if (!topItem) {
        topItem = new QTreeWidgetItem;
        topItem->setText(0, category);
        _toolboxTree.addTopLevelItem(topItem);
        topItem->setExpanded(true);
    }

    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(0, name);
    item->setIcon(0, QIcon(iconPath));
    topItem->addChild(item);
    _toolboxTree.addUrls(item, urls);
    _toolboxEditButton.setChecked(true);
}

void ToolBox::toolboxExportButtonClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    if (dialog.exec()) {
        auto dir = dname(dname(_toolboxTree.urls(_toolboxTree.currentItem())[0].toLocalFile()));
        auto toolName = _toolboxTree.currentItem()->text(0);
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
        while(toolboxTree()->contains(name)) {
            if (count > 1)
                name.remove(name.size() - 1, 1);
            count++;
            name += QString::number(count);
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

QSize ToolBox::sizeHint() const
{
    return QSize(fit(200), fit(3000));
}

#include "toolbox.moc"
