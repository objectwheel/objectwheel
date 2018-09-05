#include <qmlcodeeditortoolbar.h>

QmlCodeEditorToolBar::QmlCodeEditorToolBar(QWidget *parent) : QToolBar(parent)
  , m_pinButton(new QToolButton)
  , m_undoButton(new QToolButton)
  , m_redoButton(new QToolButton)
  , m_closeButton(new QToolButton)
  , m_saveButton(new QToolButton)
  , m_cutButton(new QToolButton)
  , m_copyButton(new QToolButton)
  , m_pasteButton(new QToolButton)
{
    redoButton->setDisabled(true);
    copyButton->setDisabled(true);
    cutButton->setDisabled(true);
    saveButton->setDisabled(true);

    connect(parent, &QmlCodeEditorWidget::modeChanged,
            this, &QmlCodeEditorWidgetPrivate::onModeChange);
    connect(codeEditor, &QmlCodeEditor::cursorPositionChanged,
            this, &QmlCodeEditorWidgetPrivate::onCursorPositionChanged);
    connect(pinButton, &QToolButton::clicked,
            this, &QmlCodeEditorWidgetPrivate::onPinButtonClicked);
    connect(undoButton, &QToolButton::clicked,
            codeEditor, &QmlCodeEditor::undo);
    connect(redoButton, &QToolButton::clicked,
            codeEditor, &QmlCodeEditor::redo);
    connect(copyButton, &QToolButton::clicked,
            codeEditor, &QmlCodeEditor::copy);
    connect(cutButton, &QToolButton::clicked,
            codeEditor, &QmlCodeEditor::cut);
    connect(pasteButton, &QToolButton::clicked,
            codeEditor, &QmlCodeEditor::paste);
    connect(codeEditor, &QmlCodeEditor::copyAvailable,
            copyButton, &QToolButton::setEnabled);
    connect(codeEditor, &QmlCodeEditor::copyAvailable,
            cutButton, &QToolButton::setEnabled);
    connect(hideShowButton, &QToolButton::clicked,
            this, &QmlCodeEditorWidgetPrivate::onHideShowButtonClicked);
    connect(closeButton, &QToolButton::clicked,
            this, &QmlCodeEditorWidgetPrivate::onCloseButtonClicked);
    connect(saveButton, &QToolButton::clicked,
            this, &QmlCodeEditorWidgetPrivate::onSaveButtonClicked);
    connect(codeEditorButton, &QToolButton::clicked,
            this, &QmlCodeEditorWidgetPrivate::onCodeEditorButtonClicked);
    connect(imageEditorButton, &QToolButton::clicked,
            this, &QmlCodeEditorWidgetPrivate::onImageEditorButtonClicked);
    connect(hexEditorButton, &QToolButton::clicked,
            this, &QmlCodeEditorWidgetPrivate::onHexEditorButtonClicked);
    connect(fileExplorer, &FileExplorer::fileOpened,
            this, &QmlCodeEditorWidgetPrivate::onFileExplorerFileOpen);
    connect(fileExplorer, &FileExplorer::fileDeleted,
            this, &QmlCodeEditorWidgetPrivate::onFileExplorerFileDeleted);
    connect(fileExplorer, &FileExplorer::fileRenamed,
            this, &QmlCodeEditorWidgetPrivate::onFileExplorerFileRenamed);
    connect(&saveAction, &QAction::triggered,
            this, &QmlCodeEditorWidgetPrivate::onSaveButtonClicked);
    // FIXME connect(SaveManager::instance(), &SaveManager::databaseChanged,
    // this, &QmlCodeEditorWidgetPrivate::updateOpenDocHistory);
    connect(SaveManager::instance(), &SaveManager::propertyChanged,
            this, &QmlCodeEditorWidgetPrivate::propertyUpdate);

    pinButton->setCursor(Qt::PointingHandCursor);
    undoButton->setCursor(Qt::PointingHandCursor);
    redoButton->setCursor(Qt::PointingHandCursor);
    closeButton->setCursor(Qt::PointingHandCursor);
    saveButton->setCursor(Qt::PointingHandCursor);
    cutButton->setCursor(Qt::PointingHandCursor);
    copyButton->setCursor(Qt::PointingHandCursor);
    pasteButton->setCursor(Qt::PointingHandCursor);

    pinButton->setToolTip("Unpin Editor.");
    undoButton->setToolTip("Undo action.");
    redoButton->setToolTip("Redo action.");
    closeButton->setToolTip("Close document.");
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

    pinButton->setFixedHeight(22);
    undoButton->setFixedHeight(22);
    redoButton->setFixedHeight(22);
    closeButton->setFixedHeight(22);
    saveButton->setFixedHeight(22);
    cutButton->setFixedHeight(22);
    copyButton->setFixedHeight(22);
    pasteButton->setFixedHeight(22);

    setFixedHeight(24);
    addWidget(pinButton);
    addSeparator();
    addWidget(undoButton);
    addWidget(redoButton);
    addSeparator();
    addWidget(cutButton);
    addWidget(copyButton);
    addWidget(pasteButton);
    addWidget(saveButton);
    addSeparator();
    sepAction = addSeparator();
    closeAction = addWidget(closeButton);
    sepAction_2 = addSeparator();
    auto empty = new QWidget;
    empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    addWidget(empty);
    addWidget(lineColLabel);
}
