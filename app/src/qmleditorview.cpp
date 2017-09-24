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

#define LINE_COLOR ("#606467")
#define INITIALWIDTH_FILEEXPLORER (fit(450))
#define MINWIDTH_FILEEXPLORER (fit(200))
#define MINWIDTH_EDITOR (fit(200))

using namespace Fit;

class QmlEditorViewPrivate : public QObject
{
        Q_OBJECT

    public:
        QmlEditorViewPrivate(QmlEditorView* parent);

    private:
        int findRatio(const QString& text);

    public slots:
        void handleCursorPositionChanged();
        void handleModeChange();
        void handleFileExplorerFileOpen(const QString& filePath);
        void handlePinButtonClicked();
        void handleZoomLevelChange(const QString& text);
        void handleHideShowButtonClicked();
        void handleCodeEditorButtonClicked();
        void handleImageEditorButtonClicked();
        void handleHexEditorButtonClicked();
        void handleDatabaseChange();
        void handleItemsComboboxActivated(const QString& text);
        void handleDocumentsComboboxActivated(const QString& text);


    public:
        QmlEditorView* parent;
        QVBoxLayout vBoxLayout;
        QWidget containerWidget;
        QVBoxLayout containerVBoxLayout;

        QToolBar toolbar;
        QToolButton pinButton;
        QToolButton undoButton;
        QToolButton redoButton;
        QToolButton backButton;
        QToolButton forthButton;
        QToolButton closeButton;
        QToolButton saveButton;
        QToolButton cutButton;
        QToolButton copyButton;
        QToolButton pasteButton;
        QComboBox itemsCombobox;
        QComboBox documentsCombobox;
        QComboBox zoomlLevelCombobox;
        QLabel lineColLabel;
        int previousRatio;
        Control* currentControl;

        QSplitter splitter;
        QWidget editorWrapper;
        QVBoxLayout editorWrapperVBoxLayout;
        QmlCodeEditor codeEditor;
        QWidget imageEditor;
        QWidget hexEditor;
        QWidget explorerWrapper;
        QHBoxLayout explorerWrapperHBoxLayout;
        QToolBar toolbar_2;
        QToolButton hideShowButton;
        QToolButton codeEditorButton;
        QToolButton imageEditorButton;
        QToolButton hexEditorButton;
        FileExplorer fileExplorer;
        int lastWidthOfExplorerWrapper;
};

QmlEditorViewPrivate::QmlEditorViewPrivate(QmlEditorView* parent)
    : QObject(parent)
    , parent(parent)
    , vBoxLayout(parent)
    , containerVBoxLayout(&containerWidget)
    , previousRatio(0)
    , editorWrapperVBoxLayout(&editorWrapper)
    , explorerWrapperHBoxLayout(&explorerWrapper)
    , lastWidthOfExplorerWrapper(INITIALWIDTH_FILEEXPLORER)
{
    vBoxLayout.setContentsMargins(0, 0, 0, 0);
    vBoxLayout.setSpacing(0);
    vBoxLayout.addWidget(&containerWidget);

    containerVBoxLayout.setContentsMargins(0, 0, 0, 0);
    containerVBoxLayout.setSpacing(0);
    containerVBoxLayout.addWidget(&toolbar);
    containerVBoxLayout.addWidget(&splitter);

    splitter.setStyleSheet("QSplitter{background: #e0e4e7;}");
    splitter.addWidget(&editorWrapper);
    splitter.addWidget(&explorerWrapper);
    splitter.setCollapsible(0, false);;
    splitter.setCollapsible(1, false);;

    editorWrapperVBoxLayout.setContentsMargins(0, 0, 0, 0);
    editorWrapperVBoxLayout.setSpacing(0);
    editorWrapperVBoxLayout.addWidget(&codeEditor);
    editorWrapperVBoxLayout.addWidget(&imageEditor);
    editorWrapperVBoxLayout.addWidget(&hexEditor);

    explorerWrapperHBoxLayout.setContentsMargins(0, 0, 0, 0);
    explorerWrapperHBoxLayout.setSpacing(0);
    explorerWrapperHBoxLayout.addWidget(&toolbar_2);
    explorerWrapperHBoxLayout.addWidget(&fileExplorer);

    fileExplorer.hide();
    fileExplorer.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    fileExplorer.setRootPath("67asdta8d9yaghqbj4");

    containerWidget.setWindowTitle("Objectwheel Qml Editor");

    itemsCombobox.setFixedWidth(fit(200));
    documentsCombobox.setFixedWidth(fit(200));

    redoButton.setDisabled(true);
    copyButton.setDisabled(true);
    cutButton.setDisabled(true);

    codeEditor.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(parent, SIGNAL(modeChanged()), SLOT(handleModeChange()));
    connect(&codeEditor, SIGNAL(cursorPositionChanged()), SLOT(handleCursorPositionChanged()));
    connect(&pinButton, SIGNAL(clicked(bool)), SLOT(handlePinButtonClicked()));
    connect(&undoButton, SIGNAL(clicked(bool)), &codeEditor, SLOT(undo()));
    connect(&redoButton, SIGNAL(clicked(bool)), &codeEditor, SLOT(redo()));
    connect(&codeEditor, SIGNAL(undoAvailable(bool)), &undoButton, SLOT(setEnabled(bool)));
    connect(&codeEditor, SIGNAL(redoAvailable(bool)), &redoButton, SLOT(setEnabled(bool)));
    connect(&copyButton, SIGNAL(clicked(bool)), &codeEditor, SLOT(copy()));
    connect(&cutButton, SIGNAL(clicked(bool)), &codeEditor, SLOT(cut()));
    connect(&pasteButton, SIGNAL(clicked(bool)), &codeEditor, SLOT(paste()));
    connect(&codeEditor, SIGNAL(copyAvailable(bool)), &copyButton, SLOT(setEnabled(bool)));
    connect(&codeEditor, SIGNAL(copyAvailable(bool)), &cutButton, SLOT(setEnabled(bool)));
    connect(&zoomlLevelCombobox, SIGNAL(currentTextChanged(QString)), SLOT(handleZoomLevelChange(QString)));
    connect(&hideShowButton, SIGNAL(clicked(bool)), SLOT(handleHideShowButtonClicked()));
    connect(&codeEditorButton, SIGNAL(clicked(bool)), SLOT(handleCodeEditorButtonClicked()));
    connect(&imageEditorButton, SIGNAL(clicked(bool)), SLOT(handleImageEditorButtonClicked()));
    connect(&hexEditorButton, SIGNAL(clicked(bool)), SLOT(handleHexEditorButtonClicked()));
    connect(&fileExplorer, SIGNAL(fileOpened(QString)), SLOT(handleFileExplorerFileOpen(QString)));
    connect(&itemsCombobox, SIGNAL(activated(QString)), SLOT(handleItemsComboboxActivated(QString)));
    connect(&documentsCombobox, SIGNAL(activated(QString)), SLOT(handleDocumentsComboboxActivated(QString)), Qt::QueuedConnection);

    QTimer::singleShot(1000, [=] {
        connect(SaveManager::instance(), SIGNAL(databaseChanged()), SLOT(handleDatabaseChange()));
    });

    zoomlLevelCombobox.addItem("25 %");
    zoomlLevelCombobox.addItem("50 %");
    zoomlLevelCombobox.addItem("75 %");
    zoomlLevelCombobox.addItem("100 %");
    zoomlLevelCombobox.addItem("125 %");
    zoomlLevelCombobox.addItem("150 %");
    zoomlLevelCombobox.addItem("175 %");
    zoomlLevelCombobox.addItem("200 %");
    zoomlLevelCombobox.addItem("300 %");
    zoomlLevelCombobox.addItem("500 %");
    zoomlLevelCombobox.addItem("900 %");
    zoomlLevelCombobox.setCurrentIndex(3);

    pinButton.setCursor(Qt::PointingHandCursor);
    backButton.setCursor(Qt::PointingHandCursor);
    forthButton.setCursor(Qt::PointingHandCursor);
    undoButton.setCursor(Qt::PointingHandCursor);
    redoButton.setCursor(Qt::PointingHandCursor);
    closeButton.setCursor(Qt::PointingHandCursor);
    zoomlLevelCombobox.setCursor(Qt::PointingHandCursor);
    documentsCombobox.setCursor(Qt::PointingHandCursor);
    itemsCombobox.setCursor(Qt::PointingHandCursor);
    saveButton.setCursor(Qt::PointingHandCursor);
    cutButton.setCursor(Qt::PointingHandCursor);
    copyButton.setCursor(Qt::PointingHandCursor);
    pasteButton.setCursor(Qt::PointingHandCursor);

    pinButton.setToolTip("Unpin Editor.");
    backButton.setToolTip("Go cursor back.");
    forthButton.setToolTip("Go cursor forth.");
    undoButton.setToolTip("Undo action.");
    redoButton.setToolTip("Redo action.");
    closeButton.setToolTip("Close document.");
    zoomlLevelCombobox.setToolTip("Change zoom level.");
    documentsCombobox.setToolTip("Change document.");
    itemsCombobox.setToolTip("Change control.");
    lineColLabel.setToolTip("Cursor line/column indicator.");
    saveButton.setToolTip("Save document.");
    cutButton.setToolTip("Cut selection.");
    copyButton.setToolTip("Copy selection.");
    pasteButton.setToolTip("Paste from clipboard.");

    pinButton.setIcon(QIcon(":/resources/images/unpin.png"));
    backButton.setIcon(QIcon(":/resources/images/dback.png"));
    forthButton.setIcon(QIcon(":/resources/images/dforth.png"));
    undoButton.setIcon(QIcon(":/resources/images/undo.png"));
    redoButton.setIcon(QIcon(":/resources/images/redo.png"));
    closeButton.setIcon(QIcon(":/resources/images/delete-icon.png"));
    saveButton.setIcon(QIcon(":/resources/images/save.png"));
    cutButton.setIcon(QIcon(":/resources/images/cut.png"));
    copyButton.setIcon(QIcon(":/resources/images/copy.png"));
    pasteButton.setIcon(QIcon(":/resources/images/paste.png"));

    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    toolbar.setStyleSheet(CSS::DesignerToolbar);
    toolbar.setIconSize(QSize(fit(14), fit(14)));
    toolbar.setFixedHeight(fit(26));
    toolbar.addWidget(&pinButton);
    toolbar.addSeparator();
    toolbar.addWidget(&undoButton);
    toolbar.addWidget(&redoButton);
    toolbar.addSeparator();
    toolbar.addWidget(&backButton);
    toolbar.addWidget(&forthButton);
    toolbar.addSeparator();
    toolbar.addWidget(&cutButton);
    toolbar.addWidget(&copyButton);
    toolbar.addWidget(&pasteButton);
    toolbar.addWidget(&saveButton);
    toolbar.addSeparator();
    toolbar.addWidget(&closeButton);
    toolbar.addWidget(&itemsCombobox);
    toolbar.addWidget(&documentsCombobox);
    toolbar.addWidget(&zoomlLevelCombobox);
    toolbar.addWidget(spacer);
    toolbar.addWidget(&lineColLabel);

    // Toolbar_2 assets
    codeEditorButton.setCheckable(true);
    imageEditorButton.setCheckable(true);
    hexEditorButton.setCheckable(true);
    codeEditorButton.setChecked(true);
    codeEditorButton.setDisabled(true);

    hideShowButton.setCursor(Qt::PointingHandCursor);
    codeEditorButton.setCursor(Qt::PointingHandCursor);
    imageEditorButton.setCursor(Qt::PointingHandCursor);
    hexEditorButton.setCursor(Qt::PointingHandCursor);

    hideShowButton.setToolTip("Show File Explorer.");
    codeEditorButton.setToolTip("Open Text Editor.");
    imageEditorButton.setToolTip("Open Image Editor.");
    hexEditorButton.setToolTip("Open Hex Editor.");

    hideShowButton.setIcon(QIcon(":/resources/images/show.png"));
    codeEditorButton.setIcon(QIcon(":/resources/images/code.png"));
    imageEditorButton.setIcon(QIcon(":/resources/images/image.png"));
    hexEditorButton.setIcon(QIcon(":/resources/images/hex.png"));

    toolbar_2.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    toolbar_2.setOrientation(Qt::Vertical);
    toolbar_2.setStyleSheet(CSS::DesignerToolbarV);
    toolbar_2.setIconSize(QSize(fit(14), fit(14)));
    toolbar_2.setFixedWidth(fit(26));
    toolbar_2.addWidget(&hideShowButton);
    toolbar_2.addSeparator();
    toolbar_2.addWidget(&codeEditorButton);
    toolbar_2.addWidget(&imageEditorButton);
    toolbar_2.addWidget(&hexEditorButton);

    splitter.handle(1)->setDisabled(true);
    editorWrapper.setMinimumWidth(MINWIDTH_FILEEXPLORER);
    explorerWrapper.setFixedWidth(toolbar_2.width());
    connect(&splitter, &QSplitter::splitterMoved, [=] {
        lastWidthOfExplorerWrapper = explorerWrapper.width();
    });
}

int QmlEditorViewPrivate::findRatio(const QString& text)
{
    if (text == "25 %")
        return -4;
    else if (text == "50 %")
        return -2;
    else if (text == "75 %")
        return -1;
    else if (text == "100 %")
        return 0;
    else if (text == "125 %")
        return 1;
    else if (text == "150 %")
        return 2;
    else if (text == "175 %")
        return 3;
    else if (text == "200 %")
        return 4;
    else if (text == "300 %")
        return 5;
    else if (text == "500 %")
        return 7;
    else if (text == "900 %")
        return 9;
    else
        return 0;
}

void QmlEditorViewPrivate::handleCursorPositionChanged()
{
    auto textCursor = codeEditor.textCursor();
    QString lineColText("# Line: %1, Col: %2");
    lineColLabel.setText(lineColText.arg(textCursor.blockNumber() + 1).arg(textCursor.columnNumber()));
}

void QmlEditorViewPrivate::handlePinButtonClicked()
{
    if (pinButton.toolTip().contains("Unpin")) {
        pinButton.setToolTip("Pin Editor.");
        pinButton.setIcon(QIcon(":/resources/images/pin.png"));
        containerWidget.setParent(nullptr);
        containerWidget.show();
        containerWidget.setWindowIcon(QIcon(":/resources/images/owicon.png"));
    } else {
        pinButton.setToolTip("Unpin Editor.");
        pinButton.setIcon(QIcon(":/resources/images/unpin.png"));
        vBoxLayout.addWidget(&containerWidget);
    }
}

void QmlEditorViewPrivate::handleZoomLevelChange(const QString& text)
{
    int ratio = findRatio(text);

    if (previousRatio > 0)
        codeEditor.zoomOut(previousRatio);
    else
        codeEditor.zoomIn(-previousRatio);

    if (ratio > 0)
        codeEditor.zoomIn(ratio);
    else
        codeEditor.zoomOut(-ratio);

    previousRatio = ratio;

}

void QmlEditorViewPrivate::handleHideShowButtonClicked()
{
    if (hideShowButton.toolTip().contains("Hide")) {
        hideShowButton.setIcon(QIcon(":/resources/images/show.png"));
        hideShowButton.setToolTip("Show File Explorer.");
        splitter.handle(1)->setDisabled(true);
        fileExplorer.hide();
        explorerWrapper.setFixedWidth(toolbar_2.width());
        QList<int> sizes;
        sizes << containerWidget.width() - toolbar_2.width();
        sizes << toolbar_2.width();
        splitter.setSizes(sizes);
    } else {
        hideShowButton.setIcon(QIcon(":/resources/images/hide.png"));
        hideShowButton.setToolTip("Hide File Explorer.");
        splitter.handle(1)->setEnabled(true);
        fileExplorer.show();
        explorerWrapper.setMinimumWidth(MINWIDTH_FILEEXPLORER);
        explorerWrapper.setMaximumWidth(9999);
        QList<int> sizes;
        sizes << containerWidget.width() - lastWidthOfExplorerWrapper;
        sizes << lastWidthOfExplorerWrapper;
        splitter.setSizes(sizes);
    }
}

void QmlEditorViewPrivate::handleModeChange()
{
    if (parent->_mode == QmlEditorView::CodeEditor) {
        codeEditorButton.setChecked(true);
        codeEditorButton.setDisabled(true);
        imageEditorButton.setChecked(false);
        hexEditorButton.setChecked(false);
        imageEditorButton.setEnabled(true);
        hexEditorButton.setEnabled(true);

        imageEditor.hide();
        hexEditor.hide();
        codeEditor.show();
    } else if (parent->_mode == QmlEditorView::ImageEditor) {
        imageEditorButton.setChecked(true);
        imageEditorButton.setDisabled(true);
        codeEditorButton.setChecked(false);
        hexEditorButton.setChecked(false);
        codeEditorButton.setEnabled(true);
        hexEditorButton.setEnabled(true);

        codeEditor.hide();
        hexEditor.hide();
        imageEditor.show();
    } else {
        hexEditorButton.setChecked(true);
        hexEditorButton.setDisabled(true);
        imageEditorButton.setChecked(false);
        codeEditorButton.setChecked(false);
        imageEditorButton.setEnabled(true);
        codeEditorButton.setEnabled(true);

        imageEditor.hide();
        codeEditor.hide();
        hexEditor.show();
    }
}

void QmlEditorViewPrivate::handleFileExplorerFileOpen(const QString& filePath)
{
    parent->addDocument(currentControl, filePath);
    parent->setCurrentDocument(currentControl, filePath);
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

// Update when changes made by file explorer
// Update when toolbox controls are changed
void QmlEditorViewPrivate::handleDatabaseChange()
{
    itemsCombobox.clear();
    documentsCombobox.clear();
    for (auto& item : parent->_editorItems) {
        if (item.control->parentControl() != nullptr)
            itemsCombobox.addItem(item.control->parentControl()->id() + ">" + item.control->id());
        else
            itemsCombobox.addItem(">" + item.control->id());

        if (item.control == currentControl) {
            documentsCombobox.addItems(item.documents.keys());

            if (item.control->parentControl() != nullptr)
                itemsCombobox.setCurrentText(item.control->parentControl()->id() + ">" + item.control->id());
            else
                itemsCombobox.setCurrentText(">" + item.control->id());

            documentsCombobox.setCurrentText(item.currentFileRelativePath);

            fileExplorer.setRootPath(item.control->dir() + separator() + DIR_THIS);
        }
    }
}

void QmlEditorViewPrivate::handleItemsComboboxActivated(const QString& text)
{
    if (text.isEmpty())
        return;
    //TODO:
}

void QmlEditorViewPrivate::handleDocumentsComboboxActivated(const QString& text)
{
    if (text.isEmpty())
        return;

    for (auto& item : parent->_editorItems) {
        if (item.control == currentControl &&
            item.currentFileRelativePath != text &&
            item.documents.keys().contains(text)) {
            parent->setCurrentDocument(item.control, item.control->dir() +
                                       separator() + DIR_THIS + separator() + text);
        }
    }
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
    item.documents[relativePath] = new QTextDocument(this);
    item.documents[relativePath]->setDocumentLayout(new QPlainTextDocumentLayout(item.documents[relativePath]));
    item.documents[relativePath]->setPlainText(rdfile(control->url()));
    new QmlHighlighter(item.documents[relativePath]);
    QFont font;
    font.setFamily("Liberation Mono");
    font.setStyleHint(QFont::Monospace);
    item.documents[relativePath]->setDefaultFont(font);
    _editorItems.append(item);

    if (control->parentControl() != nullptr)
        _d->itemsCombobox.addItem(control->parentControl()->id() + ">" + control->id());
    else
        _d->itemsCombobox.addItem(">" + control->id());

    _d->documentsCombobox.addItem(relativePath);
}

void QmlEditorView::addDocument(Control* control, const QString& documentPath)
{
    for (auto& item : _editorItems) {
        if (item.control == control) {
            auto relativePath = documentPath;
            relativePath.remove(control->dir() + separator() + DIR_THIS + separator());
            if (item.documents.keys().contains(relativePath))
                return;
            item.documents[relativePath] = new QTextDocument(this);
            item.documents[relativePath]->setDocumentLayout(new QPlainTextDocumentLayout(item.documents[relativePath]));
            item.documents[relativePath]->setPlainText(rdfile(documentPath));
            new QmlHighlighter(item.documents[relativePath]);
            QFont font;
            font.setFamily("Liberation Mono");
            font.setStyleHint(QFont::Monospace);
            item.documents[relativePath]->setDefaultFont(font);
            _d->documentsCombobox.addItem(relativePath);
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
        }
    }
}

void QmlEditorView::openControl(Control* control)
{
    for (auto& item : _editorItems) {
        if (item.control == control) {
            _d->currentControl = item.control;
            _d->codeEditor.setDocument(item.documents[item.currentFileRelativePath]);

            if (item.control->parentControl() != nullptr)
                _d->itemsCombobox.setCurrentText(item.control->parentControl()->id() + ">" + item.control->id());
            else
                _d->itemsCombobox.setCurrentText(">" + item.control->id());

            _d->documentsCombobox.clear();
            _d->documentsCombobox.addItems(item.documents.keys());
            _d->documentsCombobox.setCurrentText(item.currentFileRelativePath);
            _d->fileExplorer.setRootPath(control->dir() + separator() + DIR_THIS);
        }
    }
}

#include "qmleditorview.moc"
