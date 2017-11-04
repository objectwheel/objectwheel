#include <qmleditorview.h>
#include <formscene.h>
#include <control.h>
#include <fit.h>
#include <savemanager.h>
#include <designmanager.h>
#include <filemanager.h>
#include <qmlcodeeditor.h>
#include <css.h>
#include <fileexplorer.h>
#include <qmlhighlighter.h>
#include <control.h>

#include <QDebug>
#include <QVBoxLayout>
#include <QPainter>
#include <QToolBar>
#include <QAction>
#include <QToolButton>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSplitter>
#include <QMessageBox>
#include <QScrollBar>

#define LINE_COLOR ("#606467")
#define CHAR_SEPARATION ("::")
#define CHAR_CHANGEINDICATOR ("*")
#define INITIALWIDTH_FILEEXPLORER (fit(450))
#define MINWIDTH_FILEEXPLORER (fit(200))
#define MINWIDTH_EDITOR (fit(200))
#define UNKNOWN_PATH ("67asdta8d9yaghqbj4")
#define TAB_SPACE ("    ")

using namespace Fit;

class QmlEditorViewPrivate : public QObject
{
        Q_OBJECT

    public:
        QmlEditorViewPrivate(QmlEditorView* parent);

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

    public:
        QmlEditorView* parent;
        QVBoxLayout* vBoxLayout;
        QWidget* containerWidget;
        QVBoxLayout* containerVBoxLayout;

        Control* currentControl;
        int lastWidthOfExplorerWrapper;
        QFont defaultFont;
        QMetaObject::Connection previousUndoConnection;
        QMetaObject::Connection previousRedoConnection;
        QAction saveAction;

        QToolBar* toolbar;
        QToolButton* pinButton;
        QToolButton* undoButton;
        QToolButton* redoButton;
        QToolButton* closeButton;
        QToolButton* saveButton;
        QToolButton* cutButton;
        QToolButton* copyButton;
        QToolButton* pasteButton;
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
        QToolBar* toolbar_2;
        QToolButton* hideShowButton;
        QToolButton* codeEditorButton;
        QToolButton* imageEditorButton;
        QToolButton* hexEditorButton;
        FileExplorer* fileExplorer;
};

QmlEditorViewPrivate::QmlEditorViewPrivate(QmlEditorView* parent)
    : QObject(parent)
    , parent(parent)
    , vBoxLayout(new QVBoxLayout(parent))
    , containerWidget(new QWidget)
    , containerVBoxLayout(new QVBoxLayout(containerWidget))
    , currentControl(nullptr)
    , lastWidthOfExplorerWrapper(INITIALWIDTH_FILEEXPLORER)
    , toolbar(new QToolBar)
    , pinButton(new QToolButton)
    , undoButton(new QToolButton)
    , redoButton(new QToolButton)
    , closeButton(new QToolButton)
    , saveButton(new QToolButton)
    , cutButton(new QToolButton)
    , copyButton(new QToolButton)
    , pasteButton(new QToolButton)
    , itemsCombobox(new QComboBox)
    , documentsCombobox(new QComboBox)
    , zoomlLevelCombobox(new QComboBox)
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
    , toolbar_2(new QToolBar)
    , hideShowButton(new QToolButton)
    , codeEditorButton(new QToolButton)
    , imageEditorButton(new QToolButton)
    , hexEditorButton(new QToolButton)
    , fileExplorer(new FileExplorer)
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
    saveAction.setShortcut(QKeySequence::Save);

    splitter->setStyleSheet("QSplitter{background: #e0e4e7;}");
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

    itemsCombobox->setFixedWidth(fit(200));
    documentsCombobox->setFixedWidth(fit(200));

    redoButton->setDisabled(true);
    copyButton->setDisabled(true);
    cutButton->setDisabled(true);
    saveButton->setDisabled(true);

    codeEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    codeEditor->horizontalScrollBar()->setStyleSheet(CSS::ScrollBarH);
    codeEditor->verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    codeEditor->setDocument(nullptr);
    codeEditor->hide();

    noDocumentIndicator->setStyleSheet("QLabel {"
                                      "color: #606467;"
                                      "}");
    noDocumentIndicator->setText("No documents open");
    noDocumentIndicator->setAlignment(Qt::AlignCenter);
    noDocumentIndicator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    noDocumentIndicator->show();

    defaultFont.setFamily("Liberation Mono");
    defaultFont.setStyleHint(QFont::Monospace);

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

    //TODO: form or control removal will be handled

    QTimer::singleShot(1000, [=] {
        connect(SaveManager::instance(), SIGNAL(databaseChanged()), SLOT(updateOpenDocHistory()));
        connect(DesignManager::controlScene(), SIGNAL(aboutToRemove(Control*)), SLOT(handleControlRemoval(Control*)));
        connect(DesignManager::formScene(), SIGNAL(aboutToRemove(Control*)), SLOT(handleControlRemoval(Control*)));
    });

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

    pinButton->setIcon(QIcon(":/resources/images/unpin.png"));
    undoButton->setIcon(QIcon(":/resources/images/undo.png"));
    redoButton->setIcon(QIcon(":/resources/images/redo.png"));
    closeButton->setIcon(QIcon(":/resources/images/delete-icon.png"));
    saveButton->setIcon(QIcon(":/resources/images/save.png"));
    cutButton->setIcon(QIcon(":/resources/images/cut.png"));
    copyButton->setIcon(QIcon(":/resources/images/copy.png"));
    pasteButton->setIcon(QIcon(":/resources/images/paste.png"));

    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    toolbar->setStyleSheet(CSS::DesignerToolbar);
    toolbar->setIconSize(QSize(fit(14), fit(14)));
    toolbar->setFixedHeight(fit(21));
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
    toolbar->addWidget(spacer);
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

    hideShowButton->setIcon(QIcon(":/resources/images/show.png"));
    codeEditorButton->setIcon(QIcon(":/resources/images/code.png"));
    imageEditorButton->setIcon(QIcon(":/resources/images/image.png"));
    hexEditorButton->setIcon(QIcon(":/resources/images/hex.png"));

    toolbar_2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    toolbar_2->setOrientation(Qt::Vertical);
    toolbar_2->setStyleSheet(CSS::DesignerToolbarV);
    toolbar_2->setIconSize(QSize(fit(14), fit(14)));
    toolbar_2->setFixedWidth(fit(21));
    toolbar_2->addWidget(hideShowButton);
    toolbar_2->addSeparator();
    toolbar_2->addWidget(codeEditorButton);
    toolbar_2->addWidget(imageEditorButton);
    toolbar_2->addWidget(hexEditorButton);

    splitter->handle(1)->setDisabled(true);
    editorWrapper->setMinimumWidth(MINWIDTH_FILEEXPLORER);
    explorerWrapper->setFixedWidth(toolbar_2->width());
    connect(splitter, &QSplitter::splitterMoved, this, [=] {
        lastWidthOfExplorerWrapper = explorerWrapper->width();
    });
}

qreal QmlEditorViewPrivate::findPixelSize(const QString& text)
{
    qreal base = QFont().pixelSize();

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

void QmlEditorViewPrivate::handleCursorPositionChanged()
{
    auto textCursor = codeEditor->textCursor();
    QString lineColText("# Line: %1, Col: %2");
    lineColLabel->setText(lineColText.arg(textCursor.blockNumber() + 1).arg(textCursor.columnNumber()));
}

void QmlEditorViewPrivate::handlePinButtonClicked()
{
    if (pinButton->toolTip().contains("Unpin")) {
        pinButton->setToolTip("Pin Editor.");
        pinButton->setIcon(QIcon(":/resources/images/pin.png"));
        containerWidget->setParent(nullptr);
        containerWidget->show();
        containerWidget->setWindowIcon(QIcon(":/resources/images/owicon.png"));
    } else {
        pinButton->setToolTip("Unpin Editor.");
        pinButton->setIcon(QIcon(":/resources/images/unpin.png"));
        vBoxLayout->addWidget(containerWidget);
    }
}

void QmlEditorViewPrivate::handleCloseButtonClicked()
{
    for (auto& item : parent->_editorItems) {
        if (item.control == currentControl) {
            parent->closeDocument(item.control, item.control->dir() + separator() +
                                  DIR_THIS + separator() + item.currentFileRelativePath);
            break;
        }
    }
}

void QmlEditorViewPrivate::handleSaveButtonClicked()
{
    for (auto& item : parent->_editorItems) {
        if (item.control == currentControl) {
            parent->saveDocument(item.control, item.control->dir() + separator() +
                                  DIR_THIS + separator() + item.currentFileRelativePath);
            break;
        }
    }
}

void QmlEditorViewPrivate::handleZoomLevelChange(const QString& text)
{
    defaultFont.setPixelSize(findPixelSize(text));
    codeEditor->document()->setDefaultFont(defaultFont);
}

void QmlEditorViewPrivate::handleHideShowButtonClicked()
{
    if (hideShowButton->toolTip().contains("Hide")) {
        hideShowButton->setIcon(QIcon(":/resources/images/show.png"));
        hideShowButton->setToolTip("Show File Explorer.");
        splitter->handle(1)->setDisabled(true);
        fileExplorer->hide();
        explorerWrapper->setFixedWidth(toolbar_2->width());
        QList<int> sizes;
        sizes << containerWidget->width() - toolbar_2->width();
        sizes << toolbar_2->width();
        splitter->setSizes(sizes);
    } else {
        hideShowButton->setIcon(QIcon(":/resources/images/hide.png"));
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

void QmlEditorViewPrivate::handleModeChange()
{
    if (parent->_mode == QmlEditorView::CodeEditor) {
        codeEditorButton->setChecked(true);
        codeEditorButton->setDisabled(true);
        imageEditorButton->setChecked(false);
        hexEditorButton->setChecked(false);
        imageEditorButton->setEnabled(true);
        hexEditorButton->setEnabled(true);

        imageEditor->hide();
        hexEditor->hide();
        codeEditor->show();
    } else if (parent->_mode == QmlEditorView::ImageEditor) {
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

void QmlEditorViewPrivate::handleFileExplorerFileOpen(const QString& filePath)
{
    parent->addDocument(currentControl, filePath);
    parent->setCurrentDocument(currentControl, filePath);
}

void QmlEditorViewPrivate::handleFileExplorerFileDeleted(const QString& filePath)
{
    parent->closeDocument(currentControl, filePath, false);
}

void QmlEditorViewPrivate::handleFileExplorerFileRenamed(const QString& filePathFrom, const QString& filePathTo)
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

void QmlEditorViewPrivate::handleCodeEditorButtonClicked()
{
    parent->setMode(QmlEditorView::CodeEditor);
}

void QmlEditorViewPrivate::handleImageEditorButtonClicked()
{
    parent->setMode(QmlEditorView::ImageEditor);
}

void QmlEditorViewPrivate::handleHexEditorButtonClicked()
{
    parent->setMode(QmlEditorView::HexEditor);
}

// TODO: Update when toolbox controls are changed
void QmlEditorViewPrivate::updateOpenDocHistory()
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

void QmlEditorViewPrivate::handleItemsComboboxActivated(QString text)
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

void QmlEditorViewPrivate::handleDocumentsComboboxActivated(QString text)
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

void QmlEditorViewPrivate::handleControlRemoval(Control* control)
{
    parent->closeControl(control, false);
}

QmlEditorView::QmlEditorView(QWidget* parent)
    : QWidget(parent)
    , _d(new QmlEditorViewPrivate(this))
{
}

void QmlEditorView::paintEvent(QPaintEvent*)
{
    QPen pen;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRectF _rect(0, 0, fit(150), fit(60));
    _rect.moveCenter(rect().center());
    pen.setStyle(Qt::DotLine);
    pen.setColor(LINE_COLOR);
    painter.setPen(pen);
    painter.drawRect(_rect);
    painter.drawText(_rect, "Editor unpinned, pin it again.", QTextOption(Qt::AlignCenter));
}

QmlEditorView::Mode QmlEditorView::mode() const
{
    return _mode;
}

void QmlEditorView::setMode(const Mode& mode)
{
    _mode = mode;
    emit _d->parent->modeChanged();
}

bool QmlEditorView::pinned() const
{
    return _d->pinButton->toolTip().contains("Unpin");
}

void QmlEditorView::addControl(Control* control)
{
    for (auto& item : _editorItems)
        if (item.control == control)
            return;

    auto relativePath = control->url();
    relativePath.remove(control->dir() + separator() + DIR_THIS + separator());

    EditorItem item;
    item.control = control;
    item.currentFileRelativePath = relativePath;
    item.documents[relativePath].document = new QTextDocument(this);
    item.documents[relativePath].document->setDocumentLayout(new QPlainTextDocumentLayout(item.documents[relativePath].document));
    item.documents[relativePath].document->setPlainText(rdfile(control->url()));
    item.documents[relativePath].document->setModified(false);
    new QmlHighlighter(item.documents.value(relativePath).document);
    _editorItems.append(item);
    _d->itemsCombobox->addItem(control->id() + CHAR_SEPARATION + control->uid());
    _d->documentsCombobox->addItem(relativePath);
}

void QmlEditorView::addDocument(Control* control, const QString& documentPath)
{
    for (auto& item : _editorItems) {
        if (item.control == control) {
            auto relativePath = documentPath;
            relativePath.remove(control->dir() + separator() + DIR_THIS + separator());
            if (item.documents.keys().contains(relativePath))
                return;
            item.documents[relativePath].document = new QTextDocument(this);
            item.documents[relativePath].document->setDocumentLayout(new QPlainTextDocumentLayout(item.documents.value(relativePath).document));
            item.documents[relativePath].document->setPlainText(rdfile(documentPath));
            item.documents[relativePath].document->setModified(false);
            new QmlHighlighter(item.documents.value(relativePath).document);
            _d->documentsCombobox->addItem(relativePath);
            break;
        }
    }
}

void QmlEditorView::setCurrentDocument(Control* control, const QString& documentPath)
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

void QmlEditorView::openControl(Control* control)
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
            _d->codeEditor->setDocument(item.documents.value(item.currentFileRelativePath).document);
            _d->codeEditor->updateCompletion();
            _d->updateOpenDocHistory();

            _d->codeEditor->document()->setDefaultFont(_d->defaultFont);
            QFontMetrics metrics(_d->defaultFont);
            _d->codeEditor->setTabStopWidth(metrics.width(TAB_SPACE));


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

            if (_d->noDocumentIndicator->isVisible()) {
                _d->codeEditor->show();
                _d->noDocumentIndicator->hide();
            }
            break;
        }
    }
}

void QmlEditorView::closeControl(Control* control, const bool ask)
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

void QmlEditorView::closeDocument(Control* control, const QString& documentPath, const bool ask)
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
        _d->codeEditor->setDocument(nullptr);
        _d->codeEditor->hide();
        _d->noDocumentIndicator->show();
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
}

void QmlEditorView::saveControl(Control*)
{
    //TODO
}

void QmlEditorView::saveDocument(Control* control, const QString& documentPath)
{
    auto relativePath = documentPath;
    relativePath.remove(control->dir() + separator() + DIR_THIS + separator());

    for (auto& item : _editorItems) {
        if (item.control == control &&
            item.documents.keys().contains(relativePath)) {
            wrfile(documentPath, item.documents.value
                   (relativePath).document->toPlainText().toUtf8());
            item.documents.value(relativePath).document->setModified(false);
            control->refresh();
            break;
        }
    }

    _d->updateOpenDocHistory();
}

void QmlEditorView::raiseContainer()
{
    _d->containerWidget->raise();
}

void QmlEditorView::refreshErrors()
{
    if (_d->currentControl) {
        _d->codeEditor->clearErrorLines();
        for (auto error : _d->currentControl->errors())
            _d->codeEditor->addErrorLine(error.line());
    }
}

#include "qmleditorview.moc"
