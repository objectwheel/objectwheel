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

QmlCodeEditorToolBar::QmlCodeEditorToolBar(QmlCodeEditor* codeEditor) : QToolBar(codeEditor)
  , m_pinButton(new QToolButton)
  , m_undoButton(new QToolButton)
  , m_redoButton(new QToolButton)
  , m_closeButton(new QToolButton)
  , m_saveButton(new QToolButton)
  , m_cutButton(new QToolButton)
  , m_copyButton(new QToolButton)
  , m_pasteButton(new QToolButton)
  , m_showButton(new QToolButton)
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
    addWidget(UtilityFunctions::createSpacerWidget(Qt::Horizontal));
    addWidget(m_lineColumnLabel);
    addSeparator();
    addWidget(m_showButton);

    m_pinButton->setFixedHeight(22);
    m_undoButton->setFixedHeight(22);
    m_redoButton->setFixedHeight(22);
    m_closeButton->setFixedHeight(22);
    m_saveButton->setFixedHeight(22);
    m_cutButton->setFixedHeight(22);
    m_copyButton->setFixedHeight(22);
    m_pasteButton->setFixedHeight(22);
    m_showButton->setFixedHeight(22);
    m_lineColumnLabel->setFixedHeight(22);

    m_pinButton->setCursor(Qt::PointingHandCursor);
    m_undoButton->setCursor(Qt::PointingHandCursor);
    m_redoButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_saveButton->setCursor(Qt::PointingHandCursor);
    m_cutButton->setCursor(Qt::PointingHandCursor);
    m_copyButton->setCursor(Qt::PointingHandCursor);
    m_pasteButton->setCursor(Qt::PointingHandCursor);
    m_showButton->setCursor(Qt::PointingHandCursor);

    m_undoButton->setToolTip(tr("Undo action"));
    m_redoButton->setToolTip(tr("Redo action"));
    m_closeButton->setToolTip(tr("Close document"));
    m_saveButton->setToolTip(tr("Save document"));
    m_cutButton->setToolTip(tr("Cut selection"));
    m_copyButton->setToolTip(tr("Copy selection"));
    m_pasteButton->setToolTip(tr("Paste from clipboard"));
    m_showButton->setToolTip(tr("Show/Hide File Explorer"));
    m_lineColumnLabel->setToolTip(tr("Cursor position"));

    m_undoButton->setIcon(UNDO_TOOLBAR.icon());
    m_redoButton->setIcon(REDO_TOOLBAR.icon());
    m_closeButton->setIcon(CLOSE_TOOLBAR.icon());
    m_saveButton->setIcon(SAVEFILE_TOOLBAR.icon());
    m_cutButton->setIcon(CUT_TOOLBAR.icon());
    m_copyButton->setIcon(COPY_TOOLBAR.icon());
    m_pasteButton->setIcon(PASTE_TOOLBAR.icon());
    m_showButton->setIcon(CLOSE_SPLIT_LEFT.icon());

    connect(m_pinButton, &QToolButton::toggled,
            this, &QmlCodeEditorToolBar::onPinButtonToggle);
    connect(m_pinButton, &QToolButton::toggled,
            this, &QmlCodeEditorToolBar::pinned);
    connect(m_showButton, &QToolButton::toggled,
            this, &QmlCodeEditorToolBar::showed);
    connect(m_closeButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::closed);
    connect(m_saveButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::saved);
    connect(codeEditor, &QmlCodeEditor::copyAvailable,
            m_cutButton, &QToolButton::setEnabled);
    connect(m_cutButton, &QToolButton::clicked,
            codeEditor, &QmlCodeEditor::cut);
    connect(codeEditor, &QmlCodeEditor::copyAvailable,
            m_copyButton, &QToolButton::setEnabled);
    connect(m_copyButton, &QToolButton::clicked,
            codeEditor, &QmlCodeEditor::copy);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [=]
    { m_pasteButton->setEnabled(!m_document.isNull() && QApplication::clipboard()->mimeData()->hasText()); });
    connect(m_pasteButton, &QToolButton::clicked,
            codeEditor, &QmlCodeEditor::paste);
    connect(codeEditor, &QmlCodeEditor::cursorPositionChanged,
            this, &QmlCodeEditorToolBar::onCursorPositionChange);

    m_pinButton->setCheckable(true);
    m_showButton->setCheckable(true);
    m_cutButton->setEnabled(codeEditor->textCursor().hasSelection());
    m_copyButton->setEnabled(codeEditor->textCursor().hasSelection());
    m_pasteButton->setEnabled(!m_document.isNull() && QApplication::clipboard()->mimeData()->hasText());
}

void QmlCodeEditorToolBar::sweep()
{
    m_pinButton->setChecked(true);
    m_showButton->setChecked(false);
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
    QTextCursor textCursor = static_cast<QmlCodeEditor*>(parentWidget())->textCursor();
    QString lineColumnText(tr("Line: ") + "%1, " + tr("Col: ") + "%2  ");
    m_lineColumnLabel->setText(lineColumnText
                               .arg(textCursor.blockNumber() + 1)
                               .arg(textCursor.columnNumber() + 1));
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

QSize QmlCodeEditorToolBar::sizeHint() const
{
    return {100, 24};
}

