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
#include <QMenu>
#include <QComboBox>
#include <QTimer>

using namespace Utils;
using namespace Icons;

#define MARK_BULLET "•"

namespace {
QAction* g_globalAction;
QAction* g_internalAction;
QAction* g_externalAction;

QString choppedText(const QString& text)
{
    QString choppedText(text);
    if (choppedText.right(1) == MARK_BULLET)
        choppedText.chop(1);
    return choppedText;
}

QString bulletText(const QString& text)
{
    QString bulletText(choppedText(text));
    bulletText += MARK_BULLET;
    return bulletText;
}
}

QmlCodeEditorToolBar::QmlCodeEditorToolBar(QmlCodeEditor* m_codeEditor) : QToolBar(m_codeEditor)
  , m_codeEditor(m_codeEditor)
  , m_pinButton(new QToolButton)
  , m_undoButton(new QToolButton)
  , m_redoButton(new QToolButton)
  , m_closeButton(new QToolButton)
  , m_newFileButton(new QToolButton)
  , m_openFileButton(new QToolButton)
  , m_saveButton(new QToolButton)
  , m_cutButton(new QToolButton)
  , m_copyButton(new QToolButton)
  , m_pasteButton(new QToolButton)
  , m_showButton(new QToolButton)
  , m_scopeButton(new QToolButton)
  , m_lineColumnLabel(new QLabel)
  , m_leftCombo(new QComboBox)
  , m_rightCombo(new QComboBox)
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
    addWidget(m_scopeButton);
    addSeparator();
    m_actions.append(addWidget(m_newFileButton));   // 0
    m_actions.append(addWidget(m_openFileButton));  // 1
    m_actions.append(addSeparator());               // 2
    m_actions.append(addWidget(m_leftCombo));       // 3
    m_actions.append(addSeparator());               // 4
    m_actions.append(addWidget(m_rightCombo));      // 5
    m_actions.append(addSeparator());               // 6
    m_actions.append(addWidget(m_closeButton));     // 7
    m_actions.append(addSeparator());               // 8
    addWidget(UtilityFunctions::createSpacerWidget(Qt::Horizontal));
    m_actions.append(addSeparator());               // 9
    m_actions.append(addWidget(m_lineColumnLabel)); // 10
    m_actions.append(addSeparator());               // 11
    m_actions.append(addWidget(m_showButton));      // 12

    m_pinButton->setFixedHeight(22);
    m_undoButton->setFixedHeight(22);
    m_redoButton->setFixedHeight(22);
    m_closeButton->setFixedHeight(22);
    m_newFileButton->setFixedHeight(22);
    m_openFileButton->setFixedHeight(22);
    m_saveButton->setFixedHeight(22);
    m_cutButton->setFixedHeight(22);
    m_copyButton->setFixedHeight(22);
    m_pasteButton->setFixedHeight(22);
    m_showButton->setFixedHeight(22);
    m_scopeButton->setFixedHeight(22);
    m_leftCombo->setFixedHeight(22);
    m_rightCombo->setFixedHeight(22);
    m_lineColumnLabel->setFixedHeight(22);

    m_pinButton->setCursor(Qt::PointingHandCursor);
    m_undoButton->setCursor(Qt::PointingHandCursor);
    m_redoButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_newFileButton->setCursor(Qt::PointingHandCursor);
    m_openFileButton->setCursor(Qt::PointingHandCursor);
    m_saveButton->setCursor(Qt::PointingHandCursor);
    m_cutButton->setCursor(Qt::PointingHandCursor);
    m_copyButton->setCursor(Qt::PointingHandCursor);
    m_pasteButton->setCursor(Qt::PointingHandCursor);
    m_scopeButton->setCursor(Qt::PointingHandCursor);
    m_showButton->setCursor(Qt::PointingHandCursor);
    m_leftCombo->setCursor(Qt::PointingHandCursor);
    m_rightCombo->setCursor(Qt::PointingHandCursor);

    m_undoButton->setToolTip(tr("Undo action"));
    m_redoButton->setToolTip(tr("Redo action"));
    m_closeButton->setToolTip(tr("Close document"));
    m_newFileButton->setToolTip(tr("New external file"));
    m_openFileButton->setToolTip(tr("Open external file"));
    m_saveButton->setToolTip(tr("Save document"));
    m_cutButton->setToolTip(tr("Cut selection"));
    m_copyButton->setToolTip(tr("Copy selection"));
    m_pasteButton->setToolTip(tr("Paste from clipboard"));
    m_scopeButton->setToolTip(tr("Change the scope of open files"));
    m_lineColumnLabel->setToolTip(tr("Cursor position"));

    m_undoButton->setIcon(UNDO_TOOLBAR.icon());
    m_redoButton->setIcon(REDO_TOOLBAR.icon());
    m_closeButton->setIcon(CLOSE_TOOLBAR.icon());
    m_newFileButton->setIcon(FILENEW.icon());
    m_openFileButton->setIcon(OPENFILE_TOOLBAR.icon());
    m_saveButton->setIcon(SAVEFILE_TOOLBAR.icon());
    m_cutButton->setIcon(CUT_TOOLBAR.icon());
    m_copyButton->setIcon(COPY_TOOLBAR.icon());
    m_pasteButton->setIcon(PASTE_TOOLBAR.icon());

    auto menu = new QMenu(m_scopeButton);
    auto group = new QActionGroup(menu);
    g_globalAction = new QAction(group);
    g_internalAction = new QAction(group);
    g_externalAction = new QAction(group);

    menu->setToolTipsVisible(true);
    menu->addActions(group->actions());

    group->setExclusive(true);
    group->addAction(g_globalAction);
    group->addAction(g_internalAction);
    group->addAction(g_externalAction);

    g_globalAction->setIcon(GLOBAL_TOOLBAR.icon());
    g_globalAction->setCheckable(true);
    g_globalAction->setProperty("ow_scope", Global);

    g_internalAction->setIcon(INTERNAL_TOOLBAR.icon());
    g_internalAction->setCheckable(true);
    g_internalAction->setProperty("ow_scope", Internal);

    g_externalAction->setIcon(EXTERNAL_TOOLBAR.icon());
    g_externalAction->setCheckable(true);
    g_externalAction->setProperty("ow_scope", External);

    m_scopeButton->setAttribute(Qt::WA_Hover);
    m_scopeButton->setPopupMode(QToolButton::InstantPopup);
    m_scopeButton->setMenu(menu);

    m_cutButton->setEnabled(m_codeEditor->textCursor().hasSelection());
    m_copyButton->setEnabled(m_codeEditor->textCursor().hasSelection());
    m_pasteButton->setEnabled(!m_document.isNull()
                              && m_codeEditor->isValid()
                              && QApplication::clipboard()->mimeData()->hasText());

    m_leftCombo->setDuplicatesEnabled(true);
    m_rightCombo->setDuplicatesEnabled(true);
    QTimer::singleShot(1000, [=] { // FIXME: This fixes a weird bug on TransparentStyle
        m_leftCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        m_rightCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    });

    connect(m_pinButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::onPinButtonClick);
    connect(m_pinButton, &QToolButton::clicked,
            this, [=] {
            emit pinned(m_pinButton->property("ow_pinned").toBool());
    });
    connect(m_showButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::onShowButtonClick);
    connect(m_showButton, &QToolButton::clicked,
            this, [=] {
            emit showed(m_showButton->property("ow_showed").toBool());
    });
    connect(m_closeButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::closed);
    connect(m_newFileButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::newFile);
    connect(m_openFileButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::openFile);
    connect(m_saveButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::saved);
    connect(m_codeEditor, &QmlCodeEditor::copyAvailable,
            m_cutButton, &QToolButton::setEnabled);
    connect(m_cutButton, &QToolButton::clicked,
            m_codeEditor, &QmlCodeEditor::cut);
    connect(m_codeEditor, &QmlCodeEditor::copyAvailable,
            m_copyButton, &QToolButton::setEnabled);
    connect(m_copyButton, &QToolButton::clicked,
            m_codeEditor, &QmlCodeEditor::copy);
    connect(m_codeEditor, &QmlCodeEditor::documentChanged,
            this, &QmlCodeEditorToolBar::onClipboardDataChange);
    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &QmlCodeEditorToolBar::onClipboardDataChange);
    connect(m_pasteButton, &QToolButton::clicked,
            m_codeEditor, &QmlCodeEditor::paste);
    connect(m_codeEditor, &QmlCodeEditor::cursorPositionChanged,
            this, &QmlCodeEditorToolBar::onCursorPositionChange);
    connect(menu, &QMenu::triggered, this,
            &QmlCodeEditorToolBar::onScopeActivation);
    connect(m_leftCombo, qOverload<int>(&QComboBox::activated),
            this, &QmlCodeEditorToolBar::onComboActivation);
    connect(m_rightCombo, qOverload<int>(&QComboBox::activated),
            this, &QmlCodeEditorToolBar::onComboActivation);
}

void QmlCodeEditorToolBar::setPinned(bool pin)
{
    m_pinButton->setProperty("ow_pinned", !pin);
    onPinButtonClick();
}

void QmlCodeEditorToolBar::setShowed(bool show)
{
    m_showButton->setProperty("ow_showed", !show);
    onShowButtonClick();
}

void QmlCodeEditorToolBar::sweep()
{
    g_globalAction->setText(tr("Global\t"));
    g_internalAction->setText(tr("Internal\t"));
    g_externalAction->setText(tr("External\t"));

    setPinned(true);
    setShowed(false);
    setDocument(nullptr);

    m_leftCombo->clear();
    m_rightCombo->clear();

    setScope(Global);
}

void QmlCodeEditorToolBar::setScope(QmlCodeEditorToolBar::Scope scope)
{
    m_scopeButton->setProperty("ow_scope", scope);

    if (scope == Global)
        m_scopeButton->setIcon(GLOBAL_TOOLBAR.icon());
    else if (scope == Internal)
        m_scopeButton->setIcon(INTERNAL_TOOLBAR.icon());
    else
        m_scopeButton->setIcon(EXTERNAL_TOOLBAR.icon());

    for (QAction* action : m_scopeButton->menu()->actions()) {
        Scope s = action->property("ow_scope").value<Scope>();
        if (scope == s)
            action->setChecked(true);
    }

    // emit scopeChanged(scope);
}

void QmlCodeEditorToolBar::setScopeWide(QmlCodeEditorToolBar::Scope scope, bool wide)
{
    QAction* action;

    if (scope == Global)
        action = g_globalAction;
    else if (scope == Internal)
        action = g_internalAction;
    else
        action = g_externalAction;

    if (wide)
        action->setText(bulletText(action->text()));
    else
        action->setText(choppedText(action->text()));
}

void QmlCodeEditorToolBar::onPinButtonClick()
{
    bool pin = !m_pinButton->property("ow_pinned").toBool();
    if (pin) {
        m_pinButton->setToolTip(tr("Unpin Editor"));
        m_pinButton->setIcon(PINNED_TOOLBAR.icon());
    } else {
        m_pinButton->setToolTip(tr("Pin Editor"));
        m_pinButton->setIcon(PIN_TOOLBAR.icon());
    }
    m_pinButton->setProperty("ow_pinned", pin);
}

void QmlCodeEditorToolBar::onShowButtonClick()
{
    bool show = !m_showButton->property("ow_showed").toBool();
    if (show) {
        m_showButton->setIcon(CLOSE_SPLIT_RIGHT.icon());
        m_showButton->setToolTip(tr("Hide File Explorer"));
    } else {
        m_showButton->setIcon(CLOSE_SPLIT_LEFT.icon());
        m_showButton->setToolTip(tr("Show File Explorer"));
    }
    m_showButton->setProperty("ow_showed", show);
}

void QmlCodeEditorToolBar::onClipboardDataChange()
{
    m_pasteButton->setEnabled(!m_document.isNull()
                              && m_codeEditor->isValid()
                              && QApplication::clipboard()->mimeData()->hasText());
}

void QmlCodeEditorToolBar::onCursorPositionChange()
{
    QTextCursor textCursor = static_cast<QmlCodeEditor*>(parentWidget())->textCursor();
    QString lineColumnText(tr("  Line: ") + "%1, " + tr("Col: ") + "%2  ");
    m_lineColumnLabel->setText(lineColumnText
                               .arg(textCursor.blockNumber() + 1)
                               .arg(textCursor.columnNumber() + 1));
}

void QmlCodeEditorToolBar::onScopeActivation(QAction* action)
{
    Scope scope = action->property("ow_scope").value<Scope>();
    setScope(scope);
    emit scopeActivated(scope);
}

void QmlCodeEditorToolBar::onComboActivation()
{
    if (auto combo = qobject_cast<QComboBox*>(sender())) {
        if (combo == m_leftCombo)
            return comboActivated(LeftCombo);
        if (combo == m_rightCombo)
            return comboActivated(RightCombo);
    }
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

void QmlCodeEditorToolBar::setHiddenActions(QmlCodeEditorToolBar::DocumentActions action)
{
    m_actions.at(0)->setVisible(!(action & FileActions));
    m_actions.at(1)->setVisible(!(action & FileActions));
    m_actions.at(2)->setVisible(!(action & FileActions));
    m_actions.at(3)->setVisible(!(action & LeftAction));
    m_actions.at(4)->setVisible(!(action & LeftAction));
    m_actions.at(5)->setVisible(!(action & RightAction));
    m_actions.at(6)->setVisible(!(action & RightAction));
    m_actions.at(7)->setVisible(!(action & CloseAction));
    m_actions.at(8)->setVisible(!(action & CloseAction));
    m_actions.at(9)->setVisible(!(action & LineColAction));
    m_actions.at(10)->setVisible(!(action & LineColAction));
    m_actions.at(11)->setVisible(!(action & ShowAction));
    m_actions.at(12)->setVisible(!(action & ShowAction));
}

QmlCodeEditorToolBar::Scope QmlCodeEditorToolBar::scope() const
{
    return m_scopeButton->property("ow_scope").value<Scope>();
}

QComboBox* QmlCodeEditorToolBar::combo(QmlCodeEditorToolBar::Combo combo) const
{
    if (combo == LeftCombo)
        return m_leftCombo;
    return m_rightCombo;
}

QSize QmlCodeEditorToolBar::sizeHint() const
{
    return {100, 24};
}

