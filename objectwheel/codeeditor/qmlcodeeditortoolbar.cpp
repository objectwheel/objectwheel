#include <qmlcodeeditortoolbar.h>
#include <qmlcodedocument.h>
#include <utilityfunctions.h>
#include <qmlcodeeditor.h>

#include <QToolButton>
#include <QLabel>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QMenu>
#include <QComboBox>
#include <QLayout>

#define MARK_BULLET "•"

namespace {
QAction* g_assetsAction;
QAction* g_designsAction;
QAction* g_othersAction;
}

static QString choppedText(const QString& text)
{
    QString choppedText(text);
    if (choppedText.right(1) == MARK_BULLET)
        choppedText.chop(1);
    return choppedText;
}

static QString bulletText(const QString& text)
{
    QString bulletText(choppedText(text));
    bulletText += MARK_BULLET;
    return bulletText;
}

QmlCodeEditorToolBar::QmlCodeEditorToolBar(QmlCodeEditor* m_codeEditor) : QToolBar(m_codeEditor)
  , m_codeEditor(m_codeEditor)
  , m_pinButton(new QToolButton)
  , m_undoButton(new QToolButton)
  , m_redoButton(new QToolButton)
  , m_closeButton(new QToolButton)
  , m_newFileButton(new QToolButton)
  , m_addFileButton(new QToolButton)
  , m_saveButton(new QToolButton)
  , m_saveAllButton(new QToolButton)
  , m_cutButton(new QToolButton)
  , m_copyButton(new QToolButton)
  , m_pasteButton(new QToolButton)
  , m_showButton(new QToolButton)
  , m_scopeButton(new QToolButton)
  , m_lineColumnLabel(new QLabel)
  , m_leftCombo(new QComboBox)
  , m_rightCombo(new QComboBox)
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    layout()->setSpacing(2);

    addWidget(UtilityFunctions::createSpacingWidget({1, 1}));
    addWidget(m_pinButton);
    addSeparator();
    addWidget(m_undoButton);
    addWidget(m_redoButton);
    addSeparator();
    addWidget(m_cutButton);
    addWidget(m_copyButton);
    addWidget(m_pasteButton);
    addWidget(m_saveButton);
    addWidget(m_saveAllButton);
    addSeparator();
    addWidget(m_scopeButton);
    addSeparator();

    m_actions.append(addWidget(m_newFileButton));                               // 0
    m_actions.append(addWidget(m_addFileButton));                               // 1
    m_actions.append(addSeparator());                                           // 2
    m_actions.append(addWidget(m_closeButton));                                 // 3
    m_actions.append(addSeparator());                                           // 4
    m_actions.append(addWidget(m_leftCombo));                                   // 5
    m_actions.append(addSeparator());                                           // 6
    m_actions.append(addWidget(m_rightCombo));                                  // 7
    m_actions.append(addSeparator());                                           // 8
    addWidget(UtilityFunctions::createSpacerWidget(Qt::Horizontal));
    m_actions.append(addSeparator());                                           // 9
    m_actions.append(addWidget(m_lineColumnLabel));                             // 10
    m_actions.append(addSeparator());                                           // 11
    m_actions.append(addWidget(m_showButton));                                  // 12
    addWidget(UtilityFunctions::createSpacingWidget({1, 1}));

    m_pinButton->setFixedSize({20, 20});
    m_undoButton->setFixedSize({20, 20});
    m_redoButton->setFixedSize({20, 20});
    m_closeButton->setFixedSize({20, 20});
    m_newFileButton->setFixedSize({20, 20});
    m_addFileButton->setFixedSize({20, 20});
    m_saveButton->setFixedSize({20, 20});
    m_saveAllButton->setFixedSize({20, 20});
    m_cutButton->setFixedSize({20, 20});
    m_copyButton->setFixedSize({20, 20});
    m_pasteButton->setFixedSize({20, 20});
    m_showButton->setFixedSize({20, 20});
    m_scopeButton->setFixedSize({20, 20});
    m_leftCombo->setFixedHeight(20);
    m_rightCombo->setFixedHeight(20);
    m_lineColumnLabel->setFixedHeight(20);

    m_pinButton->setCursor(Qt::PointingHandCursor);
    m_undoButton->setCursor(Qt::PointingHandCursor);
    m_redoButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_newFileButton->setCursor(Qt::PointingHandCursor);
    m_addFileButton->setCursor(Qt::PointingHandCursor);
    m_saveButton->setCursor(Qt::PointingHandCursor);
    m_saveAllButton->setCursor(Qt::PointingHandCursor);
    m_cutButton->setCursor(Qt::PointingHandCursor);
    m_copyButton->setCursor(Qt::PointingHandCursor);
    m_pasteButton->setCursor(Qt::PointingHandCursor);
    m_scopeButton->setCursor(Qt::PointingHandCursor);
    m_showButton->setCursor(Qt::PointingHandCursor);
    m_leftCombo->setCursor(Qt::PointingHandCursor);
    m_rightCombo->setCursor(Qt::PointingHandCursor);

    m_undoButton->setToolTip(tr("Undo action") + UtilityFunctions::shortcutSymbol(QKeySequence::Undo));
    m_redoButton->setToolTip(tr("Redo action") + UtilityFunctions::shortcutSymbol(QKeySequence::Redo));
    m_closeButton->setToolTip(tr("Close document") + UtilityFunctions::shortcutSymbol(QKeySequence::Close));
    m_newFileButton->setToolTip(tr("New external file"));
    m_addFileButton->setToolTip(tr("Open external file"));
    m_saveButton->setToolTip(tr("Save document") + UtilityFunctions::shortcutSymbol(QKeySequence::Save));
    m_saveAllButton->setToolTip(tr("Save all open documents") + UtilityFunctions::shortcutSymbol(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
    m_cutButton->setToolTip(tr("Cut selection") + UtilityFunctions::shortcutSymbol(QKeySequence::Cut));
    m_copyButton->setToolTip(tr("Copy selection") + UtilityFunctions::shortcutSymbol(QKeySequence::Copy));
    m_pasteButton->setToolTip(tr("Paste from clipboard") + UtilityFunctions::shortcutSymbol(QKeySequence::Paste));
    m_scopeButton->setToolTip(tr("Change code editor scope"));
    m_lineColumnLabel->setToolTip(tr("Cursor position"));

    m_undoButton->setIcon(QIcon(QStringLiteral(":/images/designer/undo.svg")));
    m_redoButton->setIcon(QIcon(QStringLiteral(":/images/designer/redo.svg")));
    m_closeButton->setIcon(QIcon(QStringLiteral(":/images/designer/close.svg")));
    m_newFileButton->setIcon(QIcon(QStringLiteral(":/images/designer/new-file.svg")));
    m_addFileButton->setIcon(QIcon(QStringLiteral(":/images/designer/add-file.svg")));
    m_saveButton->setIcon(QIcon(QStringLiteral(":/images/designer/save.svg")));
    m_saveAllButton->setIcon(QIcon(QStringLiteral(":/images/designer/save-all.svg")));
    m_cutButton->setIcon(QIcon(QStringLiteral(":/images/designer/cut.svg")));
    m_copyButton->setIcon(QIcon(QStringLiteral(":/images/designer/copy.svg")));
    m_pasteButton->setIcon(QIcon(QStringLiteral(":/images/designer/paste.svg")));

    auto menu = new QMenu(m_scopeButton);
    auto group = new QActionGroup(menu);
    g_assetsAction = new QAction(group);
    g_designsAction = new QAction(group);
    g_othersAction = new QAction(group);

    menu->setToolTipsVisible(true);
    menu->addActions(group->actions());

    group->setExclusive(true);
    group->addAction(g_assetsAction);
    group->addAction(g_designsAction);
    group->addAction(g_othersAction);

    g_assetsAction->setIcon(QIcon(QStringLiteral(":/images/settings/assets.svg")));
    g_assetsAction->setCheckable(true);
    g_assetsAction->setProperty("ow_scope", Assets);
    g_assetsAction->setText(tr("Assets\t"));

    g_designsAction->setIcon(QIcon(QStringLiteral(":/images/designer/view-source-code.svg")));
    g_designsAction->setCheckable(true);
    g_designsAction->setProperty("ow_scope", Designs);
    g_designsAction->setText(tr("User Interface\t"));

    g_othersAction->setIcon(QIcon(QStringLiteral(":/images/designer/external-files.svg")));
    g_othersAction->setCheckable(true);
    g_othersAction->setProperty("ow_scope", Others);
    g_othersAction->setText(tr("External Files\t"));

    m_scopeButton->setPopupMode(QToolButton::InstantPopup);
    m_scopeButton->setMenu(menu);

    m_cutButton->setEnabled(m_codeEditor->textCursor().hasSelection());
    m_copyButton->setEnabled(m_codeEditor->textCursor().hasSelection());
    m_pasteButton->setEnabled(!m_document.isNull()
                              && m_codeEditor->isValid()
                              && QApplication::clipboard()->mimeData()->hasText());

    m_leftCombo->setDuplicatesEnabled(true);
    m_rightCombo->setDuplicatesEnabled(true);
    m_leftCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_rightCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    setPinned(true);
    setShowed(false);
    setDocument(nullptr);
    setScope(Assets);
    QMetaObject::invokeMethod(g_othersAction, &QAction::trigger, Qt::QueuedConnection);

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
    connect(m_addFileButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::addFile);
    connect(m_saveButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::saved);
    connect(m_saveAllButton, &QToolButton::clicked,
            this, &QmlCodeEditorToolBar::savedAll);
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

void QmlCodeEditorToolBar::discharge()
{
    g_assetsAction->setText(tr("Assets\t"));
    g_designsAction->setText(tr("User Interface\t"));
    g_othersAction->setText(tr("External Files\t"));

    setPinned(true);
    setShowed(false);
    setDocument(nullptr);

    m_leftCombo->clear();
    m_rightCombo->clear();

    setScope(Assets);
    QMetaObject::invokeMethod(g_othersAction, &QAction::trigger, Qt::QueuedConnection);
}

void QmlCodeEditorToolBar::setScope(QmlCodeEditorToolBar::Scope scope)
{
    m_scopeButton->setProperty("ow_scope", scope);

    if (scope == Assets)
        m_scopeButton->setIcon(QIcon(QStringLiteral(":/images/settings/assets.svg")));
    else if (scope == Designs)
        m_scopeButton->setIcon(QIcon(QStringLiteral(":/images/designer/view-source-code.svg")));
    else
        m_scopeButton->setIcon(QIcon(QStringLiteral(":/images/designer/external-files.svg")));

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

    if (scope == Assets)
        action = g_assetsAction;
    else if (scope == Designs)
        action = g_designsAction;
    else
        action = g_othersAction;

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
        m_pinButton->setIcon(QIcon(QStringLiteral(":/images/designer/unpin.svg")));
    } else {
        m_pinButton->setToolTip(tr("Pin Editor"));
        m_pinButton->setIcon(QIcon(QStringLiteral(":/images/designer/pin.svg")));
    }
    m_pinButton->setProperty("ow_pinned", pin);
}

void QmlCodeEditorToolBar::onShowButtonClick()
{
    bool show = !m_showButton->property("ow_showed").toBool();
    if (show) {
        m_showButton->setIcon(QIcon(QStringLiteral(":/images/designer/right.svg")));
        m_showButton->setToolTip(tr("Hide File Explorer"));
    } else {
        m_showButton->setIcon(QIcon(QStringLiteral(":/images/designer/left.svg")));
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
    m_actions.at(3)->setVisible(!(action & CloseAction));
    m_actions.at(4)->setVisible(!(action & CloseAction));
    m_actions.at(5)->setVisible(!(action & LeftAction));
    m_actions.at(6)->setVisible(!(action & LeftAction));
    m_actions.at(7)->setVisible(!(action & RightAction));
    m_actions.at(8)->setVisible(!(action & RightAction));
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
    return {100, 22};
}

QSize QmlCodeEditorToolBar::minimumSizeHint() const
{
    return {0, 22};
}

