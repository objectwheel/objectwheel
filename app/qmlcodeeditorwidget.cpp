#include <qmlcodeeditorwidget.h>
#include <designerscene.h>
#include <control.h>
#include <saveutils.h>
#include <centralwidget.h>
#include <filemanager.h>
#include <qmlcodeeditor.h>
#include <css.h>
#include <fileexplorer.h>
#include <control.h>
#include <parserutils.h>
#include <dpr.h>
#include <transparentcombobox.h>
#include <utilsicons.h>
#include <toolbar.h>
#include <toolbutton.h>
#include <appfontsettings.h>
#include <windowmanager.h>
#include <mainwindow.h>
#include <inspectorpane.h>
#include <controlpropertymanager.h>
#include <controlpreviewingmanager.h>

#include <QDebug>
#include <QVBoxLayout>
#include <QPainter>
#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QSplitter>
#include <QMessageBox>
#include <QScrollBar>

#define LINE_COLOR ("#606467")
#define CHAR_SEPARATION ("::")
#define CHAR_CHANGEINDICATOR ("*")
#define INITIALWIDTH_FILEEXPLORER (450)
#define MINWIDTH_FILEEXPLORER (200)
#define MINWIDTH_EDITOR (200)
#define UNKNOWN_PATH ("67asdta8d9yaghqbj4")
#define TAB_SPACE ("    ")

class QmlCodeEditorWidgetPrivate : public QObject
{
        Q_OBJECT

    public:
        QmlCodeEditorWidgetPrivate(QmlCodeEditorWidget* parent);

    private:
        qreal findPixelSize(const QString& text);

    public slots:
        void handleCursorPositionChanged();
        void handleModeChange();
        void handleFileExplorerFileOpen(const QString& filePath);
        void handleFileExplorerFileDeleted(const QString& filePath);
        void handleFileExplorerFileRenamed(const QString& filePathFrom, const QString& filePathTo);
        void handlePinButtonClicked();
        void handleCloseButtonClicked();
        void handleSaveButtonClicked();
        void handleZoomLevelChange(const QString& text);
        void handleHideShowButtonClicked();
        void handleCodeEditorButtonClicked();
        void handleImageEditorButtonClicked();
        void handleHexEditorButtonClicked();
        void updateOpenDocHistory();
        void handleItemsComboboxActivated(QString text);
        void handleDocumentsComboboxActivated(QString text);
        void handleControlRemoval(Control* control);
        void propertyUpdate(Control* control, const QString& property, const QString& value);

    public:
        QmlCodeEditorWidget* parent;
        QVBoxLayout* vBoxLayout;
        QWidget* containerWidget;
        QVBoxLayout* containerVBoxLayout;

        Control* currentControl;
        int lastWidthOfExplorerWrapper;
        QMetaObject::Connection previousUndoConnection;
        QMetaObject::Connection previousRedoConnection;
        QAction saveAction;

        ToolBar* toolbar;
        ToolButton* pinButton;
        ToolButton* undoButton;
        ToolButton* redoButton;
        ToolButton* closeButton;
        ToolButton* saveButton;
        ToolButton* cutButton;
        ToolButton* copyButton;
        ToolButton* pasteButton;
        QComboBox* itemsCombobox;
        QComboBox* documentsCombobox;
        QComboBox* zoomlLevelCombobox;
        QLabel* lineColLabel;

        QSplitter* splitter;
        QWidget* editorWrapper;
        QVBoxLayout* editorWrapperVBoxLayout;
        QmlCodeEditor* codeEditor;
        QWidget* imageEditor;
        QWidget* hexEditor;
        QLabel* noDocumentIndicator;

        QWidget* explorerWrapper;
        QHBoxLayout* explorerWrapperHBoxLayout;
        ToolBar* toolbar_2;
        ToolButton* hideShowButton;
        ToolButton* codeEditorButton;
        ToolButton* imageEditorButton;
        ToolButton* hexEditorButton;
        FileExplorer* fileExplorer;
        QmlCodeDocument* emptyDoc;
};

QmlCodeEditorWidgetPrivate::QmlCodeEditorWidgetPrivate(QmlCodeEditorWidget* parent)
    : QObject(parent)
    , parent(parent)
    , vBoxLayout(new QVBoxLayout(parent))
    , containerWidget(new QWidget)
    , containerVBoxLayout(new QVBoxLayout(containerWidget))
    , currentControl(nullptr)
    , lastWidthOfExplorerWrapper(INITIALWIDTH_FILEEXPLORER)
    , toolbar(new ToolBar)
    , pinButton(new ToolButton)
    , undoButton(new ToolButton)
    , redoButton(new ToolButton)
    , closeButton(new ToolButton)
    , saveButton(new ToolButton)
    , cutButton(new ToolButton)
    , copyButton(new ToolButton)
    , pasteButton(new ToolButton)
    , itemsCombobox(new TransparentComboBox)
    , documentsCombobox(new TransparentComboBox)
    , zoomlLevelCombobox(new TransparentComboBox)
    , lineColLabel(new QLabel)
    , splitter(new QSplitter)
    , editorWrapper(new QWidget)
    , editorWrapperVBoxLayout(new QVBoxLayout(editorWrapper))
    , codeEditor(new QmlCodeEditor)
    , imageEditor(new QWidget)
    , hexEditor(new QWidget)
    , noDocumentIndicator(new QLabel)
    , explorerWrapper(new QWidget)
    , explorerWrapperHBoxLayout(new QHBoxLayout(explorerWrapper))
    , toolbar_2(new ToolBar)
    , hideShowButton(new ToolButton)
    , codeEditorButton(new ToolButton)
    , imageEditorButton(new ToolButton)
    , hexEditorButton(new ToolButton)
    , fileExplorer(new FileExplorer)
    , emptyDoc(new QmlCodeDocument(codeEditor))
{
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->setSpacing(0);
    vBoxLayout->addWidget(containerWidget);

    containerVBoxLayout->setContentsMargins(0, 0, 0, 0);
    containerVBoxLayout->setSpacing(0);
    containerVBoxLayout->addWidget(toolbar);
    containerVBoxLayout->addWidget(splitter);

    containerWidget->setWindowTitle("Objectwheel Qml Editor");
    containerWidget->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint);
    containerWidget->addAction(&saveAction);

    saveAction.setText("Save document");
    saveAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    saveAction.setShortcut(QKeySequence::Save);

    splitter->addWidget(editorWrapper);
    splitter->addWidget(explorerWrapper);
    splitter->setCollapsible(0, false);;
    splitter->setCollapsible(1, false);;
    splitter->setHandleWidth(0);

    editorWrapperVBoxLayout->setContentsMargins(0, 0, 0, 0);
    editorWrapperVBoxLayout->setSpacing(0);
    editorWrapperVBoxLayout->addWidget(codeEditor);
    editorWrapperVBoxLayout->addWidget(imageEditor);
    editorWrapperVBoxLayout->addWidget(hexEditor);
    editorWrapperVBoxLayout->addWidget(noDocumentIndicator);

    explorerWrapperHBoxLayout->setContentsMargins(0, 0, 0, 0);
    explorerWrapperHBoxLayout->setSpacing(0);
    explorerWrapperHBoxLayout->addWidget(toolbar_2);
    explorerWrapperHBoxLayout->addWidget(fileExplorer);

    fileExplorer->hide();
    fileExplorer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    fileExplorer->setRootPath(UNKNOWN_PATH);

    itemsCombobox->setMinimumWidth(150);
    documentsCombobox->setMinimumWidth(150);
    zoomlLevelCombobox->setFixedWidth(100);

    redoButton->setDisabled(true);
    copyButton->setDisabled(true);
    cutButton->setDisabled(true);
    saveButton->setDisabled(true);

    codeEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    codeEditor->horizontalScrollBar()->setStyleSheet(CSS_SCROLLBAR_H);
    codeEditor->verticalScrollBar()->setStyleSheet(CSS_SCROLLBAR);
    codeEditor->setCodeDocument(emptyDoc);
    codeEditor->hide();

    noDocumentIndicator->setStyleSheet("QLabel { color: #606467; }");
    noDocumentIndicator->setText("No documents open");
    noDocumentIndicator->setAlignment(Qt::AlignCenter);
    noDocumentIndicator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    noDocumentIndicator->show();

    documentsCombobox->hide();
    itemsCombobox->hide();
    closeButton->hide();

    connect(parent, SIGNAL(modeChanged()), SLOT(handleModeChange()));
    connect(codeEditor, SIGNAL(cursorPositionChanged()), SLOT(handleCursorPositionChanged()));
    connect(pinButton, SIGNAL(clicked(bool)), SLOT(handlePinButtonClicked()));
    connect(undoButton, SIGNAL(clicked(bool)), codeEditor, SLOT(undo()));
    connect(redoButton, SIGNAL(clicked(bool)), codeEditor, SLOT(redo()));
    connect(copyButton, SIGNAL(clicked(bool)), codeEditor, SLOT(copy()));
    connect(cutButton, SIGNAL(clicked(bool)), codeEditor, SLOT(cut()));
    connect(pasteButton, SIGNAL(clicked(bool)), codeEditor, SLOT(paste()));
    connect(codeEditor, SIGNAL(copyAvailable(bool)), copyButton, SLOT(setEnabled(bool)));
    connect(codeEditor, SIGNAL(copyAvailable(bool)), cutButton, SLOT(setEnabled(bool)));
    connect(zoomlLevelCombobox, SIGNAL(activated(QString)), SLOT(handleZoomLevelChange(QString)));
    connect(hideShowButton, SIGNAL(clicked(bool)), SLOT(handleHideShowButtonClicked()));
    connect(closeButton, SIGNAL(clicked(bool)), SLOT(handleCloseButtonClicked()));
    connect(saveButton, SIGNAL(clicked(bool)), SLOT(handleSaveButtonClicked()));
    connect(codeEditorButton, SIGNAL(clicked(bool)), SLOT(handleCodeEditorButtonClicked()));
    connect(imageEditorButton, SIGNAL(clicked(bool)), SLOT(handleImageEditorButtonClicked()));
    connect(hexEditorButton, SIGNAL(clicked(bool)), SLOT(handleHexEditorButtonClicked()));
    connect(fileExplorer, SIGNAL(fileOpened(QString)), SLOT(handleFileExplorerFileOpen(QString)));
    connect(fileExplorer, SIGNAL(fileDeleted(QString)), SLOT(handleFileExplorerFileDeleted(QString)));
    connect(fileExplorer, SIGNAL(fileRenamed(QString,QString)), SLOT(handleFileExplorerFileRenamed(QString,QString)));
    connect(itemsCombobox, SIGNAL(activated(QString)), SLOT(handleItemsComboboxActivated(QString)));
    connect(documentsCombobox, SIGNAL(activated(QString)), SLOT(handleDocumentsComboboxActivated(QString)), Qt::QueuedConnection);
    connect(&saveAction, SIGNAL(triggered()), SLOT(handleSaveButtonClicked()));
//    FIXME connect(SaveManager::instance(), SIGNAL(databaseChanged()), SLOT(updateOpenDocHistory()));
    // FIXME connect(SaveManager::instance(), SIGNAL(propertyChanged(Control*,QString,QString)), SLOT(propertyUpdate(Control*,QString,QString)));

    zoomlLevelCombobox->addItem("35 %");
    zoomlLevelCombobox->addItem("50 %");
    zoomlLevelCombobox->addItem("65 %");
    zoomlLevelCombobox->addItem("75 %");
    zoomlLevelCombobox->addItem("90 %");
    zoomlLevelCombobox->addItem("100 %");
    zoomlLevelCombobox->addItem("110 %");
    zoomlLevelCombobox->addItem("120 %");
    zoomlLevelCombobox->addItem("140 %");
    zoomlLevelCombobox->addItem("170 %");
    zoomlLevelCombobox->addItem("200 %");
    zoomlLevelCombobox->addItem("250 %");
    zoomlLevelCombobox->addItem("300 %");
    zoomlLevelCombobox->addItem("400 %");
    zoomlLevelCombobox->addItem("500 %");
    zoomlLevelCombobox->setCurrentIndex(5);

    pinButton->setCursor(Qt::PointingHandCursor);
    undoButton->setCursor(Qt::PointingHandCursor);
    redoButton->setCursor(Qt::PointingHandCursor);
    closeButton->setCursor(Qt::PointingHandCursor);
    zoomlLevelCombobox->setCursor(Qt::PointingHandCursor);
    documentsCombobox->setCursor(Qt::PointingHandCursor);
    itemsCombobox->setCursor(Qt::PointingHandCursor);
    saveButton->setCursor(Qt::PointingHandCursor);
    cutButton->setCursor(Qt::PointingHandCursor);
    copyButton->setCursor(Qt::PointingHandCursor);
    pasteButton->setCursor(Qt::PointingHandCursor);

    pinButton->setToolTip("Unpin Editor.");
    undoButton->setToolTip("Undo action.");
    redoButton->setToolTip("Redo action.");
    closeButton->setToolTip("Close document.");
    zoomlLevelCombobox->setToolTip("Change zoom level.");
    documentsCombobox->setToolTip("See open document.");
    itemsCombobox->setToolTip("See open controls.");
    lineColLabel->setToolTip("Cursor line/column indicator.");
    saveButton->setToolTip("Save document.");
    cutButton->setToolTip("Cut selection.");
    copyButton->setToolTip("Copy selection.");
    pasteButton->setToolTip("Paste from clipboard.");

    pinButton->setIcon(Utils::Icons::PIN_TOOLBAR.icon());
    undoButton->setIcon(Utils::Icons::UNDO_TOOLBAR.icon());
    redoButton->setIcon(Utils::Icons::REDO_TOOLBAR.icon());
    closeButton->setIcon(Utils::Icons::CLOSE_TOOLBAR.icon());
    saveButton->setIcon(Utils::Icons::SAVEFILE_TOOLBAR.icon());
    cutButton->setIcon(Utils::Icons::CUT_TOOLBAR.icon());
    copyButton->setIcon(Utils::Icons::COPY_TOOLBAR.icon());
    pasteButton->setIcon(Utils::Icons::PASTE_TOOLBAR.icon());

    toolbar->setFixedHeight(24);
    toolbar->addWidget(pinButton);
    toolbar->addSeparator();
    toolbar->addWidget(undoButton);
    toolbar->addWidget(redoButton);
    toolbar->addSeparator();
    toolbar->addWidget(cutButton);
    toolbar->addWidget(copyButton);
    toolbar->addWidget(pasteButton);
    toolbar->addWidget(saveButton);
    toolbar->addSeparator();
    toolbar->addWidget(itemsCombobox);
    toolbar->addWidget(documentsCombobox);
    toolbar->addWidget(closeButton);
    toolbar->addSeparator();
    toolbar->addWidget(zoomlLevelCombobox);
    toolbar->addStretch();
    toolbar->addWidget(lineColLabel);

    // Toolbar_2 assets
    codeEditorButton->setCheckable(true);
    imageEditorButton->setCheckable(true);
    hexEditorButton->setCheckable(true);
    codeEditorButton->setChecked(true);
    codeEditorButton->setDisabled(true);

    hideShowButton->setCursor(Qt::PointingHandCursor);
    codeEditorButton->setCursor(Qt::PointingHandCursor);
    imageEditorButton->setCursor(Qt::PointingHandCursor);
    hexEditorButton->setCursor(Qt::PointingHandCursor);

    hideShowButton->setToolTip("Show File Explorer.");
    codeEditorButton->setToolTip("Open Text Editor.");
    imageEditorButton->setToolTip("Open Image Editor.");
    hexEditorButton->setToolTip("Open Hex Editor.");

    hideShowButton->setIcon(Utils::Icons::CLOSE_SPLIT_LEFT.icon());
    codeEditorButton->setIcon(QIcon(":/images/code.png"));
    imageEditorButton->setIcon(QIcon(":/images/image.png"));
    hexEditorButton->setIcon(QIcon(":/images/hex.png"));

    toolbar_2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    toolbar_2->setOrientation(Qt::Vertical);
    toolbar_2->setFixedWidth(24);
    toolbar_2->addWidget(hideShowButton);
    toolbar_2->addSeparator();
    toolbar_2->addWidget(codeEditorButton);
    toolbar_2->addWidget(imageEditorButton);
    toolbar_2->addWidget(hexEditorButton);
    toolbar_2->addStretch();

    splitter->handle(1)->setDisabled(true);
    editorWrapper->setMinimumWidth(MINWIDTH_FILEEXPLORER);
    explorerWrapper->setFixedWidth(toolbar_2->width());
    connect(splitter, &QSplitter::splitterMoved, this, [=] {
        lastWidthOfExplorerWrapper = explorerWrapper->width();
    });

//    QTimer::singleShot(10000, [=] {
//        handlePinButtonClicked();
//    });
}

qreal QmlCodeEditorWidgetPrivate::findPixelSize(const QString& text)
{
    qreal base = AppFontSettings::defaultPixelSize() - 0.5;

    if (text == "35 %")
        return (base * 0.35);
    else if (text == "50 %")
        return (base * 0.50);
    else if (text == "65 %")
        return (base * 0.65);
    else if (text == "75 %")
        return (base * 0.75);
    else if (text == "90 %")
        return (base * 0.90);
    else if (text == "100 %")
        return base;
    else if (text == "110 %")
        return (base * 1.10);
    else if (text == "120 %")
        return (base * 1.20);
    else if (text == "140 %")
        return (base * 1.40);
    else if (text == "170 %")
        return (base * 1.70);
    else if (text == "200 %")
        return (base * 2.00);
    else if (text == "250 %")
        return (base * 2.50);
    else if (text == "300 %")
        return (base * 3.00);
    else if (text == "400 %")
        return (base * 4.00);
    else if (text == "500 %")
        return (base * 5.00);
    else
        return base;
}

void QmlCodeEditorWidgetPrivate::handleCursorPositionChanged()
{
    auto textCursor = codeEditor->textCursor();
    QString lineColText("# Line: %1, Col: %2");
    lineColLabel->setText(lineColText.arg(textCursor.blockNumber() + 1).arg(textCursor.columnNumber()));
}

void QmlCodeEditorWidgetPrivate::handlePinButtonClicked()
{
    if (pinButton->toolTip().contains("Unpin")) {
        pinButton->setToolTip("Pin Editor.");
        pinButton->setIcon(Utils::Icons::PINNED_TOOLBAR.icon());
        containerWidget->setParent(nullptr);
        containerWidget->setWindowIcon(QIcon(":/images/owicon.png"));
        containerWidget->show();

        containerWidget->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                containerWidget->size(),
                QGuiApplication::primaryScreen()->availableGeometry()
            )
        );
    } else {
        pinButton->setToolTip("Unpin Editor.");
        pinButton->setIcon(Utils::Icons::PIN_TOOLBAR.icon());
        vBoxLayout->addWidget(containerWidget);
    }
}

void QmlCodeEditorWidgetPrivate::handleCloseButtonClicked()
{
    for (auto& item : parent->_editorItems) {
        if (item.control == currentControl) {
            parent->closeDocument(item.control, item.control->dir() + separator() +
                                  DIR_THIS + separator() + item.currentFileRelativePath);
            break;
        }
    }
}

void QmlCodeEditorWidgetPrivate::handleSaveButtonClicked()
{
    for (auto& item : parent->_editorItems) {
        if (item.control == currentControl) {
            parent->saveDocument(item.control, item.control->dir() + separator() +
                                  DIR_THIS + separator() + item.currentFileRelativePath);
            break;
        }
    }
}

void QmlCodeEditorWidgetPrivate::handleZoomLevelChange(const QString& text)
{
//    defaultFont.setPixelSize(findPixelSize(text));
//    codeEditor->document()->setDefaultFont(defaultFont); BUG
}

void QmlCodeEditorWidgetPrivate::handleHideShowButtonClicked()
{
    if (hideShowButton->toolTip().contains("Hide")) {
        hideShowButton->setIcon(Utils::Icons::CLOSE_SPLIT_LEFT.icon());
        hideShowButton->setToolTip("Show File Explorer.");
        splitter->handle(1)->setDisabled(true);
        fileExplorer->hide();
        explorerWrapper->setFixedWidth(toolbar_2->width());
        QList<int> sizes;
        sizes << containerWidget->width() - toolbar_2->width();
        sizes << toolbar_2->width();
        splitter->setSizes(sizes);
    } else {
        hideShowButton->setIcon(Utils::Icons::CLOSE_SPLIT_RIGHT.icon());
        hideShowButton->setToolTip("Hide File Explorer.");
        splitter->handle(1)->setEnabled(true);
        fileExplorer->show();
        explorerWrapper->setMinimumWidth(MINWIDTH_FILEEXPLORER);
        explorerWrapper->setMaximumWidth(9999);
        QList<int> sizes;
        sizes << containerWidget->width() - lastWidthOfExplorerWrapper;
        sizes << lastWidthOfExplorerWrapper;
        splitter->setSizes(sizes);
    }
}

void QmlCodeEditorWidgetPrivate::handleModeChange()
{
    if (parent->_mode == QmlCodeEditorWidget::CodeEditor) {
        codeEditorButton->setChecked(true);
        codeEditorButton->setDisabled(true);
        imageEditorButton->setChecked(false);
        hexEditorButton->setChecked(false);
        imageEditorButton->setEnabled(true);
        hexEditorButton->setEnabled(true);

        imageEditor->hide();
        hexEditor->hide();
        if (noDocumentIndicator->isHidden())
            codeEditor->show();
    } else if (parent->_mode == QmlCodeEditorWidget::ImageEditor) {
        imageEditorButton->setChecked(true);
        imageEditorButton->setDisabled(true);
        codeEditorButton->setChecked(false);
        hexEditorButton->setChecked(false);
        codeEditorButton->setEnabled(true);
        hexEditorButton->setEnabled(true);

        codeEditor->hide();
        hexEditor->hide();
        imageEditor->show();
    } else {
        hexEditorButton->setChecked(true);
        hexEditorButton->setDisabled(true);
        imageEditorButton->setChecked(false);
        codeEditorButton->setChecked(false);
        imageEditorButton->setEnabled(true);
        codeEditorButton->setEnabled(true);

        imageEditor->hide();
        codeEditor->hide();
        hexEditor->show();
    }
}

void QmlCodeEditorWidgetPrivate::handleFileExplorerFileOpen(const QString& filePath)
{
    parent->addDocument(currentControl, filePath);
    parent->setCurrentDocument(currentControl, filePath);
}

void QmlCodeEditorWidgetPrivate::handleFileExplorerFileDeleted(const QString& filePath)
{
    parent->closeDocument(currentControl, filePath, false);
}

void QmlCodeEditorWidgetPrivate::handleFileExplorerFileRenamed(const QString& filePathFrom, const QString& filePathTo)
{
    auto relativePath = filePathFrom;
    relativePath.remove(currentControl->dir() + separator() + DIR_THIS + separator());

    auto relativePath_2 = filePathTo;
    relativePath_2.remove(currentControl->dir() + separator() + DIR_THIS + separator());

    for (auto& item : parent->_editorItems) {
        if (item.control == currentControl && item.documents.keys().contains(relativePath)) {
            if (item.currentFileRelativePath == relativePath)
                item.currentFileRelativePath = relativePath_2;

            item.documents[relativePath_2] = item.documents.value(relativePath);
            item.documents.remove(relativePath);
            break;
        }
    }

    updateOpenDocHistory();
}

void QmlCodeEditorWidgetPrivate::handleCodeEditorButtonClicked()
{
    parent->setMode(QmlCodeEditorWidget::CodeEditor);
}

void QmlCodeEditorWidgetPrivate::handleImageEditorButtonClicked()
{
    parent->setMode(QmlCodeEditorWidget::ImageEditor);
}

void QmlCodeEditorWidgetPrivate::handleHexEditorButtonClicked()
{
    parent->setMode(QmlCodeEditorWidget::HexEditor);
}

// TODO: Update when toolbox controls are changed
void QmlCodeEditorWidgetPrivate::updateOpenDocHistory()
{
    itemsCombobox->clear();
    documentsCombobox->clear();
    for (auto& item : parent->_editorItems) {

        auto itemText = item.control->id() + CHAR_SEPARATION + item.control->uid();
        for (auto path : item.documents.keys()) {
            if (item.documents.value(path).document->isModified()) {
                itemText = CHAR_CHANGEINDICATOR + itemText;
                break;
            }
        }
        itemsCombobox->addItem(itemText);

        if (item.control == currentControl) {
            saveButton->setEnabled(item.documents.value(item.currentFileRelativePath).document->isModified());
            for (auto path : item.documents.keys())
                if (item.documents.value(path).document->isModified())
                    documentsCombobox->addItem(CHAR_CHANGEINDICATOR + path);
                else
                    documentsCombobox->addItem(path);

            itemsCombobox->setCurrentText(item.control->id() + CHAR_SEPARATION + item.control->uid());
            itemsCombobox->setCurrentText(CHAR_CHANGEINDICATOR + item.control->id() + CHAR_SEPARATION + item.control->uid());
            documentsCombobox->setCurrentText(item.currentFileRelativePath);
            documentsCombobox->setCurrentText(CHAR_CHANGEINDICATOR + item.currentFileRelativePath);

            if (fileExplorer->rootPath() != (item.control->dir() + separator() + DIR_THIS))
                fileExplorer->setRootPath(item.control->dir() + separator() + DIR_THIS);
        }
    }
}

void QmlCodeEditorWidgetPrivate::handleItemsComboboxActivated(QString text)
{
    if (text.isEmpty())
        return;

    if (text.at(0) == CHAR_CHANGEINDICATOR)
        text.remove(0, 1);

    auto strList = text.split(CHAR_SEPARATION);

    if (strList.size() != 2)
        return;

    auto uid = strList[1];

    for (auto& item : parent->_editorItems) {
        if (item.control->uid() == uid) {
            parent->openControl(item.control);
            break;
        }
    }
}

void QmlCodeEditorWidgetPrivate::handleDocumentsComboboxActivated(QString text)
{
    if (text.isEmpty())
        return;

    if (text.at(0) == CHAR_CHANGEINDICATOR)
        text.remove(0, 1);

    for (auto& item : parent->_editorItems) {
        if (item.control == currentControl &&
            item.currentFileRelativePath != text &&
            item.documents.keys().contains(text)) {
            parent->setCurrentDocument(item.control, item.control->dir() +
                                       separator() + DIR_THIS + separator() + text);
            break;
        }
    }
}

void QmlCodeEditorWidgetPrivate::handleControlRemoval(Control* control)
{
    parent->closeControl(control, false);
}

void QmlCodeEditorWidgetPrivate::propertyUpdate(Control* control, const QString& property, const QString& value)
{
    for (auto& item : parent->_editorItems) {
        if (item.control == control &&
            item.documents.keys().contains("main.qml")) {
            ParserUtils::setProperty(item.documents.value("main.qml").document, control->url(), property, value);
            break;
        }
    }
}

QmlCodeEditorWidget::QmlCodeEditorWidget(QWidget* parent)
    : QWidget(parent)
    , _d(new QmlCodeEditorWidgetPrivate(this))
{
}

QmlCodeEditorWidget::Mode QmlCodeEditorWidget::mode() const
{
    return _mode;
}

int QmlCodeEditorWidget::openControlCount() const
{
    return _editorItems.size();
}

void QmlCodeEditorWidget::setMode(const Mode& mode)
{
    _mode = mode;
    emit _d->parent->modeChanged();
}

bool QmlCodeEditorWidget::pinned() const
{
    return _d->pinButton->toolTip().contains("Unpin");
}

bool QmlCodeEditorWidget::isOpen(Control* control) const
{
    for (auto item : _editorItems)
        if (item.control == control)
            return true;
    return false;
}

bool QmlCodeEditorWidget::isOpen(const QString& controlPath) const
{
    for (auto item : _editorItems)
        if (item.control->dir().contains(controlPath))
            return true;
    return false;
}

bool QmlCodeEditorWidget::hasUnsavedDocs() const
{
    for (auto& item : _editorItems)
        if (hasChanges(item.control))
            return true;

    return false;
}

bool QmlCodeEditorWidget::hasChanges(Control* control) const
{
    if (!isOpen(control))
        return false;

    for (auto& item : _editorItems) {
        if (item.control == control) {
            for (auto& doc : item.documents.keys()) {
                if (item.documents.value(doc).document->isModified()) {
                    return true;
                }
            }
        }
    }

    return false;
}

void QmlCodeEditorWidget::addControl(Control* control)
{
    for (auto& item : _editorItems)
        if (item.control == control)
            return;

    auto relativePath = control->url();
    relativePath.remove(control->dir() + separator() + DIR_THIS + separator());

    EditorItem item;
    item.control = control;
    item.currentFileRelativePath = relativePath;
    item.documents[relativePath].document = new QmlCodeDocument(_d->codeEditor);
    item.documents[relativePath].document->setFilePath(control->url());
    item.documents[relativePath].document->setPlainText(rdfile(control->url()));
    item.documents[relativePath].document->setModified(false);
    _editorItems.append(item);
    _d->itemsCombobox->addItem(control->id() + CHAR_SEPARATION + control->uid());
    _d->documentsCombobox->addItem(relativePath);
}

void QmlCodeEditorWidget::addDocument(Control* control, const QString& documentPath)
{
    for (auto& item : _editorItems) {
        if (item.control == control) {
            auto relativePath = documentPath;
            relativePath.remove(control->dir() + separator() + DIR_THIS + separator());
            if (item.documents.keys().contains(relativePath))
                return;
            item.documents[relativePath].document = new QmlCodeDocument(_d->codeEditor);
            item.documents[relativePath].document->setFilePath(documentPath);
            item.documents[relativePath].document->setPlainText(rdfile(documentPath));
            item.documents[relativePath].document->setModified(false);
            _d->documentsCombobox->addItem(relativePath);
            break;
        }
    }
}

void QmlCodeEditorWidget::setCurrentDocument(Control* control, const QString& documentPath)
{
    for (auto& item : _editorItems) {
        if (item.control == control) {
            auto relativePath = documentPath;
            relativePath.remove(control->dir() + separator() + DIR_THIS + separator());
            if (!item.documents.keys().contains(relativePath))
                return;
            item.currentFileRelativePath = relativePath;
            if (_d->currentControl == control)
                openControl(control);
            break;
        }
    }
}

void QmlCodeEditorWidget::setCurrentLine(int lineNumber)
{
    _d->codeEditor->gotoLine(lineNumber);
}

void QmlCodeEditorWidget::openControl(Control* control)
{
    for (auto& item : _editorItems) {
        if (item.control == control) {

            for (auto& item : _editorItems) {
                if (item.control == _d->currentControl) {
                    QString previousRelativePath;
                    for (auto key : item.documents.keys())
                        if (item.documents.value(key).document == _d->codeEditor->document())
                            previousRelativePath = key;
                    if (!previousRelativePath.isEmpty())
                        item.documents[previousRelativePath].cursor = _d->codeEditor->textCursor();
                    for (auto key : item.documents.keys())
                        disconnect(item.documents.value(key).modificationConnection);
                    break;
                }
            }

            _d->currentControl = item.control;
            _d->codeEditor->setCodeDocument(item.documents.value(item.currentFileRelativePath).document);
//            _d->codeEditor->updateCompletion();
            _d->updateOpenDocHistory();

//            _d->codeEditor->document()->setDefaultFont(_d->defaultFont);
//            QFontMetrics metrics(_d->defaultFont);
//            _d->codeEditor->setTabStopWidth(metrics.width(TAB_SPACE)); BUG


            disconnect(_d->previousUndoConnection);
            disconnect(_d->previousRedoConnection);
            _d->previousUndoConnection = connect(_d->codeEditor->document(), SIGNAL(undoAvailable(bool)),
                                                 _d->undoButton, SLOT(setEnabled(bool)));
            _d->previousRedoConnection = connect(_d->codeEditor->document(), SIGNAL(redoAvailable(bool)),
                                                 _d->redoButton, SLOT(setEnabled(bool)));
            _d->undoButton->setEnabled(_d->codeEditor->document()->isUndoAvailable());
            _d->redoButton->setEnabled(_d->codeEditor->document()->isRedoAvailable());

            for (auto key : item.documents.keys())
                item.documents[key].modificationConnection =
                connect(item.documents.value(key).document,
                SIGNAL(modificationChanged(bool)), _d, SLOT(updateOpenDocHistory()));

            if (!item.documents.value(item.currentFileRelativePath).cursor.isNull())
                _d->codeEditor->setTextCursor(item.documents.value(item.currentFileRelativePath).cursor);

            _d->codeEditor->show();
            _d->noDocumentIndicator->hide();
            _d->documentsCombobox->show();
            _d->itemsCombobox->show();
            _d->closeButton->show();

            break;
        }
    }

    emit openControlCountChanged();
}

void QmlCodeEditorWidget::closeControl(Control* control, const bool ask)
{
    for (auto& item : _editorItems) {
        if (item.control == control)  {
            if (ask) {
                QMessageBox msgBox;
                msgBox.setText(QString("The control %1 has been modified in Qml Editor.").arg(control->id()));
                msgBox.setInformativeText("Do you want to save all your changes for this control?");
                msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Save);
                int ret = msgBox.exec();
                switch (ret) {
                    case QMessageBox::Cancel:
                        return;
                        break;

                    case QMessageBox::Save:
                        saveControl(control);
                        break;

                    case QMessageBox::Discard:
                    default:
                        break;
                }
            }

            for (auto path : item.documents.keys())
                closeDocument(control, control->dir() + separator() + DIR_THIS + separator() + path, false);
            break;
        }
    }
}

void QmlCodeEditorWidget::closeDocument(Control* control, const QString& documentPath, const bool ask)
{
    auto relativePath = documentPath;
    relativePath.remove(control->dir() + separator() + DIR_THIS + separator());

    EditorItem* issuerItem = nullptr;
    for (auto& item : _editorItems) {
        if (item.control == control) {
            issuerItem = &item;
            break;
        }
    }

    if (!issuerItem)
        return;

    if (!issuerItem->documents.keys().contains(relativePath))
        return;

    if (ask && issuerItem->documents.value(relativePath).document->isModified()) {
        QMessageBox msgBox;
        msgBox.setText(QString("The document %1 has been modified.").arg(relativePath));
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        switch (ret) {
            case QMessageBox::Cancel:
                return;
                break;

            case QMessageBox::Save:
                saveDocument(control, documentPath);
                break;

            case QMessageBox::Discard:
            default:
                break;
        }
    }

    if (_d->currentControl == issuerItem->control &&
        issuerItem->currentFileRelativePath == relativePath) {
        _d->codeEditor->setCodeDocument(_d->emptyDoc);
        _d->codeEditor->hide();
        _d->noDocumentIndicator->show();
        _d->documentsCombobox->hide();
        _d->itemsCombobox->hide();
        _d->closeButton->hide();
    }

    issuerItem->documents.value(relativePath).document->deleteLater();
    issuerItem->documents.remove(relativePath);

    if (!issuerItem->documents.isEmpty() &&
        issuerItem->currentFileRelativePath == relativePath) {
        issuerItem->currentFileRelativePath = issuerItem->documents.keys().at(0);
    }

    if (issuerItem->documents.isEmpty()) {
        if (_d->currentControl == issuerItem->control)
            _d->currentControl = nullptr;
        _editorItems.removeOne(*issuerItem);
    }

    if (!_d->currentControl)
        _d->fileExplorer->setRootPath(UNKNOWN_PATH);

    if (!_editorItems.isEmpty()) {
        if(_d->currentControl)
            openControl(_d->currentControl);
        else
            openControl(_editorItems[0].control);
    }

    _d->updateOpenDocHistory();
    emit openControlCountChanged();
}

void QmlCodeEditorWidget::saveControl(Control*)
{
    //TODO
}

void QmlCodeEditorWidget::saveDocument(Control* control, const QString& documentPath)
{
    auto relativePath = documentPath;
    relativePath.remove(control->dir() + separator() + DIR_THIS + separator());

    for (auto& item : _editorItems) {
        if (item.control == control &&
            item.documents.keys().contains(relativePath)) {
            wrfile(documentPath, item.documents.value
                   (relativePath).document->toPlainText().toUtf8());
            item.documents.value(relativePath).document->setModified(false);

            Q_ASSERT(!control->id().isEmpty());

            const QString& id = ParserUtils::property(documentPath, "id");

            if (control->id() != id) {
                const QString& previousId = control->id();

                if (id.isEmpty())
                    ControlPropertyManager::setId(control, "control", true, false);
                else
                    ControlPropertyManager::setId(control, id, true, false); // For refactorId

                WindowManager::mainWindow()->inspectorPane()->handleControlIdChange(control, previousId);
            }

            ControlPreviewingManager::scheduleControlCodeUpdate(control->uid());
            break;
        }
    }

    _d->updateOpenDocHistory();
    emit documentSaved();
}

QmlCodeEditor* QmlCodeEditorWidget::editor() const
{
    return _d->codeEditor;
}

void QmlCodeEditorWidget::sweep()
{
    for (auto& item : _editorItems)
        closeControl(item.control, false);

    _d->currentControl = nullptr;

    if (!pinned())
        _d->handlePinButtonClicked();

    _d->redoButton->setDisabled(true);
    _d->copyButton->setDisabled(true);
    _d->cutButton->setDisabled(true);
    _d->saveButton->setDisabled(true);

    _d->lastWidthOfExplorerWrapper = INITIALWIDTH_FILEEXPLORER;
//    _d->codeEditor->sweep();
    _d->fileExplorer->sweep();
    // TODO: _d->imageEditor->sweep();
    // TODO: _d->hexEditor->sweep();

    if (_d->hideShowButton->toolTip().contains("Hide"))
        _d->handleHideShowButtonClicked();

    _d->handleZoomLevelChange("100 %");
    setMode(CodeEditor);
}

void QmlCodeEditorWidget::saveAll()
{
    for (auto& item : _editorItems)
        saveControl(item.control);
}

void QmlCodeEditorWidget::raiseContainer()
{
    _d->containerWidget->raise();
}

void QmlCodeEditorWidget::refreshErrors()
{
    // FIXME
//    if (_d->currentControl) {
//        _d->codeEditor->clearErrorLines();
//        for (auto error : _d->currentControl->errors())
//            _d->codeEditor->addErrorLine(error.line());
//    }
}

void QmlCodeEditorWidget::handleControlRemoval(Control* control)
{
    closeControl(control, false);
}

QSize QmlCodeEditorWidget::sizeHint() const
{
    return QSize(680, 680);
}

void QmlCodeEditorWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen("#cbced1");
    painter.drawLine(QRectF(rect()).topLeft() + QPointF(0.5, 0.5),
                     QRectF(rect()).bottomLeft() + QPointF(0.5, -0.5));
}

#include "qmlcodeeditorwidget.moc"
