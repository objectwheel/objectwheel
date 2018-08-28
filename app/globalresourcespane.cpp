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
#include <QMessageBox>

// TODO List
// Drag drop from file explorer to desktop
// Drag drop from desktop into file explorer

using namespace Utils;

namespace {
const int ROW_HEIGHT = 21;
QModelIndexList lastSelectedIndexesOfViewer;
QModelIndexList lastSelectedIndexesOfExplorer;
QSet<QPersistentModelIndex> lastExpandedIndexesOfViewer;
QString lastPathofExplorer;

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
    painter->setPen(lineColor);
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

class PathIndicator : public QWidget {
    Q_OBJECT

public:
    explicit PathIndicator(QWidget* parent = nullptr) : QWidget(parent)
      , m_pathEdit(new QLineEdit(this)) {
        m_pathEdit->hide();
        connect(m_pathEdit, &QLineEdit::editingFinished,
                m_pathEdit, &QLineEdit::hide);
        connect(m_pathEdit, &QLineEdit::editingFinished,
                this, [=] { emit pathUpdated(m_pathEdit->text()); });
    }
    void setPath(const QString& path) {
        m_path = path;
        if (m_path == ".")
            m_path = "";
        m_path.prepend("/");
        m_pathEdit->setText(m_path);
        update();
    }

signals:
    void pathUpdated(const QString& newPath);

private:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), "#f6f6f6");
        painter.setPen("#c4c4c4");
        painter.drawLine(QRectF(rect()).bottomLeft(), QRectF(rect()).bottomRight());
        painter.setPen(Qt::black);

        if (m_pathEdit->isHidden()) {
            QTextOption textOption(Qt::AlignLeft | Qt::AlignVCenter);
            textOption.setWrapMode(QTextOption::NoWrap);
            painter.drawText(rect().adjusted(3, 0, -3, 0),
                             fontMetrics().elidedText(m_path, Qt::ElideLeft, width() - 6),
                             textOption);
        }
    }
    void resizeEvent(QResizeEvent* e) override {
        QWidget::resizeEvent(e);
        m_pathEdit->setGeometry(rect());
    }
    void mouseDoubleClickEvent(QMouseEvent* e) override {
        QWidget::mouseDoubleClickEvent(e);
        m_pathEdit->setText(m_path);
        m_pathEdit->show();
        m_pathEdit->setFocus();
    }

private:
    QString m_path;
    QLineEdit* m_pathEdit;
};

GlobalResourcesPane::GlobalResourcesPane(QWidget* parent) : QTreeView(parent)
  , m_searchEdit(new FocuslessLineEdit(this))
  , m_fileSystemModel(new QFileSystemModel(this))
  , m_toolBar(new QToolBar(this))
  , m_pathIndicator(new PathIndicator(this))
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

    setIconSize({15, 15});
    setDragEnabled(false);
    setSortingEnabled(false);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setItemDelegate(new GlobalListDelegate(this));
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeView::SelectRows);
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
    m_modeComboBox->addItem(tr("Viewer")); // First must be the Viewer, the index is important
    m_modeComboBox->addItem(tr("Explorer"));

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

    m_upButton->setToolTip(tr("Go to upper directory"));
    m_homeButton->setToolTip(tr("Go to the home directory"));
    m_cutButton->setToolTip(tr("Cut selected files/folders"));
    m_copyButton->setToolTip(tr("Copy selected files/folders"));
    m_pasteButton->setToolTip(tr("Paste files/folders into the current directory"));
    m_deleteButton->setToolTip(tr("Delete selected files/folders"));
    m_renameButton->setToolTip(tr("Rename selected file/folder"));
    m_newFileButton->setToolTip(tr("Create an empty new file within the current directory"));
    m_newFolderButton->setToolTip(tr("Create an empty new folder within the current directory"));
    m_downloadFileButton->setToolTip(tr("Download a file from an url into the current directory"));
    m_modeComboBox->setToolTip(tr("Change the File Explorer Mode."));

    const Icon FILTER_ICON({{":/utils/images/filtericon.png", Theme::BackgroundColorNormal}});
    auto modeIFilterIconLabel = new QLabel(m_modeComboBox);
    modeIFilterIconLabel->setPixmap(FILTER_ICON.pixmap());
    modeIFilterIconLabel->setFixedSize(16, 16);
    modeIFilterIconLabel->move(0, 3);
    m_modeComboBox->setContentsMargins(16, 0, 0, 0);

    m_upButton->setIcon(Icons::ARROW_UP.icon());
    m_homeButton->setIcon(Icons::HOME_TOOLBAR.icon());
    m_cutButton->setIcon(Icons::CUT_TOOLBAR.icon());
    m_copyButton->setIcon(Icons::COPY_TOOLBAR.icon());
    m_pasteButton->setIcon(Icons::PASTE_TOOLBAR.icon());
    m_deleteButton->setIcon(Icons::DELETE_TOOLBAR.icon());
    m_renameButton->setIcon(Icons::RENAME.icon());
    m_newFileButton->setIcon(Icons::FILENEW.icon());
    m_newFolderButton->setIcon(Icons::FOLDERNEW.icon());
    m_downloadFileButton->setIcon(Icons::DOWNLOAD.icon());

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

    TransparentStyle::attach(m_toolBar);
    TransparentStyle::attach(m_pathIndicator);
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

    m_pathIndicator->setFixedHeight(17);
    m_toolBar->setFixedHeight(24);
    m_toolBar->addWidget(m_homeButton);
    m_toolBar->addWidget(m_upButton);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(m_renameButton);
    m_toolBar->addWidget(m_deleteButton);
    m_toolBar->addWidget(m_cutButton);
    m_toolBar->addWidget(m_copyButton);
    m_toolBar->addWidget(m_pasteButton);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(m_newFileButton);
    m_toolBar->addWidget(m_newFolderButton);
    m_toolBar->addWidget(m_downloadFileButton);

    m_fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

    m_searchEdit->setPlaceholderText("Filter");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(22);
    connect(m_searchEdit, &FocuslessLineEdit::textChanged, this, &GlobalResourcesPane::filterList);
    connect(m_pathIndicator, &PathIndicator::pathUpdated, this, &GlobalResourcesPane::goToRelativePath);

    connect(ProjectManager::instance(), &ProjectManager::started,
            this, &GlobalResourcesPane::onProjectStart);
    connect(this, &GlobalResourcesPane::doubleClicked,
            this, &GlobalResourcesPane::onItemDoubleClick);
}

void GlobalResourcesPane::sweep()
{
    // TODO
    m_searchEdit->clear();
    if (selectionModel())
        selectionModel()->disconnect(this);
    setModel(nullptr);
    m_modeComboBox->setCurrentIndex(0); // Viewer
    onModeChange();
    lastSelectedIndexesOfExplorer.clear();
    lastSelectedIndexesOfViewer.clear();
    lastExpandedIndexesOfViewer.clear();
    lastPathofExplorer.clear();
}

void GlobalResourcesPane::onProjectStart()
{
    m_fileSystemModel->setRootPath(SaveUtils::toGlobalDir(ProjectManager::dir()));
    setModel(m_fileSystemModel);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &GlobalResourcesPane::onFileSelectionChange);
    onHomeButtonClick();
    hideColumn(1);
    hideColumn(2);
    hideColumn(3);
}

void GlobalResourcesPane::onModeChange()
{
    Q_D(GlobalResourcesPane);

    if (m_modeComboBox->currentIndex() == 0)
        m_mode = Viewer;
    else
        m_mode = Explorer;

    if (m_mode == Viewer) {
        lastPathofExplorer = m_fileSystemModel->filePath(rootIndex());
        lastSelectedIndexesOfExplorer = selectionModel()->selectedIndexes();

        onHomeButtonClick();

        for (const QPersistentModelIndex& index : lastExpandedIndexesOfViewer)
            setExpanded(index, true);

        selectionModel()->clear();
        for (const QModelIndex& index : lastSelectedIndexesOfViewer)
            selectionModel()->select(index, QItemSelectionModel::Select);

        setIndentation(16);
        setAcceptDrops(false);
        setRootIsDecorated(true);
        setItemsExpandable(true);
        setExpandsOnDoubleClick(true);
        setSelectionMode(QTreeView::SingleSelection);

        m_fileSystemModel->setReadOnly(true);
        m_toolBar->hide();
        m_pathIndicator->hide();
    } else {
        lastExpandedIndexesOfViewer = d->expandedIndexes;
        lastSelectedIndexesOfViewer = selectionModel()->selectedIndexes();

        goToPath(lastPathofExplorer);

        selectionModel()->clear();
        for (const QModelIndex& index : lastSelectedIndexesOfExplorer)
            selectionModel()->select(index, QItemSelectionModel::Select);

        collapseAll();
        setIndentation(0);
        setAcceptDrops(true);
        setRootIsDecorated(false);
        setItemsExpandable(false);
        setExpandsOnDoubleClick(false);
        setSelectionMode(QTreeView::ExtendedSelection);

        m_fileSystemModel->setReadOnly(false);
        m_toolBar->show();
        m_pathIndicator->show();
    }

    updateGeometries();
}

void GlobalResourcesPane::onUpButtonClick()
{
    const QString& upperDir = dname(m_fileSystemModel->filePath(rootIndex()));
    const QString& rootPath = m_fileSystemModel->rootPath();

    if (upperDir.size() > rootPath.size() || upperDir == rootPath)
        goToPath(upperDir);
}

void GlobalResourcesPane::onHomeButtonClick()
{
    goToPath(m_fileSystemModel->rootPath());
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
    int result = QMessageBox::warning(this,
                                      tr("Files about to be deleted"),
                                      tr("Do you want to delete selected files? "
                                         "Changes made in Code Editor will also be discarded."),
                                      QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        QSet<QString> deletedFiles;
        for (const QModelIndex& index : selectedIndexes()) {
            if (index.isValid()) {
                deletedFiles.insert(m_fileSystemModel->filePath(index));
                m_fileSystemModel->remove(index);
            }
        }
        emit filesDeleted(deletedFiles);
    }
}

void GlobalResourcesPane::onRenameButtonClick()
{
    scrollTo(currentIndex());
    edit(currentIndex());
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
    QString folderName = tr("New Folder");
    const QString& rootPath = m_fileSystemModel->filePath(rootIndex());

    for (int i = 1; exists(rootPath + separator() + folderName); i++)
        folderName = tr("New Folder ") + QString::number(i);

    const QModelIndex& index = m_fileSystemModel->mkdir(rootIndex(), folderName);
    if (index.isValid()) {
        selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        scrollTo(index);
        edit(index);
    } else {
        qWarning() << "GlobalResourcesPane:" << tr("Folder creation failed");
    }
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
    const int selectedItemSize = selectionModel()->selectedIndexes().size() / m_fileSystemModel->columnCount(rootIndex());
    m_deleteButton->setEnabled(selectedItemSize > 0);
    m_cutButton->setEnabled(selectedItemSize > 0);
    m_copyButton->setEnabled(selectedItemSize > 0);
    m_renameButton->setEnabled(selectedItemSize == 1);
}

void GlobalResourcesPane::onItemDoubleClick(const QModelIndex& index)
{
    if (m_fileSystemModel->isDir(index) && m_mode == Explorer)
        goToPath(m_fileSystemModel->filePath(index));
    else
        emit fileOpened(m_fileSystemModel->filePath(index));
}

void GlobalResourcesPane::goToPath(const QString& path)
{
    if (path.isEmpty())
        return;

    if (!QFileInfo(path).exists() || !QFileInfo(path).isDir())
        return;

    const QModelIndex& index = m_fileSystemModel->index(path);
    if (!index.isValid())
        return;

    setRootIndex(index);
    selectionModel()->clear();
    m_upButton->setDisabled(m_fileSystemModel->index(m_fileSystemModel->rootPath()) == index);
    m_pathIndicator->setPath(QDir(m_fileSystemModel->rootPath()).relativeFilePath(path));
}

void GlobalResourcesPane::goToRelativePath(const QString& relativePath)
{
    const QString& path = m_fileSystemModel->rootPath() + '/' + relativePath;

    if (!QFileInfo(path).exists() || !QFileInfo(path).isDir())
        return;

    if (!QFileInfo(path).canonicalFilePath().contains(m_fileSystemModel->rootPath(), Qt::CaseInsensitive))
        return;

    goToPath(path);
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
    const bool hasChild = m_fileSystemModel->QAbstractItemModel::hasChildren(index);
    const bool isSelected = selectionModel()->isSelected(index);

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
    const bool rootHasChildren = m_fileSystemModel->QAbstractItemModel::hasChildren(rootIndex());

    QPainter painter(viewport());
    painter.fillRect(rect(), palette().base());
    painter.setClipping(true);

    QColor lineColor(palette().text().color().lighter(270));
    lineColor.setAlpha(50);
    painter.setPen(lineColor);

    qreal rowCount = viewport()->height() / qreal(ROW_HEIGHT);
    for (int i = 0; i < rowCount; ++i) {
        QRectF rect(0, i * ROW_HEIGHT, viewport()->width(), ROW_HEIGHT);
        QPainterPath path;
        path.addRect(rect);
        painter.setClipPath(path);

        // Fill background
        if (i % 2) {
            painter.fillRect(rect, palette().alternateBase());
        } else if (!rootHasChildren) {
            if (i == int(rowCount / 2.0) || i == int(rowCount / 2.0) + 1) {
                painter.setPen(palette().text().color().lighter(240));
                painter.drawText(rect, Qt::AlignCenter, tr("Empty folder"));
                painter.setPen(lineColor);
            }
        }

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
    vm.setTop(m_mode == Explorer ? header()->height() + m_pathIndicator->height() + m_toolBar->height() - 2 : 0);
    setViewportMargins(vm);
    QRect vg = viewport()->geometry();
    QRect sg(vg.left(), vg.bottom(), vg.width(), m_searchEdit->height());
    m_searchEdit->setGeometry(sg);
    header()->setGeometry(vg.left(), 1, vg.width(), header()->height());
    m_modeComboBox->move(header()->width() - m_modeComboBox->width(),
                         header()->height() / 2.0 - m_modeComboBox->height() / 2.0);
    if (m_mode == Explorer) {
        QRect tg(vg.left(), header()->height() + 1, vg.width(), m_toolBar->height());
        QRect pg(vg.left(), header()->height() + 1 + m_toolBar->height(), vg.width(), m_pathIndicator->height());
        m_toolBar->setGeometry(tg);
        m_pathIndicator->setGeometry(pg);
    }
}

QSize GlobalResourcesPane::sizeHint() const
{
    return QSize{310, 280};
}

#include "globalresourcespane.moc"
