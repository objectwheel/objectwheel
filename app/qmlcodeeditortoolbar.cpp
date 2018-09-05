#include <qmlcodeeditortoolbar.h>
#include <qmlcodedocument.h>
#include <utilsicons.h>
#include <utilityfunctions.h>
#include <qmlcodeeditor.h>

#include <QToolButton>
#include <QLabel>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

using namespace Utils;
using namespace Icons;

QmlCodeEditorToolBar::QmlCodeEditorToolBar(QmlCodeEditor* parent) : QToolBar(parent)
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
    addWidget(new QWidget);
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

    connect(m_pinButton, &QToolButton::toggled,
            this, &QmlCodeEditorToolBar::onPinButtonToggle);
    connect(m_pinButton, &QToolButton::toggled,
            this, &QmlCodeEditorToolBar::pinned);
    connect(m_closeButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::closed);
    connect(m_saveButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::saved);
    connect(parent, &QmlCodeEditor::copyAvailable,
            m_cutButton, &QToolButton::setEnabled);
    connect(m_cutButton, &QToolButton::clicked,
            parent, &QmlCodeEditor::cut);
    connect(parent, &QmlCodeEditor::copyAvailable,
            m_copyButton, &QToolButton::setEnabled);
    connect(m_copyButton, &QToolButton::clicked,
            parent, &QmlCodeEditor::copy);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [=]
    { m_pasteButton->setEnabled(!m_document.isNull() && QApplication::clipboard()->mimeData()->hasText()); });
    connect(m_pasteButton, &QToolButton::clicked,
            parent, &QmlCodeEditor::paste);

    m_pinButton->setCheckable(true);
    m_cutButton->setEnabled(parent->textCursor().hasSelection());
    m_copyButton->setEnabled(parent->textCursor().hasSelection());
    m_pasteButton->setEnabled(!m_document.isNull() && QApplication::clipboard()->mimeData()->hasText());

//    connect(codeEditor, &QmlCodeEditor::cursorPositionChanged,
//            this, &QmlCodeEditorToolBar::onCursorPositionChanged);
//    connect(hideShowButton, &QToolButton::clicked,
//            this, &QmlCodeEditorToolBar::onHideShowButtonClick);
//    connect(&saveAction, &QAction::triggered,
//            this, &QmlCodeEditorToolBar::onSaveButtonClick);
}

void QmlCodeEditorToolBar::sweep()
{
    m_pinButton->setChecked(true);
    setDocument(nullptr);
}

void QmlCodeEditorToolBar::setDocument(QmlCodeDocument* document)
{
    if (m_document) {
        m_document->disconnect(m_undoButton);
        m_document->disconnect(m_redoButton);
        m_document->disconnect(m_saveButton);
        m_undoButton->disconnect(m_document);
        m_redoButton->disconnect(m_document);
        m_saveButton->disconnect(m_document);
    }

    m_document = document;

    if (m_document) {
        connect(m_document.data(), &QmlCodeDocument::undoAvailable,
                m_undoButton, &QToolButton::setEnabled);
        connect(m_undoButton, &QToolButton::clicked,
                m_document.data(), qOverload<>(&QmlCodeDocument::undo));
        connect(m_document.data(), &QmlCodeDocument::redoAvailable,
                m_redoButton, &QToolButton::setEnabled);
        connect(m_redoButton, &QToolButton::clicked,
                m_document.data(), qOverload<>(&QmlCodeDocument::redo));
        connect(m_document.data(), &QmlCodeDocument::modificationChanged,
                m_saveButton, &QToolButton::setEnabled);

        m_undoButton->setEnabled(m_document->isUndoAvailable());
        m_redoButton->setEnabled(m_document->isRedoAvailable());
        m_closeButton->setEnabled(true);
        m_saveButton->setEnabled(m_document->isModified());
        m_pasteButton->setEnabled(QApplication::clipboard()->mimeData()->hasText());
    } else {
        m_undoButton->setDisabled(true);
        m_redoButton->setDisabled(true);
        m_closeButton->setDisabled(true);
        m_saveButton->setDisabled(true);
        m_pasteButton->setDisabled(true);
    }
}

void QmlCodeEditorToolBar::onCursorPositionChange()
{
//    auto textCursor = codeEditor->textCursor();
//    QString lineColText("Line: %1, Col: %2");
//    lineColLabel->setText(lineColText.arg(textCursor.blockNumber() + 1).arg(textCursor.columnNumber() + 1));
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
//    for (auto& item : parent->_editorItems) {
//        if (item.control == currentControl) {
//            parent->closeDocument(item.control, item.control->dir() + separator() +
//                                  DIR_THIS + separator() + item.currentFileRelativePath);
//            break;
//        }
//    }
}

void QmlCodeEditorToolBar::onSaveButtonClick()
{
//    for (auto& item : parent->_editorItems) {
//        if (item.control == currentControl) {
//            if (!item.documents.value(item.currentFileRelativePath).document->isModified())
//                return;
//            parent->saveDocument(item.control, item.control->dir() + separator() +
//                                 DIR_THIS + separator() + item.currentFileRelativePath);
//            break;
//        }
//    }
}

QSize QmlCodeEditorToolBar::sizeHint() const
{
    return {100, 24};
}

