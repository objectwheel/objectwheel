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
    Block deletion of qmldir within Global dir
    Improve the performance of FileSearchModel::updateModel, don't iterate over all the files
    - on the file system for each QFileSystemWatcher's "changed" signals
*/

#include <globalresourcespane.h>
#include <focuslesslineedit.h>
#include <projectmanager.h>
#include <saveutils.h>
#include <filemanager.h>
#include <wfw.h>
#include <transparentstyle.h>
#include <utilsicons.h>
#include <delayer.h>

#include <QComboBox>
#include <QPainter>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QFileSystemModel>
#include <QToolBar>
#include <QToolButton>
#include <QLabel>
#include <QMessageBox>
#include <QScrollBar>
#include <QClipboard>
#include <QSortFilterProxyModel>
#include <QInputDialog>
#include <QGraphicsBlurEffect>
#include <QtConcurrent>
#include <QProgressDialog>
#include <QCompleter>
#include <QDirIterator>
#include <QFileIconProvider>

#define mt(index) m_fileSystemProxyModel->mapToSource(index)
#define mf(index) m_fileSystemProxyModel->mapFromSource(index)

extern const char* TOOL_KEY;

using namespace Utils;

namespace {

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
    palette.setColor(QPalette::Midlight, "#f6f6f6"); // For PathIndicator's background
    palette.setColor(QPalette::Shadow, "#c4c4c4"); // For PathIndicator's border
    widget->setPalette(palette);
}

void fillBackground(QPainter* painter, const QStyleOptionViewItem& option, int row, bool verticalLine)
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

    painter->setClipping(false);
}

void copyFiles(const QString& rootPath, const QList<QUrl>& urls, QWidget* parent)
{
    QProgressDialog progress("Copying files...", "Abort Copy", 0, urls.size(), parent);
    progress.setWindowModality(Qt::NonModal);
    progress.open();
    Delayer::delay(100);

    bool askForOverwrite = true;

    for (int i = 0; i < urls.size(); i++) {
        const QUrl& url = urls.at(i);

        progress.setValue(i);

        if (progress.wasCanceled())
            break;

        if (url.isEmpty())
            continue;

        if (!url.isValid())
            continue;

        if (!url.isLocalFile())
            continue;

        const QString& path = QFileInfo(url.toLocalFile()).canonicalFilePath();
        const QString& fileName = fname(path);
        const QString& destPath = rootPath + separator() + fileName;

        if (exists(destPath)) {
            if (askForOverwrite) {
                int ret = QMessageBox::question(
                            parent,
                            QObject::tr("File or folder exists"),
                            QObject::tr("File or folder exists. "
                                        "Would you like to overwrite following file/folder: ") + fileName,
                            QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll| QMessageBox::Abort,
                            QMessageBox::No);
                if (ret == QMessageBox::Yes) {
                    rm(destPath);
                } else if (ret == QMessageBox::No) {
                    continue;
                } else if (ret == QMessageBox::YesToAll) {
                    askForOverwrite = false;
                    rm(destPath);
                } else {
                    break;
                }
            } else {
                rm(destPath);
            }
        }

        QFuture<void> future = QtConcurrent::run(qOverload<const QString&, const QString&, bool, bool>(&cp),
                                                 path, rootPath, false, false);
        Delayer::delay(std::bind(&QFuture<void>::isRunning, &future));
    }

    progress.setValue(urls.size());
    Delayer::delay(100);
}

void expandUpToRoot(QTreeView* view, const QModelIndex& index, const QModelIndex& rootIndex)
{
    if (!index.isValid())
        return;

    if (index == rootIndex)
        return;

    view->expand(index);

    expandUpToRoot(view, index.parent(), rootIndex);
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

        // Fill background
        fillBackground(painter, option, m_globalPane->d_func()->viewIndex(index), index.column() == 0);

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
      , m_pathEdit(new QLineEdit(this))
    {
        m_pathEdit->hide();
        connect(m_pathEdit, &QLineEdit::editingFinished,
                m_pathEdit, &QLineEdit::hide);
        connect(m_pathEdit, &QLineEdit::editingFinished,
                this, [=] { emit pathUpdated(m_pathEdit->text()); });
    }

    void setPath(const QString& path)
    {
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
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), palette().midlight().color());
        painter.setPen(palette().shadow().color());
        painter.drawLine(QRectF(rect()).bottomLeft(), QRectF(rect()).bottomRight());
        painter.setPen(palette().text().color());

        if (m_pathEdit->isHidden()) {
            QTextOption textOption(Qt::AlignLeft | Qt::AlignVCenter);
            textOption.setWrapMode(QTextOption::NoWrap);
            painter.drawText(rect().adjusted(3, 0, -3, 0),
                             fontMetrics().elidedText(m_path, Qt::ElideLeft, width() - 6),
                             textOption);
        }
    }

    void resizeEvent(QResizeEvent* e) override
    {
        QWidget::resizeEvent(e);
        m_pathEdit->setGeometry(rect());
    }

    void mouseDoubleClickEvent(QMouseEvent* e) override
    {
        QWidget::mouseDoubleClickEvent(e);
        m_pathEdit->setText(m_path);
        m_pathEdit->show();
        m_pathEdit->setFocus();
    }

private:
    QString m_path;
    QLineEdit* m_pathEdit;
};

class FileSystemProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit FileSystemProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent)
    {}

private:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override
    { // Sort by "DirsFirst" rule
        if (sortColumn() == 0) {
            const QFileSystemModel* fileSystemModel = qobject_cast<QFileSystemModel*>(sourceModel());

            QFileInfo leftFileInfo  = fileSystemModel->fileInfo(left);
            QFileInfo rightFileInfo = fileSystemModel->fileInfo(right);

            if (leftFileInfo.isDir() && !rightFileInfo.isDir())
                return false;
            if (!leftFileInfo.isDir() && rightFileInfo.isDir())
                return true;

            return QString::localeAwareCompare(leftFileInfo.fileName(), rightFileInfo.fileName()) >= 0;
        }

        return !QSortFilterProxyModel::lessThan(left, right);
    }
};

class FileSearchModel : public QStringListModel
{
    Q_OBJECT

public:
    explicit FileSearchModel(QObject* parent = nullptr) : QStringListModel(parent)
      , m_fileSystemWatcher(new QFileSystemWatcher(this))
    {
        connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged,
                this, &FileSearchModel::updateModel);
        connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged,
                this, &FileSearchModel::updateModel);
    }

    void setRootPath(const QString& rootPath)
    {
        m_rootPath = rootPath;
        if (!m_fileSystemWatcher->files().isEmpty())
            m_fileSystemWatcher->removePaths(m_fileSystemWatcher->files());
        if (!m_fileSystemWatcher->directories().isEmpty())
            m_fileSystemWatcher->removePaths(m_fileSystemWatcher->directories());
        if (!m_rootPath.isEmpty())
            m_fileSystemWatcher->addPath(m_rootPath);
        updateModel();
    }

private slots:
    void updateModel()
    {
        QStringList files;
        if (!m_rootPath.isEmpty()) {
            QDirIterator it(m_rootPath, {"*"}, QDir::AllEntries | QDir::NoSymLinks
                            | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (it.hasNext())
                files << it.next();
        }
        setStringList(files);
    }

private:
    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role == Qt::EditRole) {
            QString path = QDir::toNativeSeparators(QStringListModel::data(index, Qt::EditRole).toString());
            if (path.endsWith(QDir::separator()))
                path.chop(1);
            return fname(path);
        }

        if (role == Qt::DisplayRole) {
            QString path = QDir::toNativeSeparators(QStringListModel::data(index, Qt::EditRole).toString());
            if (path.endsWith(QDir::separator()))
                path.chop(1);
            return QDir(m_rootPath).relativeFilePath(path);
        }

        if (role == Qt::DecorationRole) {
            QString path = QDir::toNativeSeparators(QStringListModel::data(index, Qt::EditRole).toString());
            if (path.endsWith(QDir::separator()))
                path.chop(1);
            return m_fileIconProvider.icon(QFileInfo(path));
        }

        return QStringListModel::data(index, role);
    }

private:
    QString m_rootPath;
    QFileIconProvider m_fileIconProvider;
    QFileSystemWatcher* m_fileSystemWatcher;
};

GlobalResourcesPane::GlobalResourcesPane(QWidget* parent) : QTreeView(parent)
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
    initPalette(this);
    initPalette(m_pathIndicator);

    QFont fontMedium(font());
    fontMedium.setWeight(QFont::Medium);

    header()->setFont(fontMedium);
    header()->setFixedHeight(23);
    header()->setDefaultSectionSize(1);
    header()->setMinimumSectionSize(1);
    header()->setSectionsMovable(false);

    setIconSize({15, 15});
    setDragEnabled(false);
    setSortingEnabled(true);
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
                    "} QHeaderView::down-arrow {"
                    "    image: none;"
                    "} QHeaderView::up-arrow {"
                    "    image: none;"
                    "}"
                }
                .arg(palette().text().color().lighter(270).name())
                .arg(palette().light().color().name())
                .arg(palette().dark().color().name())
                .arg(palette().brightText().color().name()));

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

    QPalette mp(m_modeComboBox->palette());
    mp.setColor(QPalette::Text, Qt::white);
    mp.setColor(QPalette::WindowText, Qt::white);
    mp.setColor(QPalette::ButtonText, Qt::white);
    m_modeComboBox->setPalette(mp);
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

    const Icon FILTER_ICON({{":/utils/images/filtericon.png", Theme::BackgroundColorNormal}});
    auto modeIFilterIconLabel = new QLabel(m_modeComboBox);
    modeIFilterIconLabel->setPixmap(FILTER_ICON.pixmap());
    modeIFilterIconLabel->setFixedSize(16, 16);
    modeIFilterIconLabel->move(0, 3);
    m_modeComboBox->setContentsMargins(16, 0, 0, 0);

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
            this, &GlobalResourcesPane::onUpButtonClick);
    connect(m_backButton, &QToolButton::clicked,
            this, &GlobalResourcesPane::onBackButtonClick);
    connect(m_forthButton, &QToolButton::clicked,
            this, &GlobalResourcesPane::onForthButtonClick);
    connect(m_homeButton, &QToolButton::clicked,
            this, &GlobalResourcesPane::onHomeButtonClick);
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
    m_backButton->setFixedHeight(22);
    m_forthButton->setFixedHeight(22);
    m_homeButton->setFixedHeight(22);
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
    m_toolBar->addWidget(m_downloadFileButton);

    m_fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    m_fileSystemProxyModel->setDynamicSortFilter(true);
    m_fileSystemProxyModel->setFilterKeyColumn(0);
    m_fileSystemProxyModel->setSourceModel(m_fileSystemModel);
    connect(m_fileSystemModel, &QFileSystemModel::fileRenamed, this, [=]
    {
        m_fileSystemProxyModel->setDynamicSortFilter(false);
        m_fileSystemProxyModel->setDynamicSortFilter(true);
    });

    m_searchEditCompleter->setFilterMode(Qt::MatchContains);
    m_searchEditCompleter->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    m_searchEditCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_searchEditCompleter->setModel(m_searchEditCompleterModel);
    m_searchEditCompleter->popup()->setIconSize({15, 15});
    m_searchEdit->setCompleter(m_searchEditCompleter);
    m_searchEdit->setPlaceholderText("Filter");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(22);
    connect(m_searchEdit, qOverload<>(&FocuslessLineEdit::editingFinished), this, &GlobalResourcesPane::filterList);
    connect(m_pathIndicator, &PathIndicator::pathUpdated, this, &GlobalResourcesPane::goToRelativePath);

    connect(ProjectManager::instance(), &ProjectManager::started,
            this, &GlobalResourcesPane::onProjectStart);
    connect(this, &GlobalResourcesPane::doubleClicked,
            this, &GlobalResourcesPane::onItemDoubleClick);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [=]
    { m_pasteButton->setEnabled(QApplication::clipboard()->mimeData()->hasUrls()); });
}

void GlobalResourcesPane::sweep()
{
    // TODO
    m_searchEdit->clear();
    m_searchEditCompleterModel->setRootPath(QString());

    if (selectionModel()) {
        selectionModel()->clear();
        selectionModel()->disconnect(this);
    }
    setModel(nullptr);

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
}

void GlobalResourcesPane::onProjectStart()
{
    Q_ASSERT(exists(SaveUtils::toGlobalDir(ProjectManager::dir())));
    m_fileSystemModel->setRootPath(SaveUtils::toGlobalDir(ProjectManager::dir()));

    setModel(m_fileSystemProxyModel);
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

void GlobalResourcesPane::onUpButtonClick()
{
    const QString& upperDir = dname(m_fileSystemModel->filePath(mt(rootIndex())));
    const QString& rootPath = m_fileSystemModel->rootPath();

    if (upperDir.size() > rootPath.size() || upperDir == rootPath)
        goToPath(upperDir);
}

void GlobalResourcesPane::onBackButtonClick()
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

void GlobalResourcesPane::onForthButtonClick()
{
    if (forthPathStack.isEmpty())
        return;

    const QStack<QString> cloneOfForth = forthPathStack;
    goToPath(forthPathStack.pop());
    forthPathStack = cloneOfForth;
    forthPathStack.pop();
    m_forthButton->setDisabled(forthPathStack.isEmpty());
}

void GlobalResourcesPane::onHomeButtonClick()
{
    goToPath(m_fileSystemModel->rootPath());
}

void GlobalResourcesPane::onCopyButtonClick()
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

void GlobalResourcesPane::onPasteButtonClick()
{
    const QString& rootPath = m_fileSystemModel->filePath(mt(rootIndex()));
    const QMimeData* clipboard = QApplication::clipboard()->mimeData();
    if (clipboard && clipboard->hasUrls())
        copyFiles(rootPath, clipboard->urls(), this);
}

void GlobalResourcesPane::onDeleteButtonClick()
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

void GlobalResourcesPane::onRenameButtonClick()
{
    scrollTo(currentIndex());
    edit(currentIndex());
}

void GlobalResourcesPane::onNewFileButtonClick()
{
    QString fileName = tr("Empty File.txt");
    const QString& rootPath = m_fileSystemModel->filePath(mt(rootIndex()));

    for (int i = 1; exists(rootPath + separator() + fileName); i++)
        fileName = tr("Empty File ") + QString::number(i) + ".txt";

    QModelIndex index;
    const bool suceed = mkfile(rootPath + separator() + fileName);

    if (suceed)
        index = mf(m_fileSystemModel->index(rootPath + separator() + fileName));

    if (index.isValid()) {
        selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        scrollTo(index, PositionAtCenter);
        edit(index);
    } else {
        qWarning() << "GlobalResourcesPane:" << tr("File creation failed");
    }
}

void GlobalResourcesPane::onNewFolderButtonClick()
{
    QString folderName = tr("New Folder");
    const QString& rootPath = m_fileSystemModel->filePath(mt(rootIndex()));

    for (int i = 1; exists(rootPath + separator() + folderName); i++)
        folderName = tr("New Folder ") + QString::number(i);

    const QModelIndex& index = mf(m_fileSystemModel->mkdir(mt(rootIndex()), folderName));
    if (index.isValid()) {
        selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        scrollTo(index, PositionAtCenter);
        edit(index);
    } else {
        qWarning() << "GlobalResourcesPane:" << tr("Folder creation failed");
    }
}

void GlobalResourcesPane::onDownloadButtonClick()
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
    Q_ASSERT_X(tmp.isValid(), "GlobalResourcesPane", "Cannot create a temporary dir.");

    if (!wrfile(tmp.filePath(fileName), dlfile(url))) {
        qWarning() << tr("File downlod failed.");
        return;
    }

    copyFiles(rootPath, QList<QUrl>() << QUrl::fromLocalFile(tmp.filePath(fileName)), this);
}

void GlobalResourcesPane::onFileSelectionChange()
{
    const int selectedItemSize = selectedIndexes().size();
    m_deleteButton->setEnabled(selectedItemSize > 0);
    m_copyButton->setEnabled(selectedItemSize > 0);
    m_renameButton->setEnabled(selectedItemSize == 1);
}

void GlobalResourcesPane::onItemDoubleClick(const QModelIndex& index)
{
    if (m_fileSystemModel->isDir(mt(index)) && m_mode == Explorer)
        goToPath(m_fileSystemModel->filePath(mt(index)));
    else
        emit fileOpened(m_fileSystemModel->filePath(mt(index)));
}

void GlobalResourcesPane::goToPath(const QString& path)
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

void GlobalResourcesPane::goToRelativePath(const QString& relativePath)
{
    const QString& path = m_fileSystemModel->rootPath() + separator() + relativePath;

    if (!QFileInfo(path).exists() || !QFileInfo(path).isDir())
        return;

    if (!QFileInfo(path).canonicalFilePath().contains(m_fileSystemModel->rootPath(), Qt::CaseInsensitive))
        return; // Protection against "rootPath/../../.." etc

    goToPath(path);
}

void GlobalResourcesPane::filterList()
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
         expandUpToRoot(this, searchedIndex, rootIndex());
     else
         goToPath(dname(path));
     selectionModel()->select(searchedIndex, QItemSelectionModel::ClearAndSelect);
     scrollTo(searchedIndex, PositionAtCenter);
}

void GlobalResourcesPane::dropEvent(QDropEvent* event)
{
    const QString& rootPath = m_fileSystemModel->filePath(mt(rootIndex()));
    if (event->mimeData()->hasUrls()
            && !(event->mimeData()->hasText()
                 && event->mimeData()->text() == TOOL_KEY)) {
        event->accept();
        copyFiles(rootPath, event->mimeData()->urls(), this);
    }
    m_dropHereLabel->setHidden(true);
    m_droppingBlurEffect->setEnabled(false);
}

void GlobalResourcesPane::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()
            && !(event->mimeData()->hasText()
                 && event->mimeData()->text() == TOOL_KEY)) {
        event->accept();
        m_dropHereLabel->setVisible(true);
        m_droppingBlurEffect->setEnabled(true);
    }
}

void GlobalResourcesPane::dragMoveEvent(QDragMoveEvent* event)
{
    event->accept();
}

void GlobalResourcesPane::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
    m_dropHereLabel->setHidden(true);
    m_droppingBlurEffect->setEnabled(false);
}

void GlobalResourcesPane::drawBranches(QPainter* painter, const QRect& rect,
                                       const QModelIndex& index) const
{
    Q_D(const GlobalResourcesPane);
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

void GlobalResourcesPane::paintEvent(QPaintEvent* e)
{
    const bool rootHasChildren = m_fileSystemModel->QAbstractItemModel::hasChildren(mt(rootIndex()));
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
    painter.end();

    QTreeView::paintEvent(e);
}

void GlobalResourcesPane::updateGeometries()
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

QSize GlobalResourcesPane::sizeHint() const
{
    return QSize{310, 280};
}

#include "globalresourcespane.moc"