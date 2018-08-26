#include <globalresourcespane.h>
#include <focuslesslineedit.h>
#include <projectmanager.h>
#include <saveutils.h>
#include <filemanager.h>
#include <wfw.h>
#include <transparentstyle.h>
#include <utilsicons.h>

#include <QComboBox>
#include <QPainter>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QFileSystemModel>
#include <QToolBar>
#include <QToolButton>
#include <QLabel>

namespace {
const int ROW_HEIGHT = 21;

void initPalette(QWidget* widget)
{    
    QPalette palette(widget->palette());
    palette.setColor(QPalette::Light, "#bf5861");
    palette.setColor(QPalette::Dark, "#b05159");
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::Text, "#331719");
    palette.setColor(QPalette::BrightText, Qt::white);
    palette.setColor(QPalette::WindowText, "#331719");
    palette.setColor(QPalette::AlternateBase, "#f7e6e8");
    widget->setPalette(palette);
}

void fillBackground(QPainter* painter, const QStyleOptionViewItem& option, int row, bool verticalLine)
{
    painter->save();

    bool isSelected = option.state & QStyle::State_Selected;
    const QPalette& pal = option.palette;
    const QRectF& rect = option.rect;

    QPainterPath path;
    path.addRect(rect);
    painter->setClipPath(path);
    painter->setClipping(true);

    // Fill background
    if (isSelected) {
        painter->fillRect(rect, pal.highlight());
    } else {
        if (row % 2)
            painter->fillRect(rect, pal.alternateBase());
        else
            painter->fillRect(rect, pal.base());
    }

    // Draw top and bottom lines
    QColor lineColor(isSelected ? pal.highlightedText().color() : pal.text().color().lighter(210));
    lineColor.setAlpha(50);
    painter->setPen(row == 0 ? "#c0c0c0" : lineColor);
    if (row == 0)
        painter->drawLine(rect.topLeft() + QPointF{0.5, 0.5}, rect.topRight() - QPointF{0.5, -0.5});
    else
        painter->drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
    painter->drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});

    // Draw vertical line
    if (verticalLine) {
        painter->drawLine(rect.topRight() + QPointF(-0.5, 0.5),
                          rect.bottomRight() + QPointF(-0.5, -0.5));
    }

    painter->restore();
}
}

class GlobalListDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit GlobalListDelegate(GlobalResourcesPane* parent) : QStyledItemDelegate(parent)
      , m_globalPane(parent)
    {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        const bool isSelected = option.state & QStyle::State_Selected;
        const QAbstractItemModel* model = index.model();
        const QIcon& icon = model->data(index, Qt::DecorationRole).value<QIcon>();

        QRectF iconRect({}, QSizeF{option.decorationSize});
        iconRect.moveCenter(option.rect.center());
        iconRect.moveLeft(option.rect.left() + 5);

        fillBackground(painter, option,
                       /*calculateVisibleRow(m_globalPane->itemFromIndex(index), m_globalPane)*/ m_globalPane->d_func()->viewIndex(index),
                       index.column() == 0);

        // Draw icon
        const QPixmap& iconPixmap = icon.pixmap(wfw(m_globalPane), option.decorationSize,
                                                isSelected ? QIcon::Selected : QIcon::Normal);
        painter->drawPixmap(iconRect, iconPixmap, iconPixmap.rect());

        // Draw text
        if (model->data(index, Qt::UserRole).toBool())
            painter->setPen(option.palette.link().color());
        else if (isSelected)
            painter->setPen(option.palette.highlightedText().color());
        else
            painter->setPen(option.palette.text().color());

        QRectF textRect;
        if (index.column() == 0)
            textRect = option.rect.adjusted(option.decorationSize.width() + 10, 0, 0, 0);
        else
            textRect = option.rect.adjusted(5, 0, 0, 0);

        const QString& text = index.data(Qt::DisplayRole).toString();
        painter->drawText(textRect,
                          option.fontMetrics.elidedText(text, Qt::ElideMiddle, textRect.width()),
                          QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

        painter->restore();
    }


    QSize sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& index) const override
    {
        const QSize& size = QStyledItemDelegate::sizeHint(opt, index);
        return QSize(size.width(), ROW_HEIGHT);
    }

private:
    GlobalResourcesPane* m_globalPane;
};

GlobalResourcesPane::GlobalResourcesPane(QWidget* parent) : QTreeView(parent)
  , m_searchEdit(new FocuslessLineEdit(this))
  , m_fileSystemModel(new QFileSystemModel(this))
  , m_toolbar(new QToolBar(this))
  , m_modeComboBox(new QComboBox(header()))
  , m_upButton(new QToolButton)
  , m_homeButton(new QToolButton)
  , m_cutButton(new QToolButton)
  , m_copyButton(new QToolButton)
  , m_pasteButton(new QToolButton)
  , m_deleteButton(new QToolButton)
  , m_renameButton(new QToolButton)
  , m_newFileButton(new QToolButton)
  , m_newFolderButton(new QToolButton)
  , m_downloadFileButton(new QToolButton)
{
    initPalette(this);

    QFont fontMedium(font());
    fontMedium.setWeight(QFont::Medium);

    header()->setFont(fontMedium);
    header()->setFixedHeight(23);
    header()->setDefaultSectionSize(1);
    header()->setMinimumSectionSize(1);
    header()->setSectionsMovable(false);

    setIndentation(16);
    setIconSize({15, 15});
    setAcceptDrops(true);
    setDragEnabled(false);
    setSortingEnabled(false);
    setRootIsDecorated(true);
    setItemsExpandable(true);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(true);
    setItemDelegate(new GlobalListDelegate(this));
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeView::SelectRows);
    setSelectionMode(QTreeView::SingleSelection);
    setDragDropMode(QAbstractItemView::NoDragDrop);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QTreeView::ScrollPerPixel);
    setHorizontalScrollMode(QTreeView::ScrollPerPixel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet(
                QString {
                    "QTreeView {"
                    "    border: 1px solid %1;"
                    "} QHeaderView::section {"
                    "    padding-left: 0px;"
                    "    padding-top: 3px;"
                    "    padding-bottom: 3px;"
                    "    color: %4;"
                    "    border: none;"
                    "    border-bottom: 1px solid %1;"
                    "    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
                    "                                stop:0 %2, stop:1 %3);"
                    "}"
                }
                .arg(palette().text().color().lighter(270).name())
                .arg(palette().light().color().name())
                .arg(palette().dark().color().name())
                .arg(palette().brightText().color().name())
                );

    QPalette mp(m_modeComboBox->palette());
    mp.setColor(QPalette::Text, Qt::white);
    mp.setColor(QPalette::WindowText, Qt::white);
    mp.setColor(QPalette::ButtonText, Qt::white);
    m_modeComboBox->setPalette(mp);

    m_modeComboBox->addItem("      " + tr("Viewer"));
    m_modeComboBox->addItem("      " + tr("Explorer"));

    //    m_deleteButton->setDisabled(true);
    //    m_copyButton->setDisabled(true);

    m_upButton->setCursor(Qt::PointingHandCursor);
    m_homeButton->setCursor(Qt::PointingHandCursor);
    m_cutButton->setCursor(Qt::PointingHandCursor);
    m_copyButton->setCursor(Qt::PointingHandCursor);
    m_pasteButton->setCursor(Qt::PointingHandCursor);
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    m_renameButton->setCursor(Qt::PointingHandCursor);
    m_newFileButton->setCursor(Qt::PointingHandCursor);
    m_newFolderButton->setCursor(Qt::PointingHandCursor);
    m_downloadFileButton->setCursor(Qt::PointingHandCursor);
    m_modeComboBox->setCursor(Qt::PointingHandCursor);

    m_upButton->setToolTip("Go up.");
    m_homeButton->setToolTip("Go home.");
    m_cutButton->setToolTip("Cut files/folders.");
    m_copyButton->setToolTip("Copy files/folders.");
    m_pasteButton->setToolTip("Paste files/folders.");
    m_deleteButton->setToolTip("Delete files/folders.");
    m_renameButton->setToolTip("Rename file/folder.");
    m_newFileButton->setToolTip("New file.");
    m_newFolderButton->setToolTip("New folder.");
    m_downloadFileButton->setToolTip("Download file from url.");
    m_modeComboBox->setToolTip("File Explorer Mode.");

    const Utils::Icon FFF({{QLatin1String(":/utils/images/filtericon.png"), Utils::Theme::BackgroundColorNormal}});

    auto modeIconLabel = new QLabel(m_modeComboBox);
    modeIconLabel->setPixmap(FFF.pixmap());
    modeIconLabel->setFixedSize(16, 16);
    modeIconLabel->move(0, 3);
    modeIconLabel->setScaledContents(true);

    m_upButton->setIcon(Utils::Icons::ARROW_UP.icon());
    m_homeButton->setIcon(Utils::Icons::HOME_TOOLBAR.icon());
    m_cutButton->setIcon(Utils::Icons::CUT_TOOLBAR.icon());
    m_copyButton->setIcon(Utils::Icons::COPY_TOOLBAR.icon());
    m_pasteButton->setIcon(Utils::Icons::PASTE_TOOLBAR.icon());
    m_deleteButton->setIcon(Utils::Icons::DELETE_TOOLBAR.icon());
    m_renameButton->setIcon(Utils::Icons::RENAME.icon());
    m_newFileButton->setIcon(Utils::Icons::FILENEW.icon());
    m_newFolderButton->setIcon(Utils::Icons::FOLDERNEW.icon());
    m_downloadFileButton->setIcon(Utils::Icons::DOWNLOAD.icon());

    connect(m_upButton, &QToolButton::clicked,
            this, &GlobalResourcesPane::onUpButtonClick);
    connect(m_homeButton, &QToolButton::clicked,
            this, &GlobalResourcesPane::onHomeButtonClick);
    connect(m_cutButton, &QToolButton::clicked,
            this, &GlobalResourcesPane::onCutButtonClick);
    connect(m_copyButton, &QToolButton::clicked,
            this, &GlobalResourcesPane::onCopyButtonClick);
    connect(m_pasteButton, &QToolButton::clicked,
            this, &GlobalResourcesPane::onPasteButtonClick);
    connect(m_deleteButton, &QToolButton::clicked,
            this, &GlobalResourcesPane::onDeleteButtonClick);
    connect(m_renameButton, &QToolButton::clicked,
            this, &GlobalResourcesPane::onRenameButtonClick);
    connect(m_newFileButton, &QToolButton::clicked,
            this, &GlobalResourcesPane::onNewFileButtonClick);
    connect(m_newFolderButton, &QToolButton::clicked,
            this, &GlobalResourcesPane::onNewFolderButtonClick);
    connect(m_downloadFileButton, &QToolButton::clicked,
            this, &GlobalResourcesPane::onDownloadButtonClick);
    connect(m_modeComboBox, qOverload<const QString&>(&QComboBox::activated),
            this, &GlobalResourcesPane::onModeChange);

//        QPalette tp(m_toolbar->palette());
//        tp.setColor(QPalette::Window, "#333333");
//        m_toolbar->setAutoFillBackground(true);
//        m_toolbar->setPalette(tp);

    TransparentStyle::attach(m_toolbar);
    TransparentStyle::attach(m_modeComboBox);

    m_upButton->setFixedHeight(22);
    m_homeButton->setFixedHeight(22);
    m_cutButton->setFixedHeight(22);
    m_copyButton->setFixedHeight(22);
    m_pasteButton->setFixedHeight(22);
    m_deleteButton->setFixedHeight(22);
    m_renameButton->setFixedHeight(22);
    m_newFileButton->setFixedHeight(22);
    m_newFolderButton->setFixedHeight(22);
    m_downloadFileButton->setFixedHeight(22);
    m_modeComboBox->setFixedHeight(22);

    m_toolbar->setFixedHeight(24);
    m_toolbar->addWidget(m_upButton);
    m_toolbar->addWidget(m_homeButton);
    m_toolbar->addSeparator();
    m_toolbar->addWidget(m_cutButton);
    m_toolbar->addWidget(m_copyButton);
    m_toolbar->addWidget(m_pasteButton);
    m_toolbar->addWidget(m_deleteButton);
    m_toolbar->addWidget(m_renameButton);
    m_toolbar->addSeparator();
    m_toolbar->addWidget(m_newFileButton);
    m_toolbar->addWidget(m_newFolderButton);
    m_toolbar->addWidget(m_downloadFileButton);

    m_fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

    m_searchEdit->setPlaceholderText("Filter");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(22);
    connect(m_searchEdit, &FocuslessLineEdit::textChanged, this, &GlobalResourcesPane::filterList);

    connect(ProjectManager::instance(), &ProjectManager::started,
            this, &GlobalResourcesPane::onProjectStart);
}

void GlobalResourcesPane::sweep()
{
    // TODO
    m_searchEdit->clear();
    setModel(nullptr);
}

void GlobalResourcesPane::onProjectStart()
{
    m_fileSystemModel->setRootPath(SaveUtils::toGlobalDir(ProjectManager::dir()));
    setModel(m_fileSystemModel);
    setRootIndex(m_fileSystemModel->index(m_fileSystemModel->rootPath()));
    hideColumn(1);
    hideColumn(2);
    hideColumn(3);
}

void GlobalResourcesPane::onModeChange()
{

}

void GlobalResourcesPane::onUpButtonClick()
{
    //    auto up = dname(fileList->currentPath());
    //    auto rootPath = fileList->fileModel()->rootPath();
    //    if (up.size() > rootPath.size() || up == rootPath)
    //        fileList->goPath(up);
}

void GlobalResourcesPane::onHomeButtonClick()
{
    //    fileList->goPath(fileList->fileModel()->rootPath());
}

void GlobalResourcesPane::onCutButtonClick()
{

}

void GlobalResourcesPane::onCopyButtonClick()
{
    //    auto _index = fileList->filterProxyModel()->mapToSource(fileList->currentIndex());
    //    auto index = fileList->fileModel()->index(_index.row(), 0, fileList->
    //                 filterProxyModel()->mapToSource(fileList->rootIndex()));
    //    auto fileName = fileList->fileModel()->fileName(index);
    //    auto filePath = fileList->fileModel()->filePath(index);

    //    if (!index.isValid() || fileName.isEmpty() || filePath.isEmpty())
    //        return;

    //    QMessageBox box;
    //    box.setText("<b>Do you want to make a copy of following file/folder.</b>");
    //    box.setInformativeText("<b>Name: </b>" + fileName);
    //    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    //    box.setDefaultButton(QMessageBox::No);
    //    box.setIcon(QMessageBox::Warning);
    //    const int ret = box.exec();

    //    switch (ret) {
    //        case QMessageBox::Yes: {
    //            if (fileList->fileModel()->isDir(index)) {
    //                auto copyName = fileName;
    //                for (int i = 1; exists(dname(filePath) + separator() + copyName); i++)
    //                    copyName = fileName + QString::number(i);
    //                mkdir(dname(filePath) + separator() + copyName);
    //                cp(filePath, dname(filePath) + separator() + copyName, true);
    //            } else {
    //                QString copyName = fileName;
    //                auto sfx = suffix(fileName);
    //                if (!sfx.isEmpty())
    //                    copyName.remove(copyName.lastIndexOf(sfx) - 1, sfx.size() + 1);

    //                if (sfx.isEmpty()) {
    //                    for (int i = 1; exists(dname(filePath) + separator() + copyName); i++)
    //                        copyName = fileName + QString::number(i);
    //                    QFile::copy(filePath, dname(filePath) + separator() + copyName);
    //                } else {
    //                    auto copy = copyName;
    //                    for (int i = 1; exists(dname(filePath) + separator() + copyName + "." + sfx); i++)
    //                        copyName = copy + QString::number(i);
    //                    QFile::copy(filePath, dname(filePath) + separator() + copyName + "." + sfx);
    //                }
    //            }
    //            break;
    //        } default: {
    //            // Do nothing
    //            break;
    //        }
    //    }
}

void GlobalResourcesPane::onPasteButtonClick()
{

}

void GlobalResourcesPane::onDeleteButtonClick()
{
    //    auto _index = fileList->filterProxyModel()->mapToSource(fileList->currentIndex());
    //    auto index = fileList->fileModel()->index(_index.row(), 0, fileList->
    //                 filterProxyModel()->mapToSource(fileList->rootIndex()));
    //    auto fileName = fileList->fileModel()->fileName(index);
    //    auto filePath = fileList->fileModel()->filePath(index);

    //    if (fileName.startsWith("_") || fileName == "icon.png" || fileName == "main.qml")
    //        return;

    //    if (!index.isValid() || fileName.isEmpty() || filePath.isEmpty())
    //        return;

    //    QMessageBox box;
    //    box.setText("<b>Do you want to delete following file/folder.</b>");
    //    box.setInformativeText("<b>Name: </b>" + fileName);
    //    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    //    box.setDefaultButton(QMessageBox::No);
    //    box.setIcon(QMessageBox::Warning);
    //    const int ret = box.exec();

    //    switch (ret) {
    //        case QMessageBox::Yes: {
    //            if (rm(filePath))
    //                emit parent->fileDeleted(filePath);
    //            break;
    //        } default: {
    //            // Do nothing
    //            break;
    //        }
    //    }
}

void GlobalResourcesPane::onRenameButtonClick()
{
    //    bool ok;
    //    auto _index = fileList->filterProxyModel()->mapToSource(fileList->currentIndex());
    //    auto index = fileList->fileModel()->index(_index.row(), 0, fileList->
    //                 filterProxyModel()->mapToSource(fileList->rootIndex()));
    //    auto filePath = fileList->fileModel()->filePath(index);
    //    auto fileName = fileList->fileModel()->fileName(index);

    //    if (fileName.startsWith("_") || fileName == "icon.png" || fileName == "main.qml")
    //        return;

    //    QString text = QInputDialog::getText(parent, tr("Rename file/folder"),
    //                                         tr("New name:"), QLineEdit::Normal,
    //                                         fileName, &ok);

    //    if (text.startsWith("_") || text == "icon.png" || text == "main.qml")
    //        return;

    //    if (index.isValid() && ok && !text.isEmpty() && text != fileName)
    //        if (rn(filePath, dname(filePath) + separator() + text))
    //            emit parent->fileRenamed(filePath, dname(filePath) + separator() + text);
}

void GlobalResourcesPane::onNewFileButtonClick()
{
    //    bool ok;
    //    auto index = fileList->filterProxyModel()->mapToSource(fileList->rootIndex());
    //    auto path = fileList->fileModel()->filePath(index);
    //    QString text = QInputDialog::getText(parent, tr("Create new file"),
    //                                         tr("File name:"), QLineEdit::Normal,
    //                                         QString(), &ok);

    //    if (text.startsWith("_") || text == "icon.png" || text == "main.qml")
    //        return;

    //    if (index.isValid() && ok && !text.isEmpty() && !exists(path + separator() + text))
    //        mkfile(path + separator() + text);
}

void GlobalResourcesPane::onNewFolderButtonClick()
{
    //    bool ok;
    //    auto index = fileList->filterProxyModel()->mapToSource(fileList->rootIndex());
    //    auto path = fileList->fileModel()->filePath(index);
    //    QString text = QInputDialog::getText(parent, tr("Create new folder"),
    //                                         tr("Folder name:"), QLineEdit::Normal,
    //                                         QString(), &ok);

    //    if (text.startsWith("_") || text == "icon.png" || text == "main.qml")
    //        return;

    //    if (index.isValid() && ok && !text.isEmpty() && !exists(path + separator() + text))
    //        mkdir(path + separator() + text);
}

void GlobalResourcesPane::onDownloadButtonClick()
{
    //    bool ok, ok_2;
    //    auto index = fileList->filterProxyModel()->mapToSource(fileList->rootIndex());
    //    auto path = fileList->fileModel()->filePath(index);
    //    QString text = QInputDialog::getText(parent, tr("Download file"),
    //                                         tr("Url:"), QLineEdit::Normal,
    //                                         QString(), &ok);

    //    if (!ok || text.isEmpty())
    //        return;

    //    QString text_2 = QInputDialog::getText(parent, tr("Download file"),
    //                                         tr("File name:"), QLineEdit::Normal,
    //                                         QString(), &ok_2);

    //    if (text_2.startsWith("_") || text_2 == "icon.png" || text_2 == "main.qml")
    //        return;

    //    if (index.isValid() && ok_2 && !text_2.isEmpty() && !exists(path + separator() + text_2)) {
    //        const auto& data = dlfile(text);
    //        if (data.isEmpty())
    //            return;
    //        wrfile(path + separator() + text_2, data);
    //    }
}

void GlobalResourcesPane::onFileSelectionChange()
{
    //    auto _index = fileList->filterProxyModel()->mapToSource(fileList->currentIndex());
    //    auto index = fileList->fileModel()->index(_index.row(), 0, fileList->
    //                 filterProxyModel()->mapToSource(fileList->rootIndex()));
    //    deleteButton->setEnabled(index.isValid());
    //    copyButton->setEnabled(index.isValid());
}

void GlobalResourcesPane::filterList(const QString& /*filter*/)
{
    // TODO
}

void GlobalResourcesPane::drawBranches(QPainter* painter, const QRect& rect,
                                       const QModelIndex& index) const
{
    Q_D(const GlobalResourcesPane);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const qreal width = 10;
    const QTreeViewItem &viewItem = d->viewItems.at(d->current);
    const bool hasChild = viewItem.hasChildren;
    const bool isSelected = d->selectionModel->isSelected(index);

    QRectF handleRect(0, 0, width, width);
    handleRect.moveCenter(rect.center());
    handleRect.moveRight(rect.right() - 0.5);

    QStyleOptionViewItem option;
    option.initFrom(this);
    option.rect = rect;
    if (isSelected)
        option.state |= QStyle::State_Selected;
    else if (option.state & QStyle::State_Selected)
        option.state ^= QStyle::State_Selected;

    fillBackground(painter, option, d->viewIndex(index), false);

    // Draw handle
    if (hasChild) {
        QPen pen;
        pen.setWidthF(1.2);
        pen.setColor(isSelected ? palette().highlightedText().color() : palette().text().color());
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(handleRect, 0, 0);

        painter->drawLine(QPointF(handleRect.left() + 2.5, handleRect.center().y()),
                          QPointF(handleRect.right() - 2.5, handleRect.center().y()));

        if (!isExpanded(index)) {
            painter->drawLine(QPointF(handleRect.center().x(), handleRect.top() + 2.5),
                              QPointF(handleRect.center().x(), handleRect.bottom() - 2.5));
        }
    }

    painter->restore();
}

void GlobalResourcesPane::paintEvent(QPaintEvent* e)
{
    QPainter painter(viewport());
    painter.fillRect(rect(), palette().base());
    painter.setClipping(true);

    QColor lineColor(palette().text().color().lighter(270));
    lineColor.setAlpha(50);
    painter.setPen(lineColor);

    for (int i = 0; i < viewport()->height() / qreal(ROW_HEIGHT); ++i) {
        QRectF rect(0, i * ROW_HEIGHT, viewport()->width(), ROW_HEIGHT);
        QPainterPath path;
        path.addRect(rect);
        painter.setClipPath(path);

        // Fill background
        if (i % 2)
            painter.fillRect(rect, palette().alternateBase());

        // Draw top and bottom lines
        painter.drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
        painter.drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});
    }

    QTreeView::paintEvent(e);
}

void GlobalResourcesPane::updateGeometries()
{
    QTreeView::updateGeometries();
    QMargins vm = viewportMargins();
    vm.setBottom(m_searchEdit->height());
    vm.setTop(header()->height() + m_toolbar->height());
    QRect vg = viewport()->geometry();
    QRect sg(vg.left(), vg.bottom(), vg.width(), m_searchEdit->height());
    QRect tg(vg.left(), header()->height(), vg.width(), m_toolbar->height());
    setViewportMargins(vm);
    m_searchEdit->setGeometry(sg);
    m_toolbar->setGeometry(tg);
    header()->setGeometry(vg.left(), 0, vg.width(), header()->height());
    m_modeComboBox->move(header()->width() - m_modeComboBox->width(), header()->height() / 2.0 - m_modeComboBox->height() / 2.0);
}

QSize GlobalResourcesPane::sizeHint() const
{
    return QSize{310, 280};
}

#include "globalresourcespane.moc"
