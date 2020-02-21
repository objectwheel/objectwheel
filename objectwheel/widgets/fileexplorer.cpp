/*
    TODO List
    Drag drop from the file explorer to desktop
    Show a right click menu on selected entries when user right clicks on them to show available
    - file operation options like copy, paste, delete etc.)
    Navigating on search results of the search auto completer popup should be possible via
    - using Tab key
    Add a file name auto completion when users press to Tab over PathIndicator
    Add a combobox to make it possible to short files and dirs on the tree (like QDir::SortFlags)
    Convert "Name" title of the first header to "" (empty) and put all sort of controls (like
    - sorting combobox) on it. --alight those control left to right--
    Add a rubber band for file selection
    Block deletion of qmldir within FileExplorer dir
    Improve the performance of FileSearchModel::updateModel, don't iterate over all the files
    - on the file system for each QFileSystemWatcher's "changed" signals
*/

#include <fileexplorer.h>
#include <fileexplorer_p.h>
#include <lineedit.h>
#include <utilityfunctions.h>
#include <utilsicons.h>
#include <paintutils.h>
#include <filesystemutils.h>
#include <private/qtreeview_p.h>

#include <QtWidgets>

#define mt(index) m_fileSystemProxyModel->mapToSource(index)
#define mf(index) m_fileSystemProxyModel->mapFromSource(index)

FileExplorer::FileExplorer(QWidget* parent) : QTreeView(parent)
  , m_mode(Invalid)
  , m_dropHereLabel(new QLabel(this))
  , m_droppingBlurEffect(new QGraphicsBlurEffect(this))
  , m_fileSystemModel(new QFileSystemModel(this))
  , m_fileSystemProxyModel(new FileSystemProxyModel(this))
  , m_toolBar(new QToolBar(this))
  , m_pathIndicator(new PathIndicator(this))
  , m_upButton(new QToolButton)
  , m_backButton(new QToolButton)
  , m_forthButton(new QToolButton)
  , m_homeButton(new QToolButton)
  , m_copyButton(new QToolButton)
  , m_pasteButton(new QToolButton)
  , m_deleteButton(new QToolButton)
  , m_renameButton(new QToolButton)
  , m_newFileButton(new QToolButton)
  , m_newFolderButton(new QToolButton)
  , m_lastVScrollerPosOfViewer(0)
  , m_lastHScrollerPosOfViewer(0)
  , m_lastVScrollerPosOfExplorer(0)
  , m_lastHScrollerPosOfExplorer(0)
{
    header()->setFixedHeight(20);
    header()->setDefaultSectionSize(1);
    header()->setMinimumSectionSize(1);
    header()->setSectionsMovable(false);

    setDragEnabled(false);
    setSortingEnabled(true);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setItemDelegate(new FileExplorerListDelegate(this));
    setFocusPolicy(Qt::NoFocus);
    setSelectionBehavior(SelectRows);
    setDragDropMode(NoDragDrop);
    setEditTriggers(NoEditTriggers);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto updatePalette = [=] {
        QPalette p(palette());
        p.setColor(QPalette::Light, "#ffffff");
        p.setColor(QPalette::Dark, "#f0f0f0");
        p.setColor(QPalette::AlternateBase, "#f7f7f7");
        p.setColor(QPalette::Mid, p.text().color().lighter()); // For line and "empty folder" color
        p.setColor(QPalette::Midlight, "#f6f6f6"); // For PathIndicator's background
        p.setColor(QPalette::Shadow, "#c4c4c4"); // For PathIndicator's border
        setPalette(p);
    };
    connect(qApp, &QApplication::paletteChanged, this, updatePalette);
    updatePalette();

    QPixmap p(":/images/drop.png");
    p.setDevicePixelRatio(devicePixelRatioF());
    m_dropHereLabel->setHidden(true);
    m_dropHereLabel->setAlignment(Qt::AlignCenter);
    m_dropHereLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_dropHereLabel->setScaledContents(true);
    m_dropHereLabel->setPixmap(p);
    m_dropHereLabel->raise();

    m_droppingBlurEffect->setBlurHints(QGraphicsBlurEffect::QualityHint);
    m_droppingBlurEffect->setEnabled(false);
    m_droppingBlurEffect->setBlurRadius(40);
    viewport()->setGraphicsEffect(m_droppingBlurEffect);

    m_upButton->setCursor(Qt::PointingHandCursor);
    m_backButton->setCursor(Qt::PointingHandCursor);
    m_forthButton->setCursor(Qt::PointingHandCursor);
    m_homeButton->setCursor(Qt::PointingHandCursor);
    m_copyButton->setCursor(Qt::PointingHandCursor);
    m_pasteButton->setCursor(Qt::PointingHandCursor);
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    m_renameButton->setCursor(Qt::PointingHandCursor);
    m_newFileButton->setCursor(Qt::PointingHandCursor);
    m_newFolderButton->setCursor(Qt::PointingHandCursor);

    m_upButton->setToolTip(tr("Go to the upper directory"));
    m_backButton->setToolTip(tr("Go to the previous directory"));
    m_forthButton->setToolTip(tr("Go to the next directory"));
    m_homeButton->setToolTip(tr("Go to the home directory"));
    m_copyButton->setToolTip(tr("Copy selected files/folders into the system's clipboard"));
    m_pasteButton->setToolTip(tr("Paste files/folders from system's clipboard into the current directory"));
    m_deleteButton->setToolTip(tr("Delete selected files/folders"));
    m_renameButton->setToolTip(tr("Rename selected file/folder"));
    m_newFileButton->setToolTip(tr("Create an empty new file within the current directory"));
    m_newFolderButton->setToolTip(tr("Create an empty new folder within the current directory"));
    m_pathIndicator->setToolTip(tr("Double click on this in order to edit the path"));

    m_upButton->setIcon(Utils::Icons::ARROW_UP.icon());
    m_backButton->setIcon(Utils::Icons::ARROW_BACK.icon());
    m_forthButton->setIcon(Utils::Icons::ARROW_FORTH.icon());
    m_homeButton->setIcon(Utils::Icons::HOME_TOOLBAR.icon());
    m_copyButton->setIcon(Utils::Icons::COPY_TOOLBAR.icon());
    m_pasteButton->setIcon(Utils::Icons::PASTE_TOOLBAR.icon());
    m_deleteButton->setIcon(Utils::Icons::DELETE_TOOLBAR.icon());
    m_renameButton->setIcon(Utils::Icons::RENAME.icon());
    m_newFileButton->setIcon(Utils::Icons::FILENEW.icon());
    m_newFolderButton->setIcon(Utils::Icons::FOLDERNEW.icon());

    m_upButton->setFixedSize(18, 18);
    m_backButton->setFixedSize(18, 18);
    m_forthButton->setFixedSize(18, 18);
    m_homeButton->setFixedSize(18, 18);
    m_copyButton->setFixedSize(18, 18);
    m_pasteButton->setFixedSize(18, 18);
    m_deleteButton->setFixedSize(18, 18);
    m_renameButton->setFixedSize(18, 18);
    m_newFileButton->setFixedSize(18, 18);
    m_newFolderButton->setFixedSize(18, 18);

    m_pathIndicator->setFixedHeight(16);
    m_toolBar->layout()->setSpacing(2);
    m_toolBar->setFixedHeight(20);
    m_toolBar->addWidget(m_homeButton);
    m_toolBar->addWidget(m_backButton);
    m_toolBar->addWidget(m_forthButton);
    m_toolBar->addWidget(m_upButton);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(m_renameButton);
    m_toolBar->addWidget(m_deleteButton);
    m_toolBar->addWidget(m_copyButton);
    m_toolBar->addWidget(m_pasteButton);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(m_newFileButton);
    m_toolBar->addWidget(m_newFolderButton);

    m_copyButton->setDisabled(true);
    m_pasteButton->setEnabled(QApplication::clipboard()->mimeData()->hasUrls());
    m_deleteButton->setDisabled(true);
    m_renameButton->setDisabled(true);
    m_backButton->setDisabled(true);
    m_forthButton->setDisabled(true);

    m_fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    m_fileSystemProxyModel->setDynamicSortFilter(true);
    m_fileSystemProxyModel->setFilterKeyColumn(0);
    m_fileSystemProxyModel->setSourceModel(m_fileSystemModel);

    setModel(m_fileSystemProxyModel);
    setMode(Viewer);
    setRootPath(QApplication::applicationDirPath());

    connect(m_upButton, &QToolButton::clicked,
            this, &FileExplorer::onUpButtonClick);
    connect(m_backButton, &QToolButton::clicked,
            this, &FileExplorer::onBackButtonClick);
    connect(m_forthButton, &QToolButton::clicked,
            this, &FileExplorer::onForthButtonClick);
    connect(m_homeButton, &QToolButton::clicked,
            this, &FileExplorer::onHomeButtonClick);
    connect(m_copyButton, &QToolButton::clicked,
            this, &FileExplorer::onCopyButtonClick);
    connect(m_pasteButton, &QToolButton::clicked,
            this, &FileExplorer::onPasteButtonClick);
    connect(m_deleteButton, &QToolButton::clicked,
            this, &FileExplorer::onDeleteButtonClick);
    connect(m_renameButton, &QToolButton::clicked,
            this, &FileExplorer::onRenameButtonClick);
    connect(m_newFileButton, &QToolButton::clicked,
            this, &FileExplorer::onNewFileButtonClick);
    connect(m_newFolderButton, &QToolButton::clicked,
            this, &FileExplorer::onNewFolderButtonClick);
    connect(m_fileSystemModel, &QFileSystemModel::fileRenamed,
            this, [=] {
        m_fileSystemProxyModel->setDynamicSortFilter(false);
        m_fileSystemProxyModel->setDynamicSortFilter(true);
    });
    connect(m_fileSystemModel, &QFileSystemModel::fileRenamed,
            this, &FileExplorer::fileRenamed);
    connect(m_pathIndicator, &PathIndicator::pathUpdated,
            this, &FileExplorer::goToRelativeDir);
    connect(this, &FileExplorer::doubleClicked,
            this, &FileExplorer::onItemDoubleClick);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [=]
    { m_pasteButton->setEnabled(QApplication::clipboard()->mimeData()->hasUrls()); });
}

void FileExplorer::clean()
{
    m_lastVScrollerPosOfViewer = 0;
    m_lastHScrollerPosOfViewer = 0;
    m_lastVScrollerPosOfExplorer = 0;
    m_lastHScrollerPosOfExplorer = 0;

    m_lastSelectedIndexesOfExplorer.clear();
    m_lastSelectedIndexesOfViewer.clear();
    m_lastExpandedIndexesOfViewer.clear();
    m_lastPathofExplorer.clear();
    m_backPathStack.clear();
    m_forthPathStack.clear();

    m_copyButton->setDisabled(true);
    m_pasteButton->setEnabled(QApplication::clipboard()->mimeData()->hasUrls());
    m_deleteButton->setDisabled(true);
    m_renameButton->setDisabled(true);
    m_backButton->setDisabled(true);
    m_forthButton->setDisabled(true);

    setRootPath(QApplication::applicationDirPath());
}

QString FileExplorer::rootPath() const
{
    return m_fileSystemModel->rootPath();
}

void FileExplorer::setRootPath(const QString& rootPath)
{
    if (m_fileSystemModel->rootPath() == rootPath)
        return;

    Q_ASSERT(QFileInfo::exists(rootPath));

    if (selectionModel()) {
        selectionModel()->clear();
        selectionModel()->disconnect(this, SLOT(onFileSelectionChange()));
    }

    setRootIndex(QModelIndex());
    m_lastVScrollerPosOfViewer = 0;
    m_lastHScrollerPosOfViewer = 0;
    m_lastVScrollerPosOfExplorer = 0;
    m_lastHScrollerPosOfExplorer = 0;
    m_lastSelectedIndexesOfExplorer.clear();
    m_lastSelectedIndexesOfViewer.clear();
    m_lastExpandedIndexesOfViewer.clear();
    m_lastPathofExplorer.clear();
    m_backPathStack.clear();
    m_forthPathStack.clear();

    m_copyButton->setDisabled(true);
    m_pasteButton->setEnabled(QApplication::clipboard()->mimeData()->hasUrls());
    m_deleteButton->setDisabled(true);
    m_renameButton->setDisabled(true);
    m_backButton->setDisabled(true);
    m_forthButton->setDisabled(true);

    m_fileSystemModel->setRootPath(rootPath);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &FileExplorer::onFileSelectionChange);
    onHomeButtonClick();
    hideColumn(1);
    hideColumn(2);
    hideColumn(3);
}

void FileExplorer::onUpButtonClick()
{
    const QString& upperDir = QFileInfo(m_fileSystemModel->filePath(mt(rootIndex()))).path();
    const QString& rootPath = m_fileSystemModel->rootPath();

    if (upperDir.size() > rootPath.size() || upperDir == rootPath)
        goToDir(upperDir);
}

void FileExplorer::onBackButtonClick()
{
    if (m_backPathStack.isEmpty())
        return;

    const QStack<QString> cloneOfForth = m_forthPathStack;
    const QString& rootPath = m_fileSystemModel->filePath(mt(rootIndex()));

    goToDir(m_backPathStack.pop());
    m_backPathStack.pop();

    m_forthPathStack = cloneOfForth;
    if (!rootPath.isEmpty())
        m_forthPathStack.push(rootPath);

    m_backButton->setDisabled(m_backPathStack.isEmpty());
    m_forthButton->setDisabled(m_forthPathStack.isEmpty());
}

void FileExplorer::onForthButtonClick()
{
    if (m_forthPathStack.isEmpty())
        return;

    const QStack<QString> cloneOfForth = m_forthPathStack;
    goToDir(m_forthPathStack.pop());
    m_forthPathStack = cloneOfForth;
    m_forthPathStack.pop();
    m_forthButton->setDisabled(m_forthPathStack.isEmpty());
}

void FileExplorer::onHomeButtonClick()
{
    goToDir(m_fileSystemModel->rootPath());
}

void FileExplorer::onCopyButtonClick()
{
    QSet<QUrl> urls;
    for (const QModelIndex& index : selectedIndexes())
        urls.insert(QUrl::fromLocalFile(m_fileSystemModel->filePath(mt(index))));

    if (!urls.isEmpty()) {
        auto mimeData = new QMimeData;
        mimeData->setUrls(urls.values());
        QApplication::clipboard()->setMimeData(mimeData);
    }
}

void FileExplorer::onPasteButtonClick()
{
    const QString& rootPath = m_fileSystemModel->filePath(mt(rootIndex()));
    const QMimeData* clipboard = QApplication::clipboard()->mimeData();
    if (clipboard && clipboard->hasUrls())
        UtilityFunctions::copyFiles(rootPath, clipboard->urls(), this);
}

void FileExplorer::onDeleteButtonClick()
{
    int result = UtilityFunctions::showMessage(this, tr("Confirm file deletion"),
                                               tr("Do you want to delete the selected files? "
                                                  "The changes made in the Code Editor will also be discarded."),
                                               QMessageBox::Warning,
                                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result == QMessageBox::Yes) {
        QSet<QString> deletedFiles;
        for (const QModelIndex& index : selectedIndexes()) {
            if (index.isValid())
                deletedFiles.insert(m_fileSystemModel->filePath(mt(index)));
        }

        emit filesAboutToBeDeleted(deletedFiles);

        for (const QModelIndex& index : selectedIndexes()) {
            if (index.isValid())
                m_fileSystemModel->remove(mt(index));
        }
    }
}

void FileExplorer::onRenameButtonClick()
{
    if (selectedIndexes().size() == 1) {
        setCurrentIndex(selectedIndexes().first());
        scrollTo(currentIndex());
        edit(currentIndex());
    }
}

void FileExplorer::onNewFileButtonClick()
{
    QString baseFileName = tr("Empty File");
    const QString& rootPath = m_fileSystemModel->filePath(mt(rootIndex()));

    while (QFileInfo::exists(rootPath + '/' + baseFileName + ".txt"))
        baseFileName = UtilityFunctions::increasedNumberedText(baseFileName, true, true);

    QModelIndex index;
    const bool suceed = FileSystemUtils::makeFile(rootPath + '/' + baseFileName + ".txt");

    if (suceed)
        index = mf(m_fileSystemModel->index(rootPath + '/' + baseFileName + ".txt"));

    if (index.isValid()) {
        selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        setCurrentIndex(index);
        scrollTo(index, PositionAtCenter);
        edit(index);
    } else {
        qWarning() << "FileExplorer:" << tr("File creation failed");
    }
}

void FileExplorer::onNewFolderButtonClick()
{
    QString baseFolderName = tr("New Folder");
    const QString& rootPath = m_fileSystemModel->filePath(mt(rootIndex()));

    while (QFileInfo::exists(rootPath + '/' + baseFolderName))
        baseFolderName = UtilityFunctions::increasedNumberedText(baseFolderName, true, true);

    const QModelIndex& index = mf(m_fileSystemModel->mkdir(mt(rootIndex()), baseFolderName));
    if (index.isValid()) {
        selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        setCurrentIndex(index);
        scrollTo(index, PositionAtCenter);
        edit(index);
    } else {
        qWarning() << "FileExplorer:" << tr("Folder creation failed");
    }
}

void FileExplorer::onFileSelectionChange()
{
    const int selectedItemSize = selectedIndexes().size();
    m_deleteButton->setEnabled(selectedItemSize > 0);
    m_copyButton->setEnabled(selectedItemSize > 0);
    m_renameButton->setEnabled(selectedItemSize == 1);
}

void FileExplorer::onItemDoubleClick(const QModelIndex& index)
{
    if (m_fileSystemModel->isDir(mt(index)) && m_mode == Explorer)
        return goToDir(m_fileSystemModel->filePath(mt(index)));
    if (!m_fileSystemModel->isDir(mt(index)))
        emit fileOpened(QDir(rootPath()).relativeFilePath(m_fileSystemModel->filePath(mt(index))));
}

void FileExplorer::setPalette(const QPalette& pal)
{    
    QWidget::setPalette(pal);
    m_pathIndicator->setPalette(pal);

    // Workaround for an issue caused by setting the style
    // sheet more than once, this ensures the change happens
    ensurePolished();

    setStyleSheet(QString {
                      "QTreeView {"
                      "    border: 1px solid %1;"
                      "} QHeaderView::down-arrow {"
                      "    image: none;"
                      "} QHeaderView::up-arrow {"
                      "    image: none;"
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
                  .arg(pal.dark().color().darker(120).name())
                  .arg(pal.light().color().name())
                  .arg(pal.dark().color().name())
                  .arg(pal.buttonText().color().name()));

    update();
}

void FileExplorer::paintBackground(QPainter* painter, const QStyleOptionViewItem& option, int rowNumber,
                                   bool hasVerticalLine) const
{
    painter->save();

    bool isSelected = option.state & QStyle::State_Selected;
    const QPalette& pal = option.palette;
    const QRectF& rect = option.rect;

    painter->setClipRect(rect);

    // Fill background
    if (isSelected) {
        painter->fillRect(rect, pal.highlight());
    } else {
        if (rowNumber % 2)
            painter->fillRect(rect, pal.alternateBase());
        else
            painter->fillRect(rect, pal.base());
    }

    // Draw top and bottom lines
    QColor lineColor(pal.mid().color());
    lineColor.setAlpha(50);
    painter->setPen(lineColor);
    if (rowNumber != 0) {
        painter->drawLine(rect.topLeft() + QPointF(0.5, 0.0),
                          rect.topRight() - QPointF(0.5, 0.0));
    }
    painter->drawLine(rect.bottomLeft() + QPointF(0.5, 0.0),
                      rect.bottomRight() - QPointF(0.5, 0.0));

    // Draw vertical line
    if (hasVerticalLine) {
        painter->drawLine(rect.topRight() + QPointF(-0.5, 0.5),
                          rect.bottomRight() + QPointF(-0.5, -0.5));
    }

    painter->restore();
}

void FileExplorer::goToDir(const QString& dir)
{
    if (!model())
        return;

    if (dir.isEmpty())
        return;

    if (!QFileInfo::exists(dir) || !QFileInfo(dir).isDir())
        return;

    const QString& previousDir = m_fileSystemModel->filePath(mt(rootIndex()));
    const QModelIndex& index = m_fileSystemModel->index(dir);

    if (previousDir == QFileInfo(dir).canonicalFilePath())
        return;

    if (!index.isValid())
        return;

    setRootIndex(mf(index));
    selectionModel()->clear();
    m_upButton->setDisabled(m_fileSystemModel->index(m_fileSystemModel->rootPath()) == index);
    m_homeButton->setDisabled(m_fileSystemModel->index(m_fileSystemModel->rootPath()) == index);
    m_pathIndicator->setPath(QDir(m_fileSystemModel->rootPath()).relativeFilePath(dir));

    if (!previousDir.isEmpty()) {
        if (m_backPathStack.isEmpty() || m_backPathStack.top() != previousDir)
            m_backPathStack.push(previousDir);
    }

    m_forthPathStack.clear();

    m_backButton->setDisabled(m_backPathStack.isEmpty());
    m_forthButton->setDisabled(m_forthPathStack.isEmpty());

    emit currentDirChanged(dir);
}

void FileExplorer::goToRelativeDir(const QString& relativeDir)
{
    const QString& dir = m_fileSystemModel->rootPath() + '/' + relativeDir;

    if (!QFileInfo::exists(dir) || !QFileInfo(dir).isDir())
        return;

    if (!QFileInfo(dir).canonicalFilePath().contains(m_fileSystemModel->rootPath(), Qt::CaseInsensitive))
        return; // Protection against "rootPath/../../.." etc

    goToDir(dir);
}

void FileExplorer::goToEntry(const QString& entry)
{
    const QFileInfo info(entry);
    const bool isDir = info.isDir();
    const QModelIndex& index = mf(m_fileSystemModel->index(entry));

    if (index.isValid() && (!isDir || index.parent().isValid())) {
        if (m_mode == Viewer)
            UtilityFunctions::expandUpToRoot(this, isDir ? index.parent() : index, rootIndex());
        else
            goToDir(isDir ? info.dir().path() : info.path());

        selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        scrollTo(index, PositionAtCenter);
    }
}

void FileExplorer::dropEvent(QDropEvent* event)
{
    const QString& rootPath = m_fileSystemModel->filePath(mt(rootIndex()));
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        UtilityFunctions::copyFiles(rootPath, event->mimeData()->urls(), this);
    }
    m_dropHereLabel->setHidden(true);
    m_droppingBlurEffect->setEnabled(false);
}

void FileExplorer::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->accept();
        m_dropHereLabel->setVisible(true);
        m_droppingBlurEffect->setEnabled(true);
    }
}

void FileExplorer::dragMoveEvent(QDragMoveEvent* event)
{
    event->accept();
}

void FileExplorer::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
    m_dropHereLabel->setHidden(true);
    m_droppingBlurEffect->setEnabled(false);
}

void FileExplorer::drawBranches(QPainter* painter, const QRect& rect,
                                const QModelIndex& index) const
{
    Q_D(const QTreeView);
    painter->save();

    const qreal width = 10;
    const bool hasChild = m_fileSystemModel->hasChildren(mt(index));
    const bool isSelected = selectionModel()->isSelected(index);

    const QRectF r(rect);
    QRectF handleRect(0, 0, width, width);
    handleRect.moveCenter(r.center());
    handleRect.moveRight(r.right() - 0.5);

    QStyleOptionViewItem option;
    option.initFrom(this);
    option.rect = rect;
    option.state &= ~QStyle::State_Selected;
    if (isSelected)
        option.state |= QStyle::State_Selected;

    paintBackground(painter, option, d->viewIndex(index), false);

    // Draw handle
    if (hasChild) {
        const QColor c = isSelected ? palette().highlightedText().color() : palette().text().color().lighter(220);
        painter->setPen(QPen(c, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(handleRect, 0, 0);
        painter->drawLine(QPointF(handleRect.left() + 2, handleRect.center().y()),
                          QPointF(handleRect.right() - 2, handleRect.center().y()));
        if (!isExpanded(index)) {
            painter->drawLine(QPointF(handleRect.center().x(), handleRect.top() + 2),
                              QPointF(handleRect.center().x(), handleRect.bottom() - 2));
        }
    }

    painter->restore();
}

void FileExplorer::paintEvent(QPaintEvent* event)
{
    Q_D(const QTreeView);

    const bool folderHasChildren = m_fileSystemModel->QAbstractItemModel::hasChildren(mt(rootIndex()));

    QPainter painter(viewport());
    painter.fillRect(rect(), palette().base());

    QColor lineColor(palette().mid().color());
    lineColor.setAlpha(50);
    painter.setPen(lineColor);

    qreal height = d->defaultItemHeight > 0 ? d->defaultItemHeight : 20;
    qreal rowCount = viewport()->height() / height;
    for (int i = 0; i < rowCount; ++i) {
        QRectF rect(0, i * height, viewport()->width(), height);
        painter.setClipRect(rect);

        // Fill background
        if (i % 2) {
            painter.fillRect(rect, palette().alternateBase());
        } else if (!folderHasChildren) {
            if (i == int((rowCount - 1) / 2.0) || i == int((rowCount - 1) / 2.0) + 1) {
                painter.setPen(palette().mid().color().lighter(220));
                painter.drawText(rect, Qt::AlignCenter, tr("Empty folder"));
                painter.setPen(lineColor);
            }
        }

        // Draw top and bottom lines
        painter.drawLine(rect.topLeft() + QPointF(0.5, 0.0),
                         rect.topRight() - QPointF(0.5, 0.0));
        painter.drawLine(rect.bottomLeft() + QPointF(0.5, 0.0),
                         rect.bottomRight() - QPointF(0.5, 0.0));
    }
    painter.end();

    QTreeView::paintEvent(event);
}

void FileExplorer::updateGeometries()
{
    QTreeView::updateGeometries();
    QMargins vm = viewportMargins();
    vm.setTop(header()->height() + (m_mode == Explorer ? m_pathIndicator->height() + m_toolBar->height() - 2 : 0));
    setViewportMargins(vm);

    QRect vg = viewport()->geometry();
    header()->setGeometry(vg.left(), 1, vg.width(), header()->height());

    int ds = qMin(qMin(vg.width() - 5, vg.height() - 5), 100);
    QRect dg;
    dg.setSize({ds, ds});
    dg.moveCenter(vg.center());
    m_dropHereLabel->setGeometry(dg);

    if (m_mode == Explorer) {
        QRect tg(vg.left(), header()->height() + 1, vg.width(), m_toolBar->height());
        QRect pg(vg.left(), header()->height() + 1 + m_toolBar->height(), vg.width(), m_pathIndicator->height());
        m_toolBar->setGeometry(tg);
        m_pathIndicator->setGeometry(pg);
    }
}

FileExplorer::Mode FileExplorer::mode() const
{
    return m_mode;
}

void FileExplorer::setMode(FileExplorer::Mode mode)
{
    Q_D(const QTreeView);

    if (m_mode != mode) {
        m_mode = mode;

        if (m_mode == Viewer) {
            m_lastPathofExplorer = m_fileSystemModel->filePath(mt(rootIndex()));
            m_lastSelectedIndexesOfExplorer = selectedIndexes();
            m_lastVScrollerPosOfExplorer = verticalScrollBar()->sliderPosition();
            m_lastHScrollerPosOfExplorer = horizontalScrollBar()->sliderPosition();;

            onHomeButtonClick();

            for (const QPersistentModelIndex& index : m_lastExpandedIndexesOfViewer) {
                if (index.isValid())
                    setExpanded(index, true);
            }

            selectionModel()->clear();
            for (const QModelIndex& index : m_lastSelectedIndexesOfViewer) {
                if (index.isValid())
                    selectionModel()->select(index, QItemSelectionModel::Select);
            }

            setIndentation(16);
            setAcceptDrops(false);
            setRootIsDecorated(true);
            setItemsExpandable(true);
            setExpandsOnDoubleClick(true);
            setSelectionMode(QTreeView::SingleSelection);

            m_fileSystemModel->setReadOnly(true);
            m_toolBar->hide();
            m_pathIndicator->hide();

            verticalScrollBar()->setSliderPosition(m_lastVScrollerPosOfViewer);
            horizontalScrollBar()->setSliderPosition(m_lastHScrollerPosOfViewer);
        } else {
            m_lastExpandedIndexesOfViewer = d->expandedIndexes;
            m_lastSelectedIndexesOfViewer = selectedIndexes();
            m_lastVScrollerPosOfViewer = verticalScrollBar()->sliderPosition();
            m_lastHScrollerPosOfViewer = horizontalScrollBar()->sliderPosition();;

            goToDir(m_lastPathofExplorer);

            selectionModel()->clear();
            for (const QModelIndex& index : m_lastSelectedIndexesOfExplorer) {
                if (index.isValid())
                    selectionModel()->select(index, QItemSelectionModel::Select);
            }

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

            verticalScrollBar()->setSliderPosition(m_lastVScrollerPosOfExplorer);
            horizontalScrollBar()->setSliderPosition(m_lastHScrollerPosOfExplorer);
        }

        updateGeometries();
    }
}

QSize FileExplorer::sizeHint() const
{
    return QSize(230, 640);
}
