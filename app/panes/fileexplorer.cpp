/*
    TODO List
    Drag drop from the file explorer to desktop
    Show real progress dialog when download action is in progress
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
#include <focuslesslineedit.h>
#include <utilityfunctions.h>
#include <filemanager.h>
#include <transparentstyle.h>
#include <utilsicons.h>
#include <paintutils.h>

#include <QComboBox>
#include <QHeaderView>
#include <QFileSystemModel>
#include <QToolBar>
#include <QToolButton>
#include <QLabel>
#include <QMessageBox>
#include <QScrollBar>
#include <QClipboard>
#include <QInputDialog>
#include <QGraphicsBlurEffect>
#include <QCompleter>
#include <QTemporaryDir>

#define mt(index) m_fileSystemProxyModel->mapToSource(index)
#define mf(index) m_fileSystemProxyModel->mapFromSource(index)

extern const char* TOOL_KEY;

using namespace Utils;

namespace {

QLabel* g_modeIFilterIconLabel;
const int ROW_HEIGHT = 21;
int lastVScrollerPosOfViewer = 0;
int lastHScrollerPosOfViewer = 0;
int lastVScrollerPosOfExplorer = 0;
int lastHScrollerPosOfExplorer = 0;
QModelIndexList lastSelectedIndexesOfViewer;
QModelIndexList lastSelectedIndexesOfExplorer;
QSet<QPersistentModelIndex> lastExpandedIndexesOfViewer;
QString lastPathofExplorer;
QStack<QString> backPathStack;
QStack<QString> forthPathStack;

QPalette initPalette(QWidget* widget)
{
    QPalette palette(widget->palette());
    palette.setColor(QPalette::Light, "#ffffff");
    palette.setColor(QPalette::Dark, "#f0f0f0");
    palette.setColor(QPalette::AlternateBase, "#f7f7f7");
    palette.setColor(QPalette::Mid, palette.text().color().lighter()); // For line and "empty folder" color
    palette.setColor(QPalette::Midlight, "#f6f6f6"); // For PathIndicator's background
    palette.setColor(QPalette::Shadow, "#c4c4c4"); // For PathIndicator's border
    return palette;
}
}

FileExplorer::FileExplorer(QWidget* parent) : QTreeView(parent)
  , m_dropHereLabel(new QLabel(this))
  , m_droppingBlurEffect(new QGraphicsBlurEffect(this))
  , m_searchEditCompleterModel(new FileSearchModel(this))
  , m_searchEditCompleter(new QCompleter(this))
  , m_searchEdit(new FocuslessLineEdit(this))
  , m_fileSystemModel(new QFileSystemModel(this))
  , m_fileSystemProxyModel(new FileSystemProxyModel(this))
  , m_toolBar(new QToolBar(this))
  , m_pathIndicator(new PathIndicator(this))
  , m_modeComboBox(new QComboBox(header()))
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
  , m_downloadFileButton(new QToolButton)
{
    g_modeIFilterIconLabel = new QLabel(m_modeComboBox);
    g_modeIFilterIconLabel->setFixedSize(16, 16);
    g_modeIFilterIconLabel->move(0, 3);
    m_modeComboBox->setContentsMargins(16, 0, 0, 0);

//    setPalette(initPalette(this));

    header()->setFixedHeight(23);
    header()->setDefaultSectionSize(1);
    header()->setMinimumSectionSize(1);
    header()->setSectionsMovable(false);
    header()->hide();

    setIconSize({15, 15});
    setDragEnabled(false);
    setSortingEnabled(true);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
//    setItemDelegate(new FileExplorerListDelegate(this));
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeView::SelectRows);
    setDragDropMode(QAbstractItemView::NoDragDrop);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QTreeView::ScrollPerPixel);
    setHorizontalScrollMode(QTreeView::ScrollPerPixel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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

    m_modeComboBox->addItem(tr("Viewer")); // First must be the Viewer, the index is important
    m_modeComboBox->addItem(tr("Explorer"));

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
    m_downloadFileButton->setCursor(Qt::PointingHandCursor);
    m_modeComboBox->setCursor(Qt::PointingHandCursor);

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
    m_downloadFileButton->setToolTip(tr("Download a file from an url into the current directory"));
    m_modeComboBox->setToolTip(tr("Change view mode"));
    m_pathIndicator->setToolTip(tr("Double click on this in order to edit the path"));

    m_upButton->setIcon(Icons::ARROW_UP.icon());
    m_backButton->setIcon(Icons::ARROW_BACK.icon());
    m_forthButton->setIcon(Icons::ARROW_FORTH.icon());
    m_homeButton->setIcon(Icons::HOME_TOOLBAR.icon());
    m_copyButton->setIcon(Icons::COPY_TOOLBAR.icon());
    m_pasteButton->setIcon(Icons::PASTE_TOOLBAR.icon());
    m_deleteButton->setIcon(Icons::DELETE_TOOLBAR.icon());
    m_renameButton->setIcon(Icons::RENAME.icon());
    m_newFileButton->setIcon(Icons::FILENEW.icon());
    m_newFolderButton->setIcon(Icons::FOLDERNEW.icon());
    m_downloadFileButton->setIcon(Icons::DOWNLOAD.icon());

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
    connect(m_downloadFileButton, &QToolButton::clicked,
            this, &FileExplorer::onDownloadButtonClick);
    connect(m_modeComboBox, qOverload<const QString&>(&QComboBox::activated),
            this, &FileExplorer::onModeChange);

    TransparentStyle::attach(m_toolBar);
    TransparentStyle::attach(m_pathIndicator);

    m_upButton->setFixedHeight(20);
    m_backButton->setFixedHeight(20);
    m_forthButton->setFixedHeight(20);
    m_homeButton->setFixedHeight(20);
    m_copyButton->setFixedHeight(20);
    m_pasteButton->setFixedHeight(20);
    m_deleteButton->setFixedHeight(20);
    m_renameButton->setFixedHeight(20);
    m_newFileButton->setFixedHeight(20);
    m_newFolderButton->setFixedHeight(20);
    m_downloadFileButton->setFixedHeight(20);
    m_modeComboBox->setFixedHeight(20);

    m_pathIndicator->setFixedHeight(17);
    m_toolBar->setFixedHeight(24);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({2, 2}));
    m_toolBar->addWidget(m_homeButton);
    m_toolBar->addWidget(m_backButton);
    m_toolBar->addWidget(m_forthButton);
    m_toolBar->addWidget(m_upButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({1, 1}));
    m_toolBar->addSeparator();
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({1, 1}));
    m_toolBar->addWidget(m_renameButton);
    m_toolBar->addWidget(m_deleteButton);
    m_toolBar->addWidget(m_copyButton);
    m_toolBar->addWidget(m_pasteButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({1, 1}));
    m_toolBar->addSeparator();
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({1, 1}));
    m_toolBar->addWidget(m_newFileButton);
    m_toolBar->addWidget(m_newFolderButton);
    m_toolBar->addWidget(m_downloadFileButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({2, 2}));

    m_fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    m_fileSystemProxyModel->setDynamicSortFilter(true);
    m_fileSystemProxyModel->setFilterKeyColumn(0);
    m_fileSystemProxyModel->setSourceModel(m_fileSystemModel);
    setModel(m_fileSystemProxyModel);

    m_searchEditCompleter->setFilterMode(Qt::MatchContains);
    m_searchEditCompleter->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    m_searchEditCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_searchEditCompleter->setModel(m_searchEditCompleterModel);
    m_searchEditCompleter->popup()->setIconSize({15, 15});
    m_searchEdit->setCompleter(m_searchEditCompleter);
    m_searchEdit->setPlaceholderText("Filter");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(22);
    connect(m_fileSystemModel, &QFileSystemModel::fileRenamed, this, [=]
    {
        m_fileSystemProxyModel->setDynamicSortFilter(false);
        m_fileSystemProxyModel->setDynamicSortFilter(true);
    });
    connect(m_searchEdit, qOverload<>(&FocuslessLineEdit::editingFinished), this, &FileExplorer::filterList);
    connect(m_pathIndicator, &PathIndicator::pathUpdated, this, &FileExplorer::goToRelativePath);

    connect(this, &FileExplorer::doubleClicked,
            this, &FileExplorer::onItemDoubleClick);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [=]
    { m_pasteButton->setEnabled(QApplication::clipboard()->mimeData()->hasUrls()); });
}

void FileExplorer::discharge()
{
    // TODO
    m_searchEdit->clear();

    m_mode = Viewer;
    m_modeComboBox->setCurrentIndex(0); // Viewer
    onModeChange();

    lastSelectedIndexesOfExplorer.clear();
    lastSelectedIndexesOfViewer.clear();
    lastExpandedIndexesOfViewer.clear();
    lastPathofExplorer.clear();
    lastVScrollerPosOfViewer = 0;
    lastHScrollerPosOfViewer = 0;
    lastVScrollerPosOfExplorer = 0;
    lastHScrollerPosOfExplorer = 0;
    backPathStack.clear();
    forthPathStack.clear();

    m_copyButton->setDisabled(true);
    m_pasteButton->setEnabled(QApplication::clipboard()->mimeData()->hasUrls());
    m_deleteButton->setDisabled(true);
    m_renameButton->setDisabled(true);
    m_backButton->setDisabled(true);
    m_forthButton->setDisabled(true);

    setRootPath(QApplication::applicationDirPath());
}

void FileExplorer::setRootPath(const QString& rootPath)
{
    if (m_fileSystemModel->rootPath() == rootPath)
        return;

    Q_ASSERT(exists(rootPath));

    if (selectionModel()) {
        selectionModel()->clear();
        selectionModel()->disconnect(this, SLOT(onFileSelectionChange()));
    }

    setRootIndex(QModelIndex());
    lastSelectedIndexesOfExplorer.clear();
    lastSelectedIndexesOfViewer.clear();
    lastExpandedIndexesOfViewer.clear();
    lastPathofExplorer.clear();
    lastVScrollerPosOfViewer = 0;
    lastHScrollerPosOfViewer = 0;
    lastVScrollerPosOfExplorer = 0;
    lastHScrollerPosOfExplorer = 0;
    backPathStack.clear();
    forthPathStack.clear();

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

QString FileExplorer::rootPath() const
{
    return m_fileSystemModel->rootPath();
}

void FileExplorer::onModeChange()
{
    Q_D(FileExplorer);

    if (m_modeComboBox->currentIndex() == 0)
        m_mode = Viewer;
    else
        m_mode = Explorer;

    if (m_mode == Viewer) {
        lastPathofExplorer = m_fileSystemModel->filePath(mt(rootIndex()));
        lastSelectedIndexesOfExplorer = selectedIndexes();
        lastVScrollerPosOfExplorer = verticalScrollBar()->sliderPosition();
        lastHScrollerPosOfExplorer = horizontalScrollBar()->sliderPosition();;

        onHomeButtonClick();

        for (const QPersistentModelIndex& index : lastExpandedIndexesOfViewer) {
            if (index.isValid())
                setExpanded(index, true);
        }

        selectionModel()->clear();
        for (const QModelIndex& index : lastSelectedIndexesOfViewer) {
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

        verticalScrollBar()->setSliderPosition(lastVScrollerPosOfViewer);
        horizontalScrollBar()->setSliderPosition(lastHScrollerPosOfViewer);
    } else {
        lastExpandedIndexesOfViewer = d->expandedIndexes;
        lastSelectedIndexesOfViewer = selectedIndexes();
        lastVScrollerPosOfViewer = verticalScrollBar()->sliderPosition();
        lastHScrollerPosOfViewer = horizontalScrollBar()->sliderPosition();;

        goToPath(lastPathofExplorer);

        selectionModel()->clear();
        for (const QModelIndex& index : lastSelectedIndexesOfExplorer) {
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

        verticalScrollBar()->setSliderPosition(lastVScrollerPosOfExplorer);
        horizontalScrollBar()->setSliderPosition(lastHScrollerPosOfExplorer);
    }

    updateGeometries();
}

void FileExplorer::onUpButtonClick()
{
    const QString& upperDir = dname(m_fileSystemModel->filePath(mt(rootIndex())));
    const QString& rootPath = m_fileSystemModel->rootPath();

    if (upperDir.size() > rootPath.size() || upperDir == rootPath)
        goToPath(upperDir);
}

void FileExplorer::onBackButtonClick()
{
    if (backPathStack.isEmpty())
        return;

    const QStack<QString> cloneOfForth = forthPathStack;
    const QString& rootPath = m_fileSystemModel->filePath(mt(rootIndex()));

    goToPath(backPathStack.pop());
    backPathStack.pop();

    forthPathStack = cloneOfForth;
    if (!rootPath.isEmpty())
        forthPathStack.push(rootPath);

    m_backButton->setDisabled(backPathStack.isEmpty());
    m_forthButton->setDisabled(forthPathStack.isEmpty());
}

void FileExplorer::onForthButtonClick()
{
    if (forthPathStack.isEmpty())
        return;

    const QStack<QString> cloneOfForth = forthPathStack;
    goToPath(forthPathStack.pop());
    forthPathStack = cloneOfForth;
    forthPathStack.pop();
    m_forthButton->setDisabled(forthPathStack.isEmpty());
}

void FileExplorer::onHomeButtonClick()
{
    goToPath(m_fileSystemModel->rootPath());
}

void FileExplorer::onCopyButtonClick()
{
    QSet<QUrl> urls;
    for (const QModelIndex& index : selectedIndexes())
        urls.insert(QUrl::fromLocalFile(m_fileSystemModel->filePath(mt(index))));

    if (!urls.isEmpty()) {
        auto mimeData = new QMimeData;
        mimeData->setUrls(urls.toList());
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
    int result = QMessageBox::warning(this,
                                      tr("Files about to be deleted"),
                                      tr("Do you want to delete selected files? "
                                         "Changes made in Code Editor also will be discarded."),
                                      QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        QSet<QString> deletedFiles;
        for (const QModelIndex& index : selectedIndexes()) {
            if (index.isValid()) {
                deletedFiles.insert(m_fileSystemModel->filePath(mt(index)));
                m_fileSystemModel->remove(mt(index));
            }
        }
        emit filesDeleted(deletedFiles);
    }
}

void FileExplorer::onRenameButtonClick()
{
    scrollTo(currentIndex());
    edit(currentIndex());
}

void FileExplorer::onNewFileButtonClick()
{
    QString baseFileName = tr("Empty File");
    const QString& rootPath = m_fileSystemModel->filePath(mt(rootIndex()));

    while (exists(rootPath + separator() + baseFileName + ".txt"))
        baseFileName = UtilityFunctions::increasedNumberedText(baseFileName, true, true);

    QModelIndex index;
    const bool suceed = mkfile(rootPath + separator() + baseFileName + ".txt");

    if (suceed)
        index = mf(m_fileSystemModel->index(rootPath + separator() + baseFileName + ".txt"));

    if (index.isValid()) {
        selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
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

    while (exists(rootPath + separator() + baseFolderName))
        baseFolderName = UtilityFunctions::increasedNumberedText(baseFolderName, true, true);

    const QModelIndex& index = mf(m_fileSystemModel->mkdir(mt(rootIndex()), baseFolderName));
    if (index.isValid()) {
        selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        scrollTo(index, PositionAtCenter);
        edit(index);
    } else {
        qWarning() << "FileExplorer:" << tr("Folder creation failed");
    }
}

void FileExplorer::onDownloadButtonClick()
{
    const QString& rootPath = m_fileSystemModel->filePath(mt(rootIndex()));

    if (rootPath.isEmpty())
        return;

    bool dialogOkButtonClicked;
    const QString& url = QInputDialog::getText(this, tr("Download a file"), tr("Url:"),
                                               QLineEdit::Normal, QString(), &dialogOkButtonClicked);

    if (!dialogOkButtonClicked)
        return;

    if (url.isEmpty())
        return;

    const QString& fileName = QInputDialog::getText(this, tr("Download a file"), tr("File name:"),
                                                    QLineEdit::Normal, QString(), &dialogOkButtonClicked);

    if (!dialogOkButtonClicked)
        return;

    if (fileName.isEmpty())
        return;

    QTemporaryDir tmp;
    Q_ASSERT_X(tmp.isValid(), "FileExplorer", "Cannot create a temporary dir.");

    if (!wrfile(tmp.filePath(fileName), dlfile(url))) {
        qWarning() << tr("File downlod failed.");
        return;
    }

    UtilityFunctions::copyFiles(rootPath, QList<QUrl>() << QUrl::fromLocalFile(tmp.filePath(fileName)), this);
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
        return goToPath(m_fileSystemModel->filePath(mt(index)));
    if (!m_fileSystemModel->isDir(mt(index)))
        emit fileOpened(QDir(rootPath()).relativeFilePath(m_fileSystemModel->filePath(mt(index))));
}

void FileExplorer::setPalette(const QPalette& pal)
{    
    QWidget::setPalette(pal);
    m_pathIndicator->setPalette(pal);

    QPixmap icon = PaintUtils::renderMaskedPixmap(":/utils/images/filtericon@2x.png",
                                                          pal.buttonText().color(), this);
    icon.setDevicePixelRatio(devicePixelRatioF());
    g_modeIFilterIconLabel->setPixmap(icon.scaled(16 * devicePixelRatioF(),
                                                  16 * devicePixelRatioF(),
                                                  Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    QPalette mp(m_modeComboBox->palette());
    mp.setColor(QPalette::Text, pal.text().color());
    mp.setColor(QPalette::WindowText, pal.windowText().color());
    mp.setColor(QPalette::ButtonText, pal.buttonText().color());
    m_modeComboBox->setPalette(mp);

    TransparentStyle::attach(m_modeComboBox);

    QString styleSheet = {
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
    };

    styleSheet = styleSheet
            .arg(palette().dark().color().darker(120).name())
            .arg(palette().light().color().name())
            .arg(palette().dark().color().name())
            .arg(palette().buttonText().color().name());

    setStyleSheet(styleSheet);

    update();
}

void FileExplorer::fillBackground(QPainter* painter, const QStyleOptionViewItem& option, int row, bool verticalLine) const
{
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
    QColor lineColor(pal.mid().color());
    lineColor.setAlpha(50);
    painter->setPen(lineColor);
    if (row != 0)
        painter->drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
    painter->drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});

    // Draw vertical line
    if (verticalLine) {
        painter->drawLine(rect.topRight() + QPointF(-0.5, 0.5),
                          rect.bottomRight() + QPointF(-0.5, -0.5));
    }

    painter->setClipping(false);
}

void FileExplorer::goToPath(const QString& path)
{
    if (!model())
        return;

    if (path.isEmpty())
        return;

    if (!QFileInfo(path).exists() || !QFileInfo(path).isDir())
        return;

    const QString& previousPath = m_fileSystemModel->filePath(mt(rootIndex()));
    const QModelIndex& index = m_fileSystemModel->index(path);

    if (previousPath == QFileInfo(path).canonicalFilePath())
        return;

    if (!index.isValid())
        return;

    setRootIndex(mf(index));
    selectionModel()->clear();
    m_searchEditCompleterModel->setRootPath(path);
    m_upButton->setDisabled(m_fileSystemModel->index(m_fileSystemModel->rootPath()) == index);
    m_homeButton->setDisabled(m_fileSystemModel->index(m_fileSystemModel->rootPath()) == index);
    m_pathIndicator->setPath(QDir(m_fileSystemModel->rootPath()).relativeFilePath(path));

    if (!previousPath.isEmpty()) {
        if (backPathStack.isEmpty() || backPathStack.top() != previousPath)
            backPathStack.push(previousPath);
    }

    forthPathStack.clear();

    m_backButton->setDisabled(backPathStack.isEmpty());
    m_forthButton->setDisabled(forthPathStack.isEmpty());
}

void FileExplorer::goToRelativePath(const QString& relativePath)
{
    const QString& path = m_fileSystemModel->rootPath() + separator() + relativePath;

    if (!QFileInfo(path).exists() || !QFileInfo(path).isDir())
        return;

    if (!QFileInfo(path).canonicalFilePath().contains(m_fileSystemModel->rootPath(), Qt::CaseInsensitive))
        return; // Protection against "rootPath/../../.." etc

    goToPath(path);
}

void FileExplorer::filterList()
{
    if (!m_searchEditCompleter->currentIndex().isValid())
        return;

    const QModelIndex& currentIndex = m_searchEditCompleter->popup()->currentIndex();

    if (!currentIndex.isValid())
        return;

    const QString& fileName = currentIndex.data(Qt::EditRole).toString();
    const QString& relativePath = currentIndex.data(Qt::DisplayRole).toString();

    if (relativePath.isEmpty() || fileName.isEmpty())
        return;

    if (m_searchEdit->text() != fileName)
        return;

    const QString& path = QDir(m_fileSystemModel->filePath(mt(rootIndex()))).filePath(relativePath);
    const QModelIndex& searchedIndex = mf(m_fileSystemModel->index(path));

    if (m_mode == Viewer)
        UtilityFunctions::expandUpToRoot(this, searchedIndex, rootIndex());
    else
        goToPath(dname(path));
    selectionModel()->select(searchedIndex, QItemSelectionModel::ClearAndSelect);
    scrollTo(searchedIndex, PositionAtCenter);
}

void FileExplorer::dropEvent(QDropEvent* event)
{
    const QString& rootPath = m_fileSystemModel->filePath(mt(rootIndex()));
    if (event->mimeData()->hasUrls()
            && !(event->mimeData()->hasText()
                 && event->mimeData()->text() == TOOL_KEY)) {
        event->accept();
        UtilityFunctions::copyFiles(rootPath, event->mimeData()->urls(), this);
    }
    m_dropHereLabel->setHidden(true);
    m_droppingBlurEffect->setEnabled(false);
}

void FileExplorer::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()
            && !(event->mimeData()->hasText()
                 && event->mimeData()->text() == TOOL_KEY)) {
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
    Q_D(const FileExplorer);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const qreal width = 10;
    const bool hasChild = m_fileSystemModel->hasChildren(mt(index));
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

void FileExplorer::paintEvent(QPaintEvent* e)
{
    const bool folderHasChildren = m_fileSystemModel->QAbstractItemModel::hasChildren(mt(rootIndex()));
    QPainter painter(viewport());
    painter.fillRect(rect(), palette().base());
    painter.setClipping(true);

    QColor lineColor(palette().mid().color());
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
        } else if (!folderHasChildren) {
            if (i == int((rowCount - 1) / 2.0) || i == int((rowCount - 1) / 2.0) + 1) {
                painter.setPen(palette().mid().color());
                painter.drawText(rect, Qt::AlignCenter, tr("Empty folder"));
                painter.setPen(lineColor);
            }
        }

        // Draw top and bottom lines
        painter.drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
        painter.drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});
    }
    painter.end();

    QTreeView::paintEvent(e);
}

void FileExplorer::updateGeometries()
{
    QTreeView::updateGeometries();
    QMargins vm = viewportMargins();
    vm.setBottom(m_searchEdit->height());
    vm.setTop(header()->height() + (m_mode == Explorer ? m_pathIndicator->height() + m_toolBar->height() - 2 : 0));
    setViewportMargins(vm);

    QRect vg = viewport()->geometry();
    QRect sg(vg.left(), vg.bottom(), vg.width(), m_searchEdit->height());
    m_searchEdit->setGeometry(sg);

    header()->setGeometry(vg.left(), 1, vg.width(), header()->height());
    m_modeComboBox->move(header()->width() - m_modeComboBox->width(),
                         header()->height() / 2.0 - m_modeComboBox->height() / 2.0);
    if (width() < m_modeComboBox->width() + 40) {
        m_modeComboBox->hide();
    } else if (m_modeComboBox->isHidden()) {
        m_modeComboBox->show();
        QPalette mp(m_modeComboBox->palette());
        mp.setColor(QPalette::Text, palette().text().color());
        mp.setColor(QPalette::WindowText, palette().windowText().color());
        mp.setColor(QPalette::ButtonText, palette().buttonText().color());
        m_modeComboBox->setPalette(mp); // TODO: Remove that, it's a weird workaround however
    }

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