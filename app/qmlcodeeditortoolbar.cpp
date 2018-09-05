#include <qmlcodeeditortoolbar.h>
#include <qmlcodedocument.h>
#include <utilsicons.h>

#include <QToolButton>
#include <QLabel>

using namespace Utils;
using namespace Icons;

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

    m_pinButton->setFixedHeight(22);
    m_undoButton->setFixedHeight(22);
    m_redoButton->setFixedHeight(22);
    m_closeButton->setFixedHeight(22);
    m_saveButton->setFixedHeight(22);
    m_cutButton->setFixedHeight(22);
    m_copyButton->setFixedHeight(22);
    m_pasteButton->setFixedHeight(22);
    m_lineColumnLabel->setFixedHeight(22);

    m_pinButton->setCursor(Qt::PointingHandCursor);
    m_undoButton->setCursor(Qt::PointingHandCursor);
    m_redoButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_saveButton->setCursor(Qt::PointingHandCursor);
    m_cutButton->setCursor(Qt::PointingHandCursor);
    m_copyButton->setCursor(Qt::PointingHandCursor);
    m_pasteButton->setCursor(Qt::PointingHandCursor);

    m_undoButton->setToolTip(tr("Undo action"));
    m_redoButton->setToolTip(tr("Redo action"));
    m_closeButton->setToolTip(tr("Close document"));
    m_saveButton->setToolTip(tr("Save document"));
    m_cutButton->setToolTip(tr("Cut selection"));
    m_copyButton->setToolTip(tr("Copy selection"));
    m_pasteButton->setToolTip(tr("Paste from clipboard"));
    m_lineColumnLabel->setToolTip(tr("Cursor position"));

    m_undoButton->setIcon(UNDO_TOOLBAR.icon());
    m_redoButton->setIcon(REDO_TOOLBAR.icon());
    m_closeButton->setIcon(CLOSE_TOOLBAR.icon());
    m_saveButton->setIcon(SAVEFILE_TOOLBAR.icon());
    m_cutButton->setIcon(CUT_TOOLBAR.icon());
    m_copyButton->setIcon(COPY_TOOLBAR.icon());
    m_pasteButton->setIcon(PASTE_TOOLBAR.icon());

    m_pinButton->setCheckable(true);
    connect(m_pinButton, &QToolButton::toggled,
            this, &QmlCodeEditorToolBar::onPinButtonToggle);
    connect(m_pinButton, &QToolButton::toggled,
            this, &QmlCodeEditorToolBar::pinned);

    connect(codeEditor, &QmlCodeEditor::cursorPositionChanged,
            this, &QmlCodeEditorToolBar::onCursorPositionChanged);
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
            this, &QmlCodeEditorToolBar::onHideShowButtonClick);
    connect(m_closeButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::onCloseButtonClick);
    connect(m_saveButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::onSaveButtonClick);
    connect(codeEditorButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::onCodeEditorButtonClick);
    connect(imageEditorButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::onImageEditorButtonClick);
    connect(hexEditorButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::onHexEditorButtonClick);
    connect(fileExplorer, &FileExplorer::fileOpened,
            this, &QmlCodeEditorToolBar::onFileExplorerFileOpen);
    connect(fileExplorer, &FileExplorer::fileDeleted,
            this, &QmlCodeEditorToolBar::onFileExplorerFileDeleted);
    connect(fileExplorer, &FileExplorer::fileRenamed,
            this, &QmlCodeEditorToolBar::onFileExplorerFileRenamed);
    connect(&saveAction, &QAction::triggered,
            this, &QmlCodeEditorToolBar::onSaveButtonClick);
    // FIXME connect(SaveManager::instance(), &SaveManager::databaseChanged,
    // this, &QmlCodeEditorToolBar::updateOpenDocHistory);
    connect(SaveManager::instance(), &SaveManager::propertyChanged,
            this, &QmlCodeEditorToolBar::propertyUpdate);
}

void QmlCodeEditorToolBar::sweep()
{
    setDocument(nullptr);
    m_pinButton->setChecked(true);
    m_undoButton->setDisabled(true);
    m_redoButton->setDisabled(true);
    m_copyButton->setDisabled(true);
    m_cutButton->setDisabled(true);
    m_saveButton->setDisabled(true);
}

void QmlCodeEditorToolBar::setDocument(QmlCodeDocument* document)
{
    if (m_document) {
        m_document->disconnect(m_undoButton);
        m_undoButton->disconnect(m_document);
        m_document->disconnect(m_redoButton);
        m_redoButton->disconnect(m_document);
    }

    m_document = document;

    if (m_document) {
        connect(m_document, &QmlCodeDocument::undoAvailable,
                m_undoButton, &QToolButton::setEnabled);
        connect(m_undoButton, &QToolButton::clicked,
                m_document, &QmlCodeDocument::undo);
        connect(m_document, &QmlCodeDocument::redoAvailable,
                m_redoButton, &QToolButton::setEnabled);
        connect(m_redoButton, &QToolButton::clicked,
                m_document, &QmlCodeDocument::redo);
    }
}

void QmlCodeEditorToolBar::onCursorPositionChange()
{
    auto textCursor = codeEditor->textCursor();
    QString lineColText("Line: %1, Col: %2");
    lineColLabel->setText(lineColText.arg(textCursor.blockNumber() + 1).arg(textCursor.columnNumber() + 1));
}

void QmlCodeEditorToolBar::onPinButtonToggle(bool pinned)
{
    if (pinned) {
        m_pinButton->setToolTip(tr("Unpin Editor"));
        m_pinButton->setIcon(PINNED_TOOLBAR.icon());
    } else {
        m_pinButton->setToolTip(tr("Pin Editor"));
        m_pinButton->setIcon(PIN_TOOLBAR.icon());
    }
}

void QmlCodeEditorToolBar::onCloseButtonClick()
{
    for (auto& item : parent->_editorItems) {
        if (item.control == currentControl) {
            parent->closeDocument(item.control, item.control->dir() + separator() +
                                  DIR_THIS + separator() + item.currentFileRelativePath);
            break;
        }
    }
}

void QmlCodeEditorToolBar::onSaveButtonClick()
{
    for (auto& item : parent->_editorItems) {
        if (item.control == currentControl) {
            if (!item.documents.value(item.currentFileRelativePath).document->isModified())
                return;
            parent->saveDocument(item.control, item.control->dir() + separator() +
                                  DIR_THIS + separator() + item.currentFileRelativePath);
            break;
        }
    }
}

QSize QmlCodeEditorToolBar::sizeHint() const
{
    return {100, 24};
}

