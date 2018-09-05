#include <qmlcodeeditortoolbar.h>
#include <utilsicons.h>

#include <QToolButton>
#include <QLabel>

using namespace Utils;

namespace {

QWidget* createSpacerWidget()
{
    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return spacer;
}
}

QmlCodeEditorToolBar::QmlCodeEditorToolBar(QWidget *parent) : QToolBar(parent)
  , m_pinButton(new QToolButton)
  , m_undoButton(new QToolButton)
  , m_redoButton(new QToolButton)
  , m_closeButton(new QToolButton)
  , m_saveButton(new QToolButton)
  , m_cutButton(new QToolButton)
  , m_copyButton(new QToolButton)
  , m_pasteButton(new QToolButton)
  , m_lineColumnLabel(new QLabel)
{
    setFixedHeight(24);
    addWidget(m_pinButton);
    addSeparator();
    addWidget(m_undoButton);
    addWidget(m_redoButton);
    addSeparator();
    addWidget(m_cutButton);
    addWidget(m_copyButton);
    addWidget(m_pasteButton);
    addWidget(m_saveButton);
    addSeparator();
    addWidget(m_closeButton);
    addSeparator();
    addWidget(createSpacerWidget());
    addWidget(m_lineColumnLabel);

    m_redoButton->setDisabled(true);
    m_copyButton->setDisabled(true);
    m_cutButton->setDisabled(true);
    m_saveButton->setDisabled(true);

    m_pinButton->setCursor(Qt::PointingHandCursor);
    m_undoButton->setCursor(Qt::PointingHandCursor);
    m_redoButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_saveButton->setCursor(Qt::PointingHandCursor);
    m_cutButton->setCursor(Qt::PointingHandCursor);
    m_copyButton->setCursor(Qt::PointingHandCursor);
    m_pasteButton->setCursor(Qt::PointingHandCursor);

    m_pinButton->setToolTip("Unpin Editor.");
    m_undoButton->setToolTip("Undo action.");
    m_redoButton->setToolTip("Redo action.");
    m_closeButton->setToolTip("Close document.");
    m_lineColumnLabel->setToolTip("Cursor line/column indicator.");
    m_saveButton->setToolTip("Save document.");
    m_cutButton->setToolTip("Cut selection.");
    m_copyButton->setToolTip("Copy selection.");
    m_pasteButton->setToolTip("Paste from clipboard.");

    m_pinButton->setIcon(Icons::PIN_TOOLBAR.icon());
    m_undoButton->setIcon(Icons::UNDO_TOOLBAR.icon());
    m_redoButton->setIcon(Icons::REDO_TOOLBAR.icon());
    m_closeButton->setIcon(Icons::CLOSE_TOOLBAR.icon());
    m_saveButton->setIcon(Icons::SAVEFILE_TOOLBAR.icon());
    m_cutButton->setIcon(Icons::CUT_TOOLBAR.icon());
    m_copyButton->setIcon(Icons::COPY_TOOLBAR.icon());
    m_pasteButton->setIcon(Icons::PASTE_TOOLBAR.icon());

    m_pinButton->setFixedHeight(22);
    m_undoButton->setFixedHeight(22);
    m_redoButton->setFixedHeight(22);
    m_closeButton->setFixedHeight(22);
    m_saveButton->setFixedHeight(22);
    m_cutButton->setFixedHeight(22);
    m_copyButton->setFixedHeight(22);
    m_pasteButton->setFixedHeight(22);

    connect(codeEditor, &QmlCodeEditor::cursorPositionChanged,
            this, &QmlCodeEditorWidgetPrivate::onCursorPositionChanged);
    connect(m_pinButton, &QToolButton::clicked,
            this, &QmlCodeEditorWidgetPrivate::onPinButtonClicked);
    connect(m_undoButton, &QToolButton::clicked,
            codeEditor, &QmlCodeEditor::undo);
    connect(m_redoButton, &QToolButton::clicked,
            codeEditor, &QmlCodeEditor::redo);
    connect(m_copyButton, &QToolButton::clicked,
            codeEditor, &QmlCodeEditor::copy);
    connect(m_cutButton, &QToolButton::clicked,
            codeEditor, &QmlCodeEditor::cut);
    connect(m_pasteButton, &QToolButton::clicked,
            codeEditor, &QmlCodeEditor::paste);
    connect(codeEditor, &QmlCodeEditor::copyAvailable,
            m_copyButton, &QToolButton::setEnabled);
    connect(codeEditor, &QmlCodeEditor::copyAvailable,
            m_cutButton, &QToolButton::setEnabled);
    connect(hideShowButton, &QToolButton::clicked,
            this, &QmlCodeEditorWidgetPrivate::onHideShowButtonClicked);
    connect(m_closeButton, &QToolButton::clicked,
            this, &QmlCodeEditorWidgetPrivate::onCloseButtonClicked);
    connect(m_saveButton, &QToolButton::clicked,
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
}

void QmlCodeEditorToolBar::setDocument(QmlCodeDocument* document)
{
    m_document = document;
}
