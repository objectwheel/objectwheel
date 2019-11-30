#include <qmlcodeeditor.h>
#include <rowbar.h>
#include <qmlcodeeditortoolbar.h>
#include <qtcassert.h>
#include <qmlcodedocument.h>
#include <bracketband.h>
#include <codeeditorsettings.h>
#include <fontcolorssettings.h>

#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/qmljsbind.h>
#include <qmljs/qmljsscopechain.h>
#include <qmljs/qmljsevaluate.h>
#include <qmljstools/qmljssemanticinfo.h>
#include <qmljstools/qmljsindenter.h>
#include <qmljseditor/quicktoolbar.h>
#include <qmljseditor/qmljscompletionassist.h>
#include <qmljseditor/qmljshoverhandler.h>
#include <qmljs/qmljsutils.h>
#include <coreplugin/find/basetextfind.h>

#include <utils/textutils.h>
#include <utils/tooltip/tooltip.h>
#include <utils/link.h>

#include <texteditor/codeassist/codeassistant.h>
#include <texteditor/colorpreviewhoverhandler.h>
#include <texteditor/texteditorsettings.h>
#include <texteditor/icodestylepreferences.h>
#include <texteditor/texteditoroverlay.h>
#include <texteditor/refactoroverlay.h>

#include <QFontDatabase>
#include <QPainter>
#include <QApplication>
#include <QScrollBar>
#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include <QAction>
#include <QLabel>
#include <QComboBox>

using namespace Utils;
using namespace TextEditor;
using namespace QmlJSEditor;
using namespace QmlJSEditor::Internal;
using namespace Utils;
using namespace QmlJS;
using namespace QmlJS::AST;
using namespace QmlJSTools;
using namespace Core;

class QtQuickToolbarMarker {};
Q_DECLARE_METATYPE(QtQuickToolbarMarker)

class HoverHandlerRunner
{
public:
    HoverHandlerRunner(QmlCodeEditor *widget, QList<BaseHoverHandler *> &handlers)
        : m_widget(widget)
        , m_handlers(handlers)
    {
    }

    void startChecking(const QTextCursor &textCursor, const QPoint &point)
    {
        if (m_handlers.empty())
            return;

        // Does the last handler still applies?
        const int documentRevision = textCursor.document()->revision();
        const int position = Text::wordStartCursor(textCursor).position();
        if (m_lastHandlerInfo.applies(documentRevision, position)) {
            m_lastHandlerInfo.handler->showToolTip(m_widget, point, /*decorate=*/ false);
            return;
        }

        if (isCheckRunning(documentRevision, position))
            return;

        // Cancel currently running checks
        for (BaseHoverHandler *handler : m_handlers)
            handler->abort();

        // Update invocation data
        m_documentRevision = documentRevision;
        m_position = position;
        m_point = point;

        // Re-initialize process data
        m_currentHandlerIndex = 0;
        m_bestHandler = nullptr;
        m_highestHandlerPriority = -1;

        // Start checking
        checkNext();
    }

    bool isCheckRunning(int documentRevision, int position) const
    {
        return m_currentHandlerIndex <= m_handlers.size()
                && m_documentRevision == documentRevision
                && m_position == position;
    }

    void checkNext()
    {
        QTC_ASSERT(m_currentHandlerIndex < m_handlers.size(), return);
        BaseHoverHandler *currentHandler = m_handlers[m_currentHandlerIndex];

        currentHandler->checkPriority(m_widget, m_position, [this](int priority) {
            onHandlerFinished(m_documentRevision, m_position, priority);
        });
    }

    void onHandlerFinished(int documentRevision, int position, int priority)
    {
        QTC_ASSERT(m_currentHandlerIndex < m_handlers.size(), return);
        QTC_ASSERT(documentRevision == m_documentRevision, return);
        QTC_ASSERT(position == m_position, return);

        BaseHoverHandler *currentHandler = m_handlers[m_currentHandlerIndex];
        if (priority > m_highestHandlerPriority) {
            m_highestHandlerPriority = priority;
            m_bestHandler = currentHandler;
        }

        // There are more, check next
        ++m_currentHandlerIndex;
        if (m_currentHandlerIndex < m_handlers.size()) {
            checkNext();
            return;
        }

        // All were queried, run the best
        if (m_bestHandler) {
            m_lastHandlerInfo = LastHandlerInfo(m_bestHandler, m_documentRevision, m_position);
            m_bestHandler->showToolTip(m_widget, m_point);
        }
    }

private:
    QmlCodeEditor *m_widget = nullptr;
    const QList<BaseHoverHandler *> &m_handlers;

    struct LastHandlerInfo {
        LastHandlerInfo() = default;
        LastHandlerInfo(BaseHoverHandler *handler, int documentRevision, int cursorPosition)
            : handler(handler)
            , documentRevision(documentRevision)
            , cursorPosition(cursorPosition)
        {}

        bool applies(int documentRevision, int cursorPosition) const
        {
            return handler
                    && documentRevision == this->documentRevision
                    && cursorPosition == this->cursorPosition;
        }

        BaseHoverHandler *handler = nullptr;
        int documentRevision = -1;
        int cursorPosition = -1;
    } m_lastHandlerInfo;

    // invocation data
    QPoint m_point;
    int m_position = -1;
    int m_documentRevision = -1;

    // processing data
    int m_currentHandlerIndex = -1;
    int m_highestHandlerPriority = -1;
    BaseHoverHandler *m_bestHandler = nullptr;
};

struct PaintEventData
{
    PaintEventData(QmlCodeEditor *editor, QPaintEvent *event, QPointF offset)
        : offset(offset)
        , viewportRect(editor->viewport()->rect())
        , eventRect(event->rect())
        , doc(editor->document())
        , documentLayout(qobject_cast<QPlainTextDocumentLayout*>(doc->documentLayout()))
        , documentWidth(int(doc->size().width()))
        , textCursor(editor->textCursor())
        , textCursorBlock(textCursor.block())
        , isEditable(!editor->isReadOnly())
        , searchScopeFormat(CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_SEARCH_SCOPE))
        , searchResultFormat(CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_SEARCH_RESULT))
        , visualWhitespaceFormat(CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_VISUAL_WHITESPACE))
        , ifdefedOutFormat(CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_DISABLED_CODE))
        , suppressSyntaxInIfdefedOutBlock(ifdefedOutFormat.foreground() != editor->palette().windowText())
    { }
    QPointF offset;
    const QRect viewportRect;
    const QRect eventRect;
    qreal rightMargin = -1;
    const QTextDocument *doc;
    QPlainTextDocumentLayout *documentLayout;
    const int documentWidth;
    const QTextCursor textCursor;
    const QTextBlock textCursorBlock;
    const bool isEditable;
    const QTextCharFormat searchScopeFormat;
    const QTextCharFormat searchResultFormat;
    const QTextCharFormat visualWhitespaceFormat;
    const QTextCharFormat ifdefedOutFormat;
    const bool suppressSyntaxInIfdefedOutBlock;
    QAbstractTextDocumentLayout::PaintContext context;
    QTextBlock visibleCollapsedBlock;
    QPointF visibleCollapsedBlockOffset;
    QTextBlock block;
    QTextLayout *cursorLayout = nullptr;
    QPointF cursorOffset;
    int cursorPos = 0;
    QPen cursorPen;
    int blockSelectionIndex = -1;
};

struct PaintEventBlockData
{
    QRectF boundingRect;
    QVector<QTextLayout::FormatRange> selections;
    QVector<QTextLayout::FormatRange> prioritySelections;
    QRectF blockSelectionCursorRect;
    QTextLayout *layout = nullptr;
    int position = 0;
    int length = 0;
};

static inline bool isPrintableText(const QString &text)
{
    return !text.isEmpty() && (text.at(0).isPrint() || text.at(0) == QLatin1Char('\t'));
}

static QTextLayout::FormatRange createBlockCursorCharFormatRange(int pos, const QPalette &palette)
{
    QTextLayout::FormatRange o;
    o.start = pos;
    o.length = 1;
    o.format.setForeground(palette.base());
    o.format.setBackground(palette.text());
    return o;
}

// could go into QTextCursor...
static QTextLine currentTextLine(const QTextCursor &cursor)
{
    const QTextBlock block = cursor.block();
    if (!block.isValid())
        return {};

    const QTextLayout *layout = block.layout();
    if (!layout)
        return {};

    const int relativePos = cursor.position() - block.position();
    return layout->lineForTextPosition(relativePos);
}

TextEditorAnimator::TextEditorAnimator(QObject *parent)
    : QObject(parent), m_timeline(256)
{
    m_value = 0;
    m_timeline.setCurveShape(QTimeLine::SineCurve);
    connect(&m_timeline, &QTimeLine::valueChanged, this, &TextEditorAnimator::step);
    connect(&m_timeline, &QTimeLine::finished, this, &QObject::deleteLater);
    m_timeline.start();
}

void TextEditorAnimator::init(const QTextCursor &cursor, const QFont &f, const QPalette &pal)
{
    m_cursor = cursor;
    m_font = f;
    m_palette = pal;
    m_text = cursor.selectedText();
    QFontMetrics fm(m_font);
    m_size = QSizeF(fm.horizontalAdvance(m_text), fm.height());
}

void TextEditorAnimator::draw(QPainter *p, const QPointF &pos)
{
    m_lastDrawPos = pos;
    p->setPen(m_palette.text().color());
    QFont f = m_font;
    f.setPixelSize(f.pixelSize() * (1.0 + m_value/2));
    QFontMetrics fm(f);
    int width = fm.horizontalAdvance(m_text);
    QRectF r((m_size.width()-width)/2, (m_size.height() - fm.height())/2, width, fm.height());
    r.translate(pos);
    p->fillRect(r, m_palette.base());
    p->setFont(f);
    p->drawText(r, m_text);
}

bool TextEditorAnimator::isRunning() const
{
    return m_timeline.state() == QTimeLine::Running;
}

QRectF TextEditorAnimator::rect() const
{
    QFont f = m_font;
    f.setPixelSize(f.pixelSize() * (1.0 + m_value/2));
    QFontMetrics fm(f);
    int width = fm.horizontalAdvance(m_text);
    return QRectF((m_size.width()-width)/2, (m_size.height() - fm.height())/2, width, fm.height());
}

void TextEditorAnimator::step(qreal v)
{
    QRectF before = rect();
    m_value = v;
    QRectF after = rect();
    emit updateRequest(m_cursor, m_lastDrawPos, before.united(after));
}

void TextEditorAnimator::finish()
{
    m_timeline.stop();
    step(0);
    deleteLater();
}

static void appendExtraSelectionsForMessages(
        QList<QTextEdit::ExtraSelection> *selections,
        const QList<DiagnosticMessage> &messages,
        const QmlCodeDocument *document)
{
    foreach (const DiagnosticMessage &d, messages) {
        const int line = d.loc.startLine;
        const int column = qMax(1U, d.loc.startColumn);

        QTextEdit::ExtraSelection sel;
        QTextCursor c(document->findBlockByNumber(line - 1));
        sel.cursor = c;

        sel.cursor.setPosition(c.position() + column - 1);

        if (d.loc.length == 0) {
            if (sel.cursor.atBlockEnd())
                sel.cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
            else
                sel.cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        } else {
            sel.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, d.loc.length);
        }

        if (d.isWarning())
            sel.format = CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_WARNING);
        else
            sel.format = CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_ERROR);

        sel.format.setToolTip(d.message);

        selections->append(sel);
    }
}

template <class T>
static QList<RefactorMarker> removeMarkersOfType(const QList<RefactorMarker> &markers)
{
    QList<RefactorMarker> result;
    foreach (const RefactorMarker &marker, markers) {
        if (!marker.data.canConvert<T>())
            result += marker;
    }
    return result;
}

CompletionAssistProvider* QmlCodeEditor::m_completionAssistProvider = nullptr;
QmlJSHoverHandler* QmlCodeEditor::m_qmlJsHoverHandler = nullptr;
ColorPreviewHoverHandler* QmlCodeEditor::m_colorPreviewHoverHandler = nullptr;
QuickToolBar* QmlCodeEditor::m_contextPane = nullptr;

QmlCodeEditor::QmlCodeEditor(QWidget* parent) : QPlainTextEdit(parent)
  , m_initialEmptyDocument(new QmlCodeDocument(this))
  , m_noDocsLabel(new QLabel(this))
  , m_rowBar(new RowBar(this, this))
  , m_toolBar(new QmlCodeEditorToolBar(this))
  , m_linkPressed(false)
  , m_fontSettingsNeedsApply(true)
  , m_parenthesesMatchingEnabled(true)
  , m_oldCursorPosition(-1)
  , m_visibleFoldedBlockNumber(-1)
  , m_updateUsesTimer(new QTimer(this))
  , m_contextPaneTimer(new QTimer(this))
  , m_parenthesesMatchingTimer(new QTimer(this))
  , m_hoverHandlerRunner(new HoverHandlerRunner(this, m_hoverHandlers))
  , m_codeAssistant(new TextEditor::CodeAssistant)
  , m_bracketsAnimator(nullptr)
  , m_autocompleteAnimator(nullptr)
  , m_overlay(new TextEditor::Internal::TextEditorOverlay(this))
  , m_refactorOverlay(new TextEditor::RefactorOverlay(this))
  , m_searchResultOverlay(new TextEditor::Internal::TextEditorOverlay(this))
  , m_autoCompleter(new QmlJSEditor::Internal::AutoCompleter)
{
    m_noDocsLabel->setVisible(false);
    m_noDocsLabel->setAlignment(Qt::AlignCenter);
    m_noDocsLabel->setText(tr("No documents\nopen"));
    m_noDocsLabel->setStyleSheet("QLabel { background: #f0f0f0; color: #808080;}");

    auto baseTextFind = new BaseTextFind(this); // BUG
    connect(baseTextFind, &BaseTextFind::highlightAllRequested,
            this, &QmlCodeEditor::highlightSearchResultsSlot);
    connect(baseTextFind, &BaseTextFind::findScopeChanged,
            this, &QmlCodeEditor::setFindScope);

    setAcceptDrops(false);
    setMouseTracking(true);

    if (!m_qmlJsHoverHandler) {
        m_completionAssistProvider = new QmlJSCompletionAssistProvider;
        m_colorPreviewHoverHandler = new ColorPreviewHoverHandler;
        m_qmlJsHoverHandler = new QmlJSHoverHandler;
    }

    addHoverHandler(m_qmlJsHoverHandler);
    addHoverHandler(m_colorPreviewHoverHandler);

    if (!m_contextPane)
        m_contextPane = new QuickToolBar;
    m_contextPaneTimer->setInterval(500);
    m_contextPaneTimer->setSingleShot(true);
    connect(m_contextPaneTimer, &QTimer::timeout, this, &QmlCodeEditor::updateContextPane);
    if (m_contextPane) {
        connect(this, &QmlCodeEditor::cursorPositionChanged,
                m_contextPaneTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
        connect(m_contextPane, &IContextPane::closed, this, &QmlCodeEditor::showTextMarker);
    }

    m_updateUsesTimer->setInterval(150);
    m_updateUsesTimer->setSingleShot(true);
    connect(m_updateUsesTimer, &QTimer::timeout, this, &QmlCodeEditor::updateUses);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            m_updateUsesTimer, static_cast<void (QTimer::*)()>(&QTimer::start));

    connect(this, &QPlainTextEdit::selectionChanged, this, &QmlCodeEditor::slotSelectionChanged);
    connect(this, &QmlCodeEditor::blockCountChanged, this, &QmlCodeEditor::updateViewportMargins);
    connect(this, &QmlCodeEditor::updateRequest, this, &QmlCodeEditor::updateRowBar);
    connect(this, &QmlCodeEditor::cursorPositionChanged, this, &QmlCodeEditor::slotCursorPositionChanged);
    connect(CodeEditorSettings::instance(), &CodeEditorSettings::fontColorsSettingsChanged,
            this, &QmlCodeEditor::applyFontSettingsDelayed, Qt::QueuedConnection);
    // NOTE: It is a QueuedConnection connection, hence all the docs get updated before we call applyFontSettingsDelayed

    m_parenthesesMatchingTimer->setSingleShot(true);
    connect(m_parenthesesMatchingTimer, &QTimer::timeout, this, &QmlCodeEditor::matchParentheses);

    QAction *completionAction = new QAction(tr("Trigger Completion"), this);
    completionAction->setShortcut(QKeySequence(HostOsInfo::isMacHost() ? tr("Alt+Space") : tr("Ctrl+Space")));
    completionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(completionAction);

    connect(completionAction, &QAction::triggered, [this]() {
        invokeAssist(TextEditor::Completion);
    });

    setCodeDocument(m_initialEmptyDocument);
    m_codeAssistant->configure(this);
    m_autoCompleter->setTabSettings(codeDocument()->tabSettings());

    m_toolBar->adjustSize();
    updateViewportMargins();
    setWordWrapMode(QTextOption::NoWrap);
    updateHighlights();
    createToolBar();

    // That's how find results will be highlighted
    // Let's trig search on text editor after 4 seconds BUG
    //    QTimer::singleShot(4000, [=]{
    //        highlightSearchResultsSlot("swipeView", FindFlags(FindCaseSensitively | FindWholeWords));
    //    });
}

QmlCodeEditor::~QmlCodeEditor()
{
    delete m_codeAssistant;
}

void QmlCodeEditor::setCodeDocument(QmlCodeDocument* document)
{
    auto settings = TextEditorSettings::instance();
    auto documentLayout = qobject_cast<QPlainTextDocumentLayout*>(document->documentLayout());

    if (codeDocument()) {
        disconnect(codeDocument(), 0, this, 0);
        disconnect(this, 0, codeDocument()->documentLayout(), 0);
        disconnect(codeDocument()->documentLayout(), 0, this, 0);
        disconnect(codeDocument()->documentLayout(), 0, m_rowBar, 0);
        disconnect(settings, 0, codeDocument(), 0);
    }
    disconnect(settings, 0, this, 0);

    m_extraSelections.clear();
    QPlainTextEdit::setExtraSelections({});
    setDocument(document);
    setCursorWidth(2);
    m_toolBar->setDocument(document);

    connect(documentLayout, &QPlainTextDocumentLayout::updateBlock,
            this, &QmlCodeEditor::slotUpdateBlockNotify);

    //    connect(documentLayout, &QPlainTextDocumentLayout::updateExtraArea,
    //            m_rowBar, static_cast<void (QWidget::*)()>(&QWidget::update));

    connect(this, &QmlCodeEditor::requestBlockUpdate,
            documentLayout, &QPlainTextDocumentLayout::updateBlock);

    //    connect(documentLayout, &QPlainTextDocumentLayout::updateExtraArea,
    //            this, &QmlCodeEditor::scheduleUpdateHighlightScrollBar);

    //    connect(documentLayout, &QAbstractTextDocumentLayout::documentSizeChanged,
    //            this, &QmlCodeEditor::scheduleUpdateHighlightScrollBar);

    //    connect(documentLayout, &QAbstractTextDocumentLayout::update,
    //            this, &QmlCodeEditor::scheduleUpdateHighlightScrollBar);

    connect(document, &QmlCodeDocument::contentsChange,
            this, &QmlCodeEditor::editorContentsChange);

    //    connect(document, &QmlCodeDocument::aboutToReload,
    //            this, &QmlCodeEditor::documentAboutToBeReloaded);

    //    connect(document, &QmlCodeDocument::reloadFinished,
    //            this, &QmlCodeEditor::documentReloadFinished);

    connect(document, &QmlCodeDocument::tabSettingsChanged,
            this, [this] () {
        updateTabStops();
        m_autoCompleter->setTabSettings(codeDocument()->tabSettings());
    });

    applyFontSettingsDelayed();

    //    connect(document, &QmlCodeDocument::markRemoved,
    //            this, &QmlCodeEditor::markRemoved);

    connect(document, &QmlCodeDocument::semanticInfoUpdated,
            this, &QmlCodeEditor::semanticInfoUpdated);
    connect(document, &QmlCodeDocument::updateCodeWarnings,
            this, &QmlCodeEditor::updateCodeWarnings);

    updateViewportMargins();

    // Apply current settings
    document->setTabSettings(settings->codeStyle()->tabSettings()); // also set through code style ???
    document->setTypingSettings(TypingSettings()); // FIXME
    document->setStorageSettings(StorageSettings()); // FIXME
    //    setBehaviorSettings(settings->behaviorSettings());
    //    setMarginSettings(settings->marginSettings());
    //    setDisplaySettings(settings->displaySettings());
    setCompletionSettings(settings->completionSettings());
    //    setExtraEncodingSettings(settings->extraEncodingSettings());
    setCodeStyle(settings->codeStyle(m_tabSettingsId));

    // Connect to settings change signals
    connect(settings, &TextEditorSettings::typingSettingsChanged,
            document, &QmlCodeDocument::setTypingSettings);
    connect(settings, &TextEditorSettings::storageSettingsChanged,
            document, &QmlCodeDocument::setStorageSettings);
    //    connect(settings, &TextEditorSettings::behaviorSettingsChanged,
    //            this, &QmlCodeEditor::setBehaviorSettings);
    //    connect(settings, &TextEditorSettings::marginSettingsChanged,
    //            this, &QmlCodeEditor::setMarginSettings);
    //    connect(settings, &TextEditorSettings::displaySettingsChanged,
    //            this, &QmlCodeEditor::setDisplaySettings);
    connect(settings, &TextEditorSettings::completionSettingsChanged,
            this, &QmlCodeEditor::setCompletionSettings);
    //    connect(settings, &TextEditorSettings::extraEncodingSettingsChanged,
    //            this, &QmlCodeEditor::setExtraEncodingSettings);

    update();
    emit documentChanged();
}

void QmlCodeEditor::setCodeStyle(ICodeStylePreferences *preferences)
{
    codeDocument()->indenter()->setCodeStylePreferences(preferences);
    if (m_codeStylePreferences) {
        disconnect(m_codeStylePreferences, &ICodeStylePreferences::currentTabSettingsChanged,
                   codeDocument(), &QmlCodeDocument::setTabSettings);
        disconnect(m_codeStylePreferences, &ICodeStylePreferences::currentValueChanged,
                   this, &QmlCodeEditor::slotCodeStyleSettingsChanged);
    }
    m_codeStylePreferences = preferences;
    if (m_codeStylePreferences) {
        connect(m_codeStylePreferences, &ICodeStylePreferences::currentTabSettingsChanged,
                codeDocument(), &QmlCodeDocument::setTabSettings);
        connect(m_codeStylePreferences, &ICodeStylePreferences::currentValueChanged,
                this, &QmlCodeEditor::slotCodeStyleSettingsChanged);
        codeDocument()->setTabSettings(m_codeStylePreferences->currentTabSettings());
        slotCodeStyleSettingsChanged(m_codeStylePreferences->currentValue());
    }
}

void QmlCodeEditor::slotUpdateBlockNotify(const QTextBlock &block)
{
    static bool blockRecursion = false;
    if (blockRecursion)
        return;
    blockRecursion = true;
    if (m_overlay->isVisible()) {
        /* an overlay might draw outside the block bounderies, force
           complete viewport update */
        viewport()->update();
    } else {
        if (block.previous().isValid() && block.userState() != block.previous().userState()) {
            /* The syntax highlighting state changes. This opens up for
           the possibility that the paragraph has braces that support
           code folding. In this case, do the save thing and also
           update the previous block, which might contain a fold
           box which now is invalid.*/
            emit requestBlockUpdate(block.previous());
        }
        if (!m_findScopeStart.isNull()) {
            if (block.position() < m_findScopeEnd.position()
                    && block.position() + block.length() >= m_findScopeStart.position()) {
                QTextBlock b = block.document()->findBlock(m_findScopeStart.position());
                do {
                    emit requestBlockUpdate(b);
                    b = b.next();
                } while (b.isValid() && b.position() < m_findScopeEnd.position());
            }
        }
    }
    blockRecursion = false;
}

void QmlCodeEditor::paintFindScope(const PaintEventData &data, QPainter &painter)
{
    if (m_findScopeStart.isNull())
        return;
    if (m_findScopeVerticalBlockSelectionFirstColumn >= 0) {
        QTextBlock block = data.block;
        QPointF offset = data.offset;
        while (block.isValid()) {

            QRectF blockBoundingRect = this->blockBoundingRect(block).translated(offset);

            if (blockBoundingRect.bottom() >= data.eventRect.top()
                    && blockBoundingRect.top() <= data.eventRect.bottom()) {

                if (block.position() >= m_findScopeStart.block().position()
                        && block.position() <= m_findScopeEnd.block().position()) {
                    QTextLayout *layout = block.layout();
                    QString text = block.text();
                    const TabSettings &ts = codeDocument()->tabSettings();
                    qreal spacew = fontMetrics().horizontalAdvance(QLatin1Char(' '));

                    int offset = 0;
                    int relativePos  =  ts.positionAtColumn(text,
                                                            m_findScopeVerticalBlockSelectionFirstColumn,
                                                            &offset);
                    QTextLine line = layout->lineForTextPosition(relativePos);
                    qreal x = line.cursorToX(relativePos) + offset * spacew;

                    int eoffset = 0;
                    int erelativePos  =  ts.positionAtColumn(text,
                                                             m_findScopeVerticalBlockSelectionLastColumn,
                                                             &eoffset);
                    QTextLine eline = layout->lineForTextPosition(erelativePos);
                    qreal ex = eline.cursorToX(erelativePos) + eoffset * spacew;

                    QRectF lineRect = line.naturalTextRect();
                    lineRect.moveTop(lineRect.top() + blockBoundingRect.top());
                    lineRect.setLeft(blockBoundingRect.left() + x);
                    if (line.lineNumber() == eline.lineNumber())
                        lineRect.setRight(blockBoundingRect.left() + ex);
                    painter.fillRect(lineRect, data.searchScopeFormat.background());

                    QColor lineCol = data.searchScopeFormat.foreground().color();
                    QPen pen = painter.pen();
                    painter.setPen(lineCol);
                    if (block == m_findScopeStart.block())
                        painter.drawLine(lineRect.topLeft(), lineRect.topRight());
                    if (block == m_findScopeEnd.block())
                        painter.drawLine(lineRect.bottomLeft(), lineRect.bottomRight());
                    painter.drawLine(lineRect.topLeft(), lineRect.bottomLeft());
                    painter.drawLine(lineRect.topRight(), lineRect.bottomRight());
                    painter.setPen(pen);
                }
            }
            offset.ry() += blockBoundingRect.height();

            if (offset.y() > data.viewportRect.height())
                break;

            block = nextVisibleBlock(block, data.doc);
        }
    } else {
        auto overlay = new TextEditor::Internal::TextEditorOverlay(this);
        overlay->addOverlaySelection(m_findScopeStart.position(),
                                     m_findScopeEnd.position(),
                                     data.searchScopeFormat.foreground().color(),
                                     data.searchScopeFormat.background().color(),
                                     TextEditor::Internal::TextEditorOverlay::ExpandBegin);
        overlay->setAlpha(false);
        overlay->paint(&painter, data.eventRect);
        delete overlay;
    }
}

bool QmlCodeEditor::inFindScope(int selectionStart, int selectionEnd)
{
    if (m_findScopeStart.isNull())
        return true; // no scope, everything is included
    if (selectionStart < m_findScopeStart.position())
        return false;
    if (selectionEnd > m_findScopeEnd.position())
        return false;
    if (m_findScopeVerticalBlockSelectionFirstColumn < 0)
        return true;
    QTextBlock block = document()->findBlock(selectionStart);
    if (block != document()->findBlock(selectionEnd))
        return false;
    QString text = block.text();
    const TabSettings &ts = codeDocument()->tabSettings();
    int startPosition = ts.positionAtColumn(text, m_findScopeVerticalBlockSelectionFirstColumn);
    int endPosition = ts.positionAtColumn(text, m_findScopeVerticalBlockSelectionLastColumn);
    if (selectionStart - block.position() < startPosition)
        return false;
    if (selectionEnd - block.position() > endPosition)
        return false;
    return true;
}

bool QmlCodeEditor::isValid() const
{
    return codeDocument() != m_initialEmptyDocument;
}

void QmlCodeEditor::discharge()
{
    m_toolBar->discharge();
    setNoDocsVisible(true);
    // FIXME
}

void QmlCodeEditor::slotCodeStyleSettingsChanged(const QVariant &)
{

}

void QmlCodeEditor::setLanguageSettingsId(Core::Id settingsId)
{
    m_tabSettingsId = settingsId;
}

Core::Id QmlCodeEditor::languageSettingsId() const
{
    return m_tabSettingsId;
}

void QmlCodeEditor::setCursorPosition(int pos)
{
    //    if (d->m_inBlockSelectionMode)
    //        d->disableBlockSelection(TextEditorWidgetPrivate::NoCursorUpdate);
    QTextCursor tc = textCursor();
    tc.setPosition(pos);
    setTextCursor(tc);
}

void QmlCodeEditor::replace(int length, const QString &string)
{
    QTextCursor tc = textCursor();
    tc.setPosition(tc.position() + length, QTextCursor::KeepAnchor);
    tc.insertText(string);
}

QRect QmlCodeEditor::cursorRect(int pos) const
{
    QTextCursor tc = textCursor();
    if (pos >= 0)
        tc.setPosition(pos);
    QRect result = cursorRect(tc);
    result.moveTo(viewport()->mapToGlobal(result.topLeft()));
    return result;
}

void QmlCodeEditor::setAutoCompleteSkipPosition(const QTextCursor &cursor)
{
    QTextCursor tc = cursor;
    // Create a selection of the next character but keep the current position, otherwise
    // the cursor would be removed from the list of automatically inserted text positions
    tc.movePosition(QTextCursor::NextCharacter);
    tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    autocompleterHighlight(tc);
}

void QmlCodeEditor::invokeAssist(AssistKind kind, IAssistProvider *provider)
{
    //    if (kind == QuickFix && d->m_snippetOverlay->isVisible()) {
    //        d->m_snippetOverlay->setVisible(false);
    //        d->m_snippetOverlay->mangle();
    //        d->m_snippetOverlay->clear();
    //    }

    bool previousMode = overwriteMode();
    setOverwriteMode(false);
    ensureCursorVisible();
    m_codeAssistant->invoke(kind, provider);
    setOverwriteMode(previousMode);
}

void QmlCodeEditor::abortAssist()
{
    m_codeAssistant->destroyContext();
}

void QmlCodeEditor::handleBackspaceKey()
{
    QTextCursor cursor = textCursor();
    QTC_ASSERT(!cursor.hasSelection(), return);

    const int pos = cursor.position();
    if (!pos)
        return;

    bool cursorWithinSnippet = false;
    //    if (m_snippetOverlay->isVisible()) {
    //        QTextCursor snippetCursor = cursor;
    //        snippetCursor.movePosition(QTextCursor::Left);
    //        cursorWithinSnippet = snippetCheckCursor(snippetCursor);
    //    }

    const TabSettings &tabSettings = codeDocument()->tabSettings();
    const TypingSettings &typingSettings = codeDocument()->typingSettings();

    if (typingSettings.m_autoIndent
            && !m_autoCompleteHighlightPos.isEmpty()
            && (m_autoCompleteHighlightPos.last() == cursor)
            && m_removeAutoCompletedText
            && m_autoCompleter->autoBackspace(cursor)) {
        return;
    }

    bool handled = false;
    if (typingSettings.m_smartBackspaceBehavior == TypingSettings::BackspaceNeverIndents) {
        if (cursorWithinSnippet)
            cursor.beginEditBlock();
        cursor.deletePreviousChar();
        handled = true;
    } else if (typingSettings.m_smartBackspaceBehavior == TypingSettings::BackspaceFollowsPreviousIndents) {
        QTextBlock currentBlock = cursor.block();
        int positionInBlock = pos - currentBlock.position();
        const QString blockText = currentBlock.text();
        if (cursor.atBlockStart() || tabSettings.firstNonSpace(blockText) < positionInBlock) {
            if (cursorWithinSnippet)
                cursor.beginEditBlock();
            cursor.deletePreviousChar();
            handled = true;
        } else {
            if (cursorWithinSnippet) {
                //                m_snippetOverlay->mangle();
                //                m_snippetOverlay->clear();
                cursorWithinSnippet = false;
            }
            int previousIndent = 0;
            const int indent = tabSettings.columnAt(blockText, positionInBlock);
            for (QTextBlock previousNonEmptyBlock = currentBlock.previous();
                 previousNonEmptyBlock.isValid();
                 previousNonEmptyBlock = previousNonEmptyBlock.previous()) {
                QString previousNonEmptyBlockText = previousNonEmptyBlock.text();
                if (previousNonEmptyBlockText.trimmed().isEmpty())
                    continue;
                previousIndent =
                        tabSettings.columnAt(previousNonEmptyBlockText,
                                             tabSettings.firstNonSpace(previousNonEmptyBlockText));
                if (previousIndent < indent) {
                    cursor.beginEditBlock();
                    cursor.setPosition(currentBlock.position(), QTextCursor::KeepAnchor);
                    cursor.insertText(tabSettings.indentationString(previousNonEmptyBlockText));
                    cursor.endEditBlock();
                    handled = true;
                    break;
                }
            }
        }
    } else if (typingSettings.m_smartBackspaceBehavior == TypingSettings::BackspaceUnindents) {
        const QChar c = codeDocument()->characterAt(pos - 1);
        if (!(c == QLatin1Char(' ') || c == QLatin1Char('\t'))) {
            if (cursorWithinSnippet)
                cursor.beginEditBlock();
            cursor.deletePreviousChar();
        } else {
            if (cursorWithinSnippet) {
                //                m_snippetOverlay->mangle();
                //                m_snippetOverlay->clear();
                cursorWithinSnippet = false;
            }
            unindent();
        }
        handled = true;
    }

    if (!handled) {
        if (cursorWithinSnippet)
            cursor.beginEditBlock();
        cursor.deletePreviousChar();
    }

    if (cursorWithinSnippet) {
        cursor.endEditBlock();
        //        m_snippetOverlay->updateEquivalentSelections(cursor);
    }

    setTextCursor(cursor);
}

void QmlCodeEditor::handleHomeKey(bool anchor)
{
    QTextCursor cursor = textCursor();
    QTextCursor::MoveMode mode = QTextCursor::MoveAnchor;

    if (anchor)
        mode = QTextCursor::KeepAnchor;

    const int initpos = cursor.position();
    int pos = cursor.block().position();
    QChar character = codeDocument()->characterAt(pos);
    const QLatin1Char tab = QLatin1Char('\t');

    while (character == tab || character.category() == QChar::Separator_Space) {
        ++pos;
        if (pos == initpos)
            break;
        character = codeDocument()->characterAt(pos);
    }

    // Go to the start of the block when we're already at the start of the text
    if (pos == initpos)
        pos = cursor.block().position();

    cursor.setPosition(pos, mode);
    setTextCursor(cursor);
}

bool QmlCodeEditor::camelCaseLeft(QTextCursor& cursor, QTextCursor::MoveMode mode)
{
    int state = 0;
    enum Input {
        Input_U,
        Input_l,
        Input_underscore,
        Input_space,
        Input_other
    };

    if (!cursor.movePosition(QTextCursor::Left, mode))
        return false;

    forever {
        QChar c = codeDocument()->characterAt(cursor.position());
        Input input = Input_other;
        if (c.isUpper())
            input = Input_U;
        else if (c.isLower() || c.isDigit())
            input = Input_l;
        else if (c == QLatin1Char('_'))
            input = Input_underscore;
        else if (c.isSpace() && c != QChar::ParagraphSeparator)
            input = Input_space;
        else
            input = Input_other;

        switch (state) {
        case 0:
            switch (input) {
            case Input_U:
                state = 1;
                break;
            case Input_l:
                state = 2;
                break;
            case Input_underscore:
                state = 3;
                break;
            case Input_space:
                state = 4;
                break;
            default:
                cursor.movePosition(QTextCursor::Right, mode);
                return cursor.movePosition(QTextCursor::WordLeft, mode);
            }
            break;
        case 1:
            switch (input) {
            case Input_U:
                break;
            default:
                cursor.movePosition(QTextCursor::Right, mode);
                return true;
            }
            break;

        case 2:
            switch (input) {
            case Input_U:
                return true;
            case Input_l:
                break;
            default:
                cursor.movePosition(QTextCursor::Right, mode);
                return true;
            }
            break;
        case 3:
            switch (input) {
            case Input_underscore:
                break;
            case Input_U:
                state = 1;
                break;
            case Input_l:
                state = 2;
                break;
            default:
                cursor.movePosition(QTextCursor::Right, mode);
                return true;
            }
            break;
        case 4:
            switch (input) {
            case Input_space:
                break;
            case Input_U:
                state = 1;
                break;
            case Input_l:
                state = 2;
                break;
            case Input_underscore:
                state = 3;
                break;
            default:
                cursor.movePosition(QTextCursor::Right, mode);
                if (cursor.positionInBlock() == 0)
                    return true;
                return cursor.movePosition(QTextCursor::WordLeft, mode);
            }
        }

        if (!cursor.movePosition(QTextCursor::Left, mode))
            return true;
    }
}

bool QmlCodeEditor::camelCaseRight(QTextCursor& cursor, QTextCursor::MoveMode mode)
{
    int state = 0;
    enum Input {
        Input_U,
        Input_l,
        Input_underscore,
        Input_space,
        Input_other
    };

    forever {
        QChar c = codeDocument()->characterAt(cursor.position());
        Input input = Input_other;
        if (c.isUpper())
            input = Input_U;
        else if (c.isLower() || c.isDigit())
            input = Input_l;
        else if (c == QLatin1Char('_'))
            input = Input_underscore;
        else if (c.isSpace() && c != QChar::ParagraphSeparator)
            input = Input_space;
        else
            input = Input_other;

        switch (state) {
        case 0:
            switch (input) {
            case Input_U:
                state = 4;
                break;
            case Input_l:
                state = 1;
                break;
            case Input_underscore:
                state = 6;
                break;
            default:
                return cursor.movePosition(QTextCursor::WordRight, mode);
            }
            break;
        case 1:
            switch (input) {
            case Input_U:
                return true;
            case Input_l:
                break;
            case Input_underscore:
                state = 6;
                break;
            case Input_space:
                state = 7;
                break;
            default:
                return true;
            }
            break;
        case 2:
            switch (input) {
            case Input_U:
                break;
            case Input_l:
                cursor.movePosition(QTextCursor::Left, mode);
                return true;
            case Input_underscore:
                state = 6;
                break;
            case Input_space:
                state = 7;
                break;
            default:
                return true;
            }
            break;
        case 4:
            switch (input) {
            case Input_U:
                state = 2;
                break;
            case Input_l:
                state = 1;
                break;
            case Input_underscore:
                state = 6;
                break;
            case Input_space:
                state = 7;
                break;
            default:
                return true;
            }
            break;
        case 6:
            switch (input) {
            case Input_underscore:
                break;
            case Input_space:
                state = 7;
                break;
            default:
                return true;
            }
            break;
        case 7:
            switch (input) {
            case Input_space:
                break;
            default:
                return true;
            }
            break;
        }
        cursor.movePosition(QTextCursor::Right, mode);
    }
}

void QmlCodeEditor::setCompletionSettings(const CompletionSettings &completionSettings)
{
    m_autoCompleter->setAutoInsertBracketsEnabled(completionSettings.m_autoInsertBrackets);
    m_autoCompleter->setSurroundWithBracketsEnabled(completionSettings.m_surroundingAutoBrackets);
    m_autoCompleter->setAutoInsertQuotesEnabled(completionSettings.m_autoInsertQuotes);
    m_autoCompleter->setSurroundWithQuotesEnabled(completionSettings.m_surroundingAutoQuotes);
    m_animateAutoComplete = completionSettings.m_animateAutoComplete;
    m_highlightAutoComplete = completionSettings.m_highlightAutoComplete;
    m_skipAutoCompletedText = completionSettings.m_skipAutoCompletedText;
    m_removeAutoCompletedText = completionSettings.m_autoRemove;
}

QmlCodeDocument* QmlCodeEditor::codeDocument() const
{
    return qobject_cast<QmlCodeDocument*>(document());
}

TextEditor::CompletionAssistProvider* QmlCodeEditor::completionAssistProvider()
{
    return m_completionAssistProvider;
}

TextEditor::IAssistProvider* QmlCodeEditor::quickFixAssistProvider()
{
    return nullptr;
}

TextEditor::AssistInterface* QmlCodeEditor::createAssistInterface(TextEditor::AssistKind assistKind,
                                                                  TextEditor::AssistReason reason)
{
    using namespace TextEditor;
    using namespace QmlJSEditor;

    if (assistKind == Completion) {
        return new QmlJSCompletionAssistInterface(document(),
                                                  textCursor().position(),
                                                  codeDocument()->filePath(),
                                                  reason,
                                                  codeDocument()->semanticInfo());
    }/* else if (assistKind == QuickFix) { // We don't want Refactor feature
        return new QmlJSQuickFixAssistInterface(const_cast<QmlCodeEditor *>(this), reason);
    }*/
    return 0;
}

void QmlCodeEditor::updateViewportMargins()
{
    if (isLeftToRight())
        setViewportMargins(m_rowBar->calculatedWidth(), m_toolBar->height() - 1, 1, 0);
    else
        setViewportMargins(1, m_toolBar->height() - 1, m_rowBar->calculatedWidth(), 0);
}

void QmlCodeEditor::updateRowBar(const QRect& rect, int dy)
{
    if (dy) {
        m_rowBar->update();
    } else if (rect.width() > 4) { // wider than cursor width, not just cursor blinking
        m_rowBar->update(0, rect.y(), m_rowBar->width(), rect.height());
        if (!m_searchExpr.isEmpty()) {
            const int m = m_searchResultOverlay->dropShadowWidth();
            viewport()->update(rect.adjusted(-m, -m, m, m));
        }
    }

    if (rect.contains(viewport()->rect()))
        updateViewportMargins();
}

void QmlCodeEditor::editorContentsChange(int /*position*/, int /*charsRemoved*/, int /*charsAdded*/)
{
    if (m_bracketsAnimator)
        m_bracketsAnimator->finish();
}

void QmlCodeEditor::paintOverlays(const PaintEventData& data, QPainter& painter) const
{
    // draw the overlays, but only if we do not have a find scope, otherwise the
    // view becomes too noisy.
    if (m_findScopeStart.isNull()) {
        if (m_overlay->isVisible())
            m_overlay->paint(&painter, data.eventRect);

        //        if (m_snippetOverlay->isVisible())
        //            m_snippetOverlay->paint(&painter, data.eventRect);

        if (!m_refactorOverlay->isEmpty())
            m_refactorOverlay->paint(&painter, data.eventRect);
    }

    if (!m_searchResultOverlay->isEmpty()) {
        m_searchResultOverlay->paint(&painter, data.eventRect);
        m_searchResultOverlay->clear();
    }
}

void QmlCodeEditor::setFindScope(const QTextCursor &start, const QTextCursor &end,
                                 int verticalBlockSelectionFirstColumn,
                                 int verticalBlockSelectionLastColumn)
{
    if (start != m_findScopeStart
            || end != m_findScopeEnd
            || verticalBlockSelectionFirstColumn != m_findScopeVerticalBlockSelectionFirstColumn
            || verticalBlockSelectionLastColumn != m_findScopeVerticalBlockSelectionLastColumn) {
        m_findScopeStart = start;
        m_findScopeEnd = end;
        m_findScopeVerticalBlockSelectionFirstColumn = verticalBlockSelectionFirstColumn;
        m_findScopeVerticalBlockSelectionLastColumn = verticalBlockSelectionLastColumn;
        viewport()->update();
        //        highlightSearchResultsInScrollBar();
    }
}

bool QmlCodeEditor::viewportEvent(QEvent *event)
{
    //    m_contentsChanged = false;
    if (event->type() == QEvent::ToolTip) {
        if (QApplication::keyboardModifiers() & Qt::ControlModifier
                || (!(QApplication::keyboardModifiers() & Qt::ShiftModifier)
                    && false /* FIXME: m_behaviorSettings.m_constrainHoverTooltips*/)) {
            // Tooltips should be eaten when either control is pressed (so they don't get in the
            // way of code navigation) or if they are in constrained mode and shift is not pressed.
            return true;
        }
        const QHelpEvent *he = static_cast<QHelpEvent*>(event);
        const QPoint &pos = he->pos();

        RefactorMarker refactorMarker = m_refactorOverlay->markerAt(pos);
        if (refactorMarker.isValid() && !refactorMarker.tooltip.isEmpty()) {
            ToolTip::show(he->globalPos(), refactorMarker.tooltip,
                          viewport(), QString(), refactorMarker.rect);
            return true;
        }

        QTextCursor tc = cursorForPosition(pos);
        QTextBlock block = tc.block();
        QTextLine line = block.layout()->lineForTextPosition(tc.positionInBlock());
        QTC_CHECK(line.isValid());
        // Only handle tool tip for text cursor if mouse is within the block for the text cursor,
        // and not if the mouse is e.g. in the empty space behind a short line.
        if (line.isValid()) {
            if (pos.x() <= blockBoundingGeometry(block).left() + line.naturalTextRect().right()) {
                processTooltipRequest(tc);
                return true;
            }/* else if (processAnnotaionTooltipRequest(block, pos)) {
                return true;
            }*/
            ToolTip::hide();
        }
    }
    return QPlainTextEdit::viewportEvent(event);
}

void QmlCodeEditor::mouseReleaseEvent(QMouseEvent *e)
{
    if (/*mouseNavigationEnabled()
                                                                                                                                                                            && */m_linkPressed
            && e->modifiers() & Qt::ControlModifier
            && !(e->modifiers() & Qt::ShiftModifier)
            && e->button() == Qt::LeftButton
            ) {

        //        EditorManager::addCurrentPositionToNavigationHistory();

        findLinkAt(textCursor(),
                   [self = QPointer<QmlCodeEditor>(this)](const Link &symbolLink) {
            if (self && self->openLink(symbolLink))
                self->clearLink();
        });
    }

    //    if (!HostOsInfo::isLinuxHost() && handleForwardBackwardMouseButtons(e))
    //        return;

    QPlainTextEdit::mouseReleaseEvent(e);
}

void QmlCodeEditor::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (e->modifiers() == Qt::AltModifier) {
            const QTextCursor &cursor = cursorForPosition(e->pos());
            int column = codeDocument()->tabSettings().columnAt(
                        cursor.block().text(), cursor.positionInBlock());
            if (cursor.positionInBlock() == cursor.block().length()-1)
                column += (e->pos().x() - cursorRect(cursor).center().x()) / fontMetrics().horizontalAdvance(QLatin1Char(' '));
            int block = cursor.blockNumber();
            if (block == blockCount() - 1)
                block += (e->pos().y() - cursorRect(cursor).center().y()) / fontMetrics().lineSpacing();
            //            if (m_inBlockSelectionMode) { BUG
            //                m_blockSelection.positionBlock = block;
            //                m_blockSelection.positionColumn = column;

            //                doSetTextCursor(m_blockSelection.selection(m_document.data()), true);
            //                viewport()->update();
            //            } else {
            //                  enableBlockSelection(block, column, block, column);
            //            }
        } else {
            //            if (m_inBlockSelectionMode)
            //                disableBlockSelection(QmlCodeEditor::NoCursorUpdate);

            QTextBlock foldedBlock = foldedBlockAt(e->pos());
            if (foldedBlock.isValid()) {
                m_rowBar->bracketBand()->toggleFold(e->pos());
                viewport()->setCursor(Qt::IBeamCursor);
            }

            RefactorMarker refactorMarker = m_refactorOverlay->markerAt(e->pos());
            if (refactorMarker.isValid()) {
                onRefactorMarkerClicked(refactorMarker);
            } else {
                requestUpdateLink(e, true);

                if (m_currentLink.hasValidLinkText())
                    m_linkPressed = true;
            }
        }
    } else if (e->button() == Qt::RightButton) {
        int eventCursorPosition = cursorForPosition(e->pos()).position();
        if (eventCursorPosition < textCursor().selectionStart()
                || eventCursorPosition > textCursor().selectionEnd()) {
            setTextCursor(cursorForPosition(e->pos()));
        }
    }

    /*if (HostOsInfo::isLinuxHost() && handleForwardBackwardMouseButtons(e))
        return;*/

    QPlainTextEdit::mousePressEvent(e);
}

void QmlCodeEditor::onRefactorMarkerClicked(const RefactorMarker &marker)
{
    if (marker.data.canConvert<QtQuickToolbarMarker>())
        showContextPane();
}

void QmlCodeEditor::clearVisibleFoldedBlock()
{
    if (m_suggestedVisibleFoldedBlockNumber) {
        m_suggestedVisibleFoldedBlockNumber = -1;
        m_foldedBlockTimer.stop();
    }
    if (m_visibleFoldedBlockNumber >= 0) {
        m_visibleFoldedBlockNumber = -1;
        viewport()->update();
    }
}

void QmlCodeEditor::timerEvent(QTimerEvent* e)
{
    /*if (e->timerId() == m_autoScrollTimer.timerId()) {
        const QPoint globalPos = QCursor::pos();
        const QPoint pos = m_rowBar->mapFromGlobal(globalPos);
        QRect visible = m_rowBar->rect();
        verticalScrollBar()->triggerAction( pos.y() < visible.center().y() ?
                                            QAbstractSlider::SliderSingleStepSub
                                            : QAbstractSlider::SliderSingleStepAdd);
        QMouseEvent ev(QEvent::MouseMove, pos, globalPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//        extraAreaMouseEvent(&ev); BUG ??
        int delta = qMax(pos.y() - visible.top(), visible.bottom() - pos.y()) - visible.height();
        if (delta < 7)
            delta = 7;
        int timeout = 4900 / (delta * delta);
        m_autoScrollTimer.start(timeout, this);

    } else */if (e->timerId() == m_foldedBlockTimer.timerId()) {
        m_visibleFoldedBlockNumber = m_suggestedVisibleFoldedBlockNumber;
        m_suggestedVisibleFoldedBlockNumber = -1;
        m_foldedBlockTimer.stop();
        viewport()->update();
    } else if (e->timerId() == m_cursorFlashTimer.timerId()) {
        //        m_cursorVisible = !m_cursorVisible;
        viewport()->update();
    }
    QPlainTextEdit::timerEvent(e);
}

QTextBlock QmlCodeEditor::foldedBlockAt(const QPoint &pos, QRect *box) const
{
    QPointF offset = contentOffset();
    QTextBlock block = firstVisibleBlock();
    qreal top = blockBoundingGeometry(block).translated(offset).top();
    qreal bottom = top + blockBoundingRect(block).height();

    int viewportHeight = viewport()->height();

    while (block.isValid() && top <= viewportHeight) {
        QTextBlock nextBlock = block.next();
        if (block.isVisible() && bottom >= 0 && replacementVisible(block.blockNumber())) {
            if (nextBlock.isValid() && !nextBlock.isVisible()) {
                QTextLayout *layout = block.layout();
                QTextLine line = layout->lineAt(layout->lineCount()-1);
                QRectF lineRect = line.naturalTextRect().translated(offset.x(), top);
                lineRect.adjust(0, 0, -1, -1);

                QString replacement = QLatin1String(" {") + foldReplacementText(block)
                        + QLatin1String("}; ");

                QRectF collapseRect(lineRect.right() + 12,
                                    lineRect.top(),
                                    fontMetrics().horizontalAdvance(replacement),
                                    lineRect.height());
                if (collapseRect.contains(pos)) {
                    QTextBlock result = block;
                    if (box)
                        *box = collapseRect.toAlignedRect();
                    return result;
                } else {
                    block = nextBlock;
                    while (nextBlock.isValid() && !nextBlock.isVisible()) {
                        block = nextBlock;
                        nextBlock = block.next();
                    }
                }
            }
        }

        block = nextBlock;
        top = bottom;
        bottom = top + blockBoundingRect(block).height();
    }
    return QTextBlock();
}

void QmlCodeEditor::mouseMoveEvent(QMouseEvent *e)
{
    requestUpdateLink(e);

    if (e->buttons() == Qt::NoButton) { //BUG
        const QTextBlock collapsedBlock = foldedBlockAt(e->pos());
        const int blockNumber = collapsedBlock.next().blockNumber();
        if (blockNumber < 0) {
            clearVisibleFoldedBlock();
        } else if (blockNumber != m_visibleFoldedBlockNumber) {
            m_suggestedVisibleFoldedBlockNumber = blockNumber;
            m_foldedBlockTimer.start(40, this);
        }

        const RefactorMarker refactorMarker = m_refactorOverlay->markerAt(e->pos());

        // Update the mouse cursor
        if ((collapsedBlock.isValid() || refactorMarker.isValid()) && !m_mouseOnFoldedMarker) {
            m_mouseOnFoldedMarker = true;
            viewport()->setCursor(Qt::PointingHandCursor);
        } else if (!collapsedBlock.isValid() && !refactorMarker.isValid() && m_mouseOnFoldedMarker) {
            m_mouseOnFoldedMarker = false;
            viewport()->setCursor(Qt::IBeamCursor);
        }
    } else {
        QPlainTextEdit::mouseMoveEvent(e);

        if (e->modifiers() & Qt::AltModifier) {
            //            if (!m_inBlockSelectionMode) {
            //                if (textCursor().hasSelection()) {
            //                    enableBlockSelection(textCursor());
            //                } else {
            //                    const QTextCursor &cursor = cursorForPosition(e->pos());
            //                    int column = m_document->tabSettings().columnAt(
            //                                cursor.block().text(), cursor.positionInBlock());
            //                    if (cursor.positionInBlock() == cursor.block().length()-1)
            //                        column += (e->pos().x() - cursorRect().center().x()) / fontMetrics().horizontalAdvance(QLatin1Char(' '));
            //                    int block = cursor.blockNumber();
            //                    if (block == blockCount() - 1)
            //                        block += (e->pos().y() - cursorRect().center().y()) / fontMetrics().lineSpacing();
            //                    enableBlockSelection(block, column, block, column);
            //                }
            //            } else {
            //                const QTextCursor &cursor = textCursor();

            //                // get visual column
            //                int column = m_document->tabSettings().columnAt(
            //                            cursor.block().text(), cursor.positionInBlock());
            //                if (cursor.positionInBlock() == cursor.block().length()-1)
            //                    column += (e->pos().x() - cursorRect().center().x()) / fontMetrics().horizontalAdvance(QLatin1Char(' '));

            //                m_blockSelection.positionBlock = cursor.blockNumber();
            //                m_blockSelection.positionColumn = column;

            //                doSetTextCursor(m_blockSelection.selection(m_document.data()), true);
            //                viewport()->update();
            //            }
        }/* else if (m_inBlockSelectionMode) {
            disableBlockSelection(QmlCodeEditor::CursorUpdateKeepSelection);
        }*/
    }
    if (viewport()->cursor().shape() == Qt::BlankCursor)
        viewport()->setCursor(Qt::IBeamCursor);
}

void QmlCodeEditor::leaveEvent(QEvent *e)
{
    // Clear link emulation when the mouse leaves the editor
    clearLink();
    QPlainTextEdit::leaveEvent(e);
}

void QmlCodeEditor::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Control) {
        clearLink();
    } else if (e->key() == Qt::Key_Shift
               && false // FIXME: m_behaviorSettings.m_constrainHoverTooltips
               && ToolTip::isVisible()) {
        ToolTip::hide();
    } else if (e->key() == Qt::Key_Alt
               /*&& m_maybeFakeTooltipEvent*/) {
        //        m_maybeFakeTooltipEvent = false;
        processTooltipRequest(textCursor());
    }

    QPlainTextEdit::keyReleaseEvent(e);
}

void QmlCodeEditor::setVisibleFoldedBlockNumber(int visibleFoldedBlockNumber)
{
    if (visibleFoldedBlockNumber == m_visibleFoldedBlockNumber)
        return;
    m_visibleFoldedBlockNumber = visibleFoldedBlockNumber;
    viewport()->update(viewport()->visibleRegion());
}

void QmlCodeEditor::setExtraSelections(const QString& kind, const QList<QTextEdit::ExtraSelection> &selections)
{
    m_extraSelections[kind] = selections;

    if (kind == "CodeSemanticsSelection") {
        m_overlay->clear();
        foreach (const QTextEdit::ExtraSelection &selection, m_extraSelections[kind]) {
            m_overlay->addOverlaySelection(selection.cursor,
                                           selection.format.background().color(),
                                           selection.format.background().color(),
                                           TextEditor::Internal::TextEditorOverlay::LockSize);
        }
        m_overlay->setVisible(!m_overlay->isEmpty());
    } else {
        if (selections.isEmpty() && m_extraSelections[kind].isEmpty())
            return;
        QList<QTextEdit::ExtraSelection> all;
        for (auto i = m_extraSelections.constBegin(); i != m_extraSelections.constEnd(); ++i) {
            if (i.key() == "CodeSemanticsSelection")
                continue;
            all += i.value();
        }
        QPlainTextEdit::setExtraSelections(all);
    }
    viewport()->update(viewport()->visibleRegion());
}

QList<QTextEdit::ExtraSelection> QmlCodeEditor::extraSelections(const QString& kind) const
{
    return m_extraSelections.value(kind);
}

void QmlCodeEditor::applyFontSettings()
{
    m_fontSettingsNeedsApply = false;
    const QTextCharFormat textFormat = CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_TEXT);
    const QTextCharFormat selectionFormat = CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_SELECTION);
    const QTextCharFormat lineNumberFormat = CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_LINE_NUMBER);
    QFont font(textFormat.font());

    const QColor foreground = textFormat.foreground().color();
    const QColor background = textFormat.background().color();
    QPalette p = palette();
    p.setColor(QPalette::Text, foreground);
    p.setColor(QPalette::WindowText, foreground);
    p.setColor(QPalette::Base, background);
    p.setColor(QPalette::Highlight, (selectionFormat.background().style() != Qt::NoBrush) ?
                   selectionFormat.background().color() :
                   QApplication::palette().color(QPalette::Highlight));

    p.setBrush(QPalette::HighlightedText, selectionFormat.foreground());

    p.setBrush(QPalette::Inactive, QPalette::Highlight, p.highlight());
    p.setBrush(QPalette::Inactive, QPalette::HighlightedText, p.highlightedText());
    setPalette(p);
    viewport()->setPalette(p);
    viewport()->setFont(font);
    m_rowBar->setFont(font);
    updateTabStops(); // update tab stops, they depend on the font

    // Line numbers
    QPalette ep;
    ep.setColor(QPalette::Dark, lineNumberFormat.foreground().color());
    ep.setColor(QPalette::Window, lineNumberFormat.background().style() != Qt::NoBrush ?
                lineNumberFormat.background().color() : background);
    m_rowBar->setPalette(ep);

    updateViewportMargins();   // Adjust to new font width
    updateHighlights();

    if (!codeDocument()->isSemanticInfoOutdated())
        updateUses();
}

void QmlCodeEditor::slotCursorPositionChanged()
{
#if 0
    qDebug() << "block" << textCursor().blockNumber()+1
             << "brace depth:" << BaseTextDocumentLayout::braceDepth(textCursor().block())
             << "indent:" << BaseTextDocumentLayout::userData(textCursor().block())->foldingIndent();
#endif
    //    if (!m_contentsChanged && m_lastCursorChangeWasInteresting) { // BUG
    //        if (EditorManager::currentEditor() && EditorManager::currentEditor()->widget() == this)
    //            EditorManager::addCurrentPositionToNavigationHistory(m_tempNavigationState);
    //        m_lastCursorChangeWasInteresting = false;
    //    } else if (m_contentsChanged) {
    //        saveCurrentCursorPositionForNavigation();
    //    }

    updateHighlights();
}

void QmlCodeEditor::matchParentheses()
{
    if (isReadOnly())
        return;

    QTextCursor backwardMatch = textCursor();
    QTextCursor forwardMatch = textCursor();
    if (overwriteMode())
        backwardMatch.movePosition(QTextCursor::Right);
    const MatchType backwardMatchType = QmlCodeDocument::matchCursorBackward(&backwardMatch);
    const MatchType forwardMatchType = QmlCodeDocument::matchCursorForward(&forwardMatch);

    QList<QTextEdit::ExtraSelection> extraSelections;

    if (backwardMatchType == NoMatch && forwardMatchType == NoMatch) {
        setExtraSelections("ParenthesesMatchingSelection", extraSelections); // clear
        return;
    }

    const QTextCharFormat &matchFormat
            = CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_PARENTHESES);
    const QTextCharFormat &mismatchFormat
            = CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_PARENTHESES_MISMATCH);
    int animatePosition = -1;

    if (backwardMatch.hasSelection()) {
        QTextEdit::ExtraSelection sel;
        if (backwardMatchType == Mismatch) {
            sel.cursor = backwardMatch;
            sel.format = mismatchFormat;
            extraSelections.append(sel);
        } else {

            sel.cursor = backwardMatch;
            sel.format = matchFormat;

            sel.cursor.setPosition(backwardMatch.selectionStart());
            sel.cursor.setPosition(sel.cursor.position() + 1, QTextCursor::KeepAnchor);
            extraSelections.append(sel);

            if (/*m_displaySettings.m_animateMatchingParentheses && */sel.cursor.block().isVisible())
                animatePosition = backwardMatch.selectionStart();

            sel.cursor.setPosition(backwardMatch.selectionEnd());
            sel.cursor.setPosition(sel.cursor.position() - 1, QTextCursor::KeepAnchor);
            extraSelections.append(sel);
        }
    }

    if (forwardMatch.hasSelection()) {
        QTextEdit::ExtraSelection sel;
        if (forwardMatchType == Mismatch) {
            sel.cursor = forwardMatch;
            sel.format = mismatchFormat;
            extraSelections.append(sel);
        } else {

            sel.cursor = forwardMatch;
            sel.format = matchFormat;

            sel.cursor.setPosition(forwardMatch.selectionStart());
            sel.cursor.setPosition(sel.cursor.position() + 1, QTextCursor::KeepAnchor);
            extraSelections.append(sel);

            sel.cursor.setPosition(forwardMatch.selectionEnd());
            sel.cursor.setPosition(sel.cursor.position() - 1, QTextCursor::KeepAnchor);
            extraSelections.append(sel);

            if (/*m_displaySettings.m_animateMatchingParentheses && */sel.cursor.block().isVisible())
                animatePosition = forwardMatch.selectionEnd() - 1;
        }
    }


    if (animatePosition >= 0) {
        for (const QTextEdit::ExtraSelection &sel : this->extraSelections("ParenthesesMatchingSelection")) {
            if (sel.cursor.selectionStart() == animatePosition
                    || sel.cursor.selectionEnd() - 1 == animatePosition) {
                animatePosition = -1;
                break;
            }
        }
    }

    if (animatePosition >= 0) {
        cancelCurrentAnimations();// one animation is enough
        QPalette pal;
        pal.setBrush(QPalette::Text, matchFormat.foreground());
        pal.setBrush(QPalette::Base, matchFormat.background());
        QTextCursor cursor = textCursor();
        cursor.setPosition(animatePosition + 1);
        cursor.setPosition(animatePosition, QTextCursor::KeepAnchor);
        m_bracketsAnimator = new TextEditorAnimator(this);
        m_bracketsAnimator->init(cursor, font(), pal);
        connect(m_bracketsAnimator.data(), &TextEditorAnimator::updateRequest,
                this, &QmlCodeEditor::animateUpdate);
    }
    //    if (m_displaySettings.m_highlightMatchingParentheses)
    setExtraSelections("ParenthesesMatchingSelection", extraSelections);
}

void QmlCodeEditor::updateHighlights()
{
    if (m_parenthesesMatchingEnabled && hasFocus()) {
        // Delay update when no matching is displayed yet, to avoid flicker
        if (extraSelections("ParenthesesMatchingSelection").isEmpty()
                && m_bracketsAnimator == nullptr) {
            m_parenthesesMatchingTimer->start(50);
        } else {
            // when we uncheck "highlight matching parentheses"
            // we need clear current selection before viewport update
            // otherwise we get sticky highlighted parentheses
            //            if (!m_displaySettings.m_highlightMatchingParentheses)
            //                setExtraSelections("ParenthesesMatchingSelection", QList<QTextEdit::ExtraSelection>());

            // use 0-timer, not direct call, to give the syntax highlighter a chance
            // to update the parentheses information
            m_parenthesesMatchingTimer->start(0);
        }
    }

    if (m_highlightAutoComplete && !m_autoCompleteHighlightPos.isEmpty()) {
        QTimer::singleShot(0, this, [this](){
            const QTextCursor &cursor = textCursor();
            auto popAutoCompletion = [&]() {
                return !m_autoCompleteHighlightPos.isEmpty()
                        && m_autoCompleteHighlightPos.last() != cursor;
            };
            if ((/*!m_keepAutoCompletionHighlight*/ false && !hasFocus()) || popAutoCompletion()) {
                while (popAutoCompletion())
                    m_autoCompleteHighlightPos.pop_back();
                updateAutoCompleteHighlight();
            }
        });
    }

    updateCurrentLineHighlight();

    //        if (m_displaySettings.m_highlightBlocks) {
    //                QTextCursor cursor = textCursor();
    //                extraAreaHighlightFoldedBlockNumber = cursor.blockNumber(); // BUG
    //                m_highlightBlocksTimer.start(100);
    //        }
}

void QmlCodeEditor::updateCurrentLineHighlight()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (/*m_highlightCurrentLine*/ codeDocument()) {
        QTextEdit::ExtraSelection sel;
        sel.format.setBackground(CodeEditorSettings::fontColorsSettings()
                                 ->toTextCharFormat(C_CURRENT_LINE).background());
        sel.format.setProperty(QTextFormat::FullWidthSelection, true);
        sel.cursor = textCursor();
        sel.cursor.clearSelection();
        extraSelections.append(sel);
    }
    //    updateCurrentLineInScrollbar();

    setExtraSelections("CurrentLineSelection", extraSelections);

    // the extra area shows information for the entire current block, not just the currentline.
    // This is why we must force a bigger update region.
    //        int cursorBlockNumber = textCursor().blockNumber();
    //        if (cursorBlockNumber != m_cursorBlockNumber) {
    //            QPointF offset = contentOffset();
    //            QTextBlock block = document()->findBlockByNumber(m_cursorBlockNumber);
    //            if (block.isValid())
    //                m_extraArea->update(q->blockBoundingGeometry(block).translated(offset).toAlignedRect());
    //            block = q->document()->findBlockByNumber(cursorBlockNumber);
    //            if (block.isValid() && block.isVisible())
    //                m_extraArea->update(q->blockBoundingGeometry(block).translated(offset).toAlignedRect());
    //            m_cursorBlockNumber = cursorBlockNumber;
    //        }
    updateRowBar({}, 1);
}

void QmlCodeEditor::changeEvent(QEvent* e)
{
    QPlainTextEdit::changeEvent(e);
    if (e->type() == QEvent::ApplicationFontChange
            || e->type() == QEvent::FontChange) {
        updateViewportMargins();
    }
}

RowBar* QmlCodeEditor::rowBar() const
{
    return m_rowBar;
}

QmlCodeEditorToolBar* QmlCodeEditor::toolBar() const
{
    return m_toolBar;
}

RefactorMarkers QmlCodeEditor::refactorMarkers() const
{
    return m_refactorOverlay->markers();
}

void QmlCodeEditor::setRefactorMarkers(const RefactorMarkers &markers)
{
    foreach (const RefactorMarker &marker, m_refactorOverlay->markers())
        emit requestBlockUpdate(marker.cursor.block());
    m_refactorOverlay->setMarkers(markers);
    foreach (const RefactorMarker &marker, markers)
        emit requestBlockUpdate(marker.cursor.block());
}

void QmlCodeEditor::focusInEvent(QFocusEvent *e)
{
    QPlainTextEdit::focusInEvent(e);
    updateHighlights();
}

void QmlCodeEditor::focusOutEvent(QFocusEvent *e)
{
    QPlainTextEdit::focusOutEvent(e);
    if (viewport()->cursor().shape() == Qt::BlankCursor)
        viewport()->setCursor(Qt::IBeamCursor);
    updateHighlights();
}

void QmlCodeEditor::updateAutoCompleteHighlight()
{
    const QTextCharFormat &matchFormat
            = CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_AUTOCOMPLETE);

    QList<QTextEdit::ExtraSelection> extraSelections;
    for (const QTextCursor &cursor : qAsConst(m_autoCompleteHighlightPos)) {
        QTextEdit::ExtraSelection sel;
        sel.cursor = cursor;
        sel.format.setBackground(matchFormat.background());
        extraSelections.append(sel);
    }
    setExtraSelections("AutoCompleteSelection", extraSelections);
}

void QmlCodeEditor::setNoDocsVisible(bool visible)
{
    setReadOnly(visible);
    m_rowBar->setDisabled(visible);
    viewport()->setDisabled(visible);
    m_noDocsLabel->setVisible(visible);
    if (visible) {
        m_initialEmptyDocument->clear();
        m_initialEmptyDocument->setModified(false);
        m_initialEmptyDocument->clearUndoRedoStacks();
        setCodeDocument(m_initialEmptyDocument);
        textCursor().clearSelection();
        setTextCursor(QTextCursor());
        m_noDocsLabel->raise();
    }
}

void QmlCodeEditor::autocompleterHighlight(const QTextCursor &cursor)
{
    if ((!m_animateAutoComplete && !m_highlightAutoComplete)
            || isReadOnly() || !cursor.hasSelection()) {
        m_autoCompleteHighlightPos.clear();
    } else if (m_highlightAutoComplete) {
        m_autoCompleteHighlightPos.push_back(cursor);
    }
    if (m_animateAutoComplete) {
        const QTextCharFormat &matchFormat
                = CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_AUTOCOMPLETE);
        cancelCurrentAnimations();// one animation is enough
        QPalette pal;
        pal.setBrush(QPalette::Text, matchFormat.foreground());
        pal.setBrush(QPalette::Base, matchFormat.background());
        m_autocompleteAnimator = new TextEditorAnimator(this);
        m_autocompleteAnimator->init(cursor, font(), pal);
        connect(m_autocompleteAnimator.data(), &TextEditorAnimator::updateRequest,
                this, &QmlCodeEditor::animateUpdate);
    }
    updateAutoCompleteHighlight();
}

void QmlCodeEditor::updateAnimator(QPointer<TextEditorAnimator> animator, QPainter &painter)
{
    if (animator && animator->isRunning())
        animator->draw(&painter, cursorRect(animator->cursor()).topLeft());
}

void QmlCodeEditor::cancelCurrentAnimations()
{
    if (m_autocompleteAnimator)
        m_autocompleteAnimator->finish();
    if (m_bracketsAnimator)
        m_bracketsAnimator->finish();
}

void QmlCodeEditor::convertPosition(int pos, int* line, int* column) const
{
    Text::convertPosition(document(), pos, line, column);
}

QRegion QmlCodeEditor::translatedLineRegion(int lineStart, int lineEnd) const
{
    QRegion region;
    for (int i = lineStart ; i <= lineEnd; i++) {
        QTextBlock block = document()->findBlockByNumber(i);
        QPoint topLeft = blockBoundingGeometry(block).translated(contentOffset()).topLeft().toPoint();

        if (block.isValid()) {
            QTextLayout *layout = block.layout();

            for (int i = 0; i < layout->lineCount();i++) {
                QTextLine line = layout->lineAt(i);
                region += line.naturalTextRect().translated(topLeft).toRect();
            }
        }
    }
    return region;
}

int QmlCodeEditor::position(TextPositionOperation posOp, int at) const
{
    QTextCursor tc = textCursor();

    if (at != -1)
        tc.setPosition(at);

    if (posOp == CurrentPosition)
        return tc.position();

    switch (posOp) {
    case EndOfLinePosition:
        tc.movePosition(QTextCursor::EndOfLine);
        return tc.position();
    case StartOfLinePosition:
        tc.movePosition(QTextCursor::StartOfLine);
        return tc.position();
    case AnchorPosition:
        if (tc.hasSelection())
            return tc.anchor();
        break;
    case EndOfDocPosition:
        tc.movePosition(QTextCursor::End);
        return tc.position();
    default:
        break;
    }

    return -1;
}

void QmlCodeEditor::addHoverHandler(BaseHoverHandler* handler)
{
    m_hoverHandlers.append(handler);
}

QString QmlCodeEditor::extraSelectionTooltip(int pos) const
{
    foreach (const QList<QTextEdit::ExtraSelection> &sel, m_extraSelections) {
        for (const QTextEdit::ExtraSelection &s : sel) {
            if (s.cursor.selectionStart() <= pos
                    && s.cursor.selectionEnd() >= pos
                    && !s.format.toolTip().isEmpty())
                return s.format.toolTip();
        }
    }
    return QString();
}

void QmlCodeEditor::processTooltipRequest(const QTextCursor &c)
{
    const QPoint toolTipPoint = toolTipPosition(c);
    bool handled = false;
    //    emit tooltipOverrideRequested(q, toolTipPoint, c.position(), &handled);
    if (handled)
        return;

    if (m_hoverHandlers.isEmpty()) {
        //        emit tooltipRequested(toolTipPoint, c.position());
        return;
    }

    m_hoverHandlerRunner->startChecking(c, toolTipPoint);
}

QPoint QmlCodeEditor::toolTipPosition(const QTextCursor &c) const
{
    const QPoint cursorPos = mapToGlobal(cursorRect(c).bottomRight() + QPoint(1,1));
    return cursorPos + QPoint(/*m_extraArea->*/m_rowBar->width(), HostOsInfo::isWindowsHost() ? -24 : -16);
}

void QmlCodeEditor::drawCollapsedBlockPopup(QPainter &painter,
                                            const QTextBlock &block,
                                            QPointF offset,
                                            const QRect &clip)
{
    if (!block.isValid())
        return;

    int margin = int(block.document()->documentMargin());
    qreal maxWidth = 0;
    qreal blockHeight = 0;
    QTextBlock b = block;

    while (!b.isVisible()) {
        b.setVisible(true); // make sure block bounding rect works
        QRectF r = blockBoundingRect(b).translated(offset);

        QTextLayout *layout = b.layout();
        for (int i = layout->lineCount()-1; i >= 0; --i)
            maxWidth = qMax(maxWidth, layout->lineAt(i).naturalTextWidth() + 2*margin);

        blockHeight += r.height();

        b.setVisible(false); // restore previous state
        b.setLineCount(0); // restore 0 line count for invisible block
        b = b.next();
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.translate(.5, .5);
    QBrush brush = palette().base();
    const QTextCharFormat &ifdefedOutFormat
            = CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_DISABLED_CODE);
    if (ifdefedOutFormat.hasProperty(QTextFormat::BackgroundBrush))
        brush = ifdefedOutFormat.background();
    painter.setBrush(brush);
    painter.drawRoundedRect(QRectF(offset.x(),
                                   offset.y(),
                                   maxWidth, blockHeight).adjusted(0, 0, 0, 0), 3, 3);
    painter.restore();

    QTextBlock end = b;
    b = block;
    while (b != end) {
        b.setVisible(true); // make sure block bounding rect works
        QRectF r = blockBoundingRect(b).translated(offset);
        QTextLayout *layout = b.layout();
        QVector<QTextLayout::FormatRange> selections;
        layout->draw(&painter, offset, selections, clip);

        b.setVisible(false); // restore previous state
        b.setLineCount(0); // restore 0 line count for invisible block
        offset.ry() += r.height();
        b = b.next();
    }
}

void QmlCodeEditor::paintSearchResultOverlay(const PaintEventData &data,
                                             QPainter &painter)
{
    m_searchResultOverlay->clear();

    if (m_searchExpr.isEmpty())
        return;

    const int margin = 5;
    QTextBlock block = data.block;
    QPointF offset = data.offset;
    while (block.isValid()) {
        QRectF blockBoundingRect = this->blockBoundingRect(block).translated(offset);

        if (blockBoundingRect.bottom() >= data.eventRect.top() - margin
                && blockBoundingRect.top() <= data.eventRect.bottom() + margin) {
            highlightSearchResults(block, m_searchResultOverlay);
        }
        offset.ry() += blockBoundingRect.height();

        if (offset.y() > data.viewportRect.height() + margin)
            break;

        block = nextVisibleBlock(block, data.doc);
    }

    m_searchResultOverlay->fill(&painter,
                                data.searchResultFormat.background().color(),
                                data.eventRect);
}

QTextBlock QmlCodeEditor::nextVisibleBlock(const QTextBlock &block,
                                           const QTextDocument *doc) const
{
    QTextBlock nextVisibleBlock = block.next();
    if (!nextVisibleBlock.isVisible()) {
        // invisible blocks do have zero line count
        nextVisibleBlock = doc->findBlockByLineNumber(nextVisibleBlock.firstLineNumber());
        // paranoia in case our code somewhere did not set the line count
        // of the invisible block to 0
        while (nextVisibleBlock.isValid() && !nextVisibleBlock.isVisible())
            nextVisibleBlock = nextVisibleBlock.next();
    }
    return nextVisibleBlock;
}

void QmlCodeEditor::highlightSearchResults(const QTextBlock &block,
                                           TextEditor::Internal::TextEditorOverlay *overlay)
{
    if (m_searchExpr.isEmpty())
        return;

    int blockPosition = block.position();

    QTextCursor cursor = textCursor();
    QString text = block.text();
    text.replace(QChar::Nbsp, QLatin1Char(' '));
    int idx = -1;
    int l = 1;

    while (idx < text.length()) {
        idx = m_searchExpr.indexIn(text, idx + l);
        if (idx < 0)
            break;
        l = m_searchExpr.matchedLength();
        if (l == 0)
            break;
        if ((m_findFlags & FindWholeWords)
                && ((idx && text.at(idx-1).isLetterOrNumber())
                    || (idx + l < text.length() && text.at(idx + l).isLetterOrNumber())))
            continue;

        if (!inFindScope(blockPosition + idx, blockPosition + idx + l))
            continue;

        const QTextCharFormat &searchResultFormat
                = CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_SEARCH_RESULT);
        overlay->addOverlaySelection(blockPosition + idx,
                                     blockPosition + idx + l,
                                     searchResultFormat.background().color().darker(120),
                                     QColor(),
                                     (idx == cursor.selectionStart() - blockPosition
                                      && idx + l == cursor.selectionEnd() - blockPosition)?
                                         TextEditor::Internal::TextEditorOverlay::DropShadow : 0);

    }
}

void QmlCodeEditor::searchResultsReady(int /*beginIndex*/, int /*endIndex*/)
{
    //    QVector<SearchResult> results;
    //    for (int index = beginIndex; index < endIndex; ++index) {
    //        foreach (Utils::FileSearchResult result, m_searchWatcher->resultAt(index)) {
    //            const QTextBlock &block = document()->findBlockByNumber(result.lineNumber - 1);
    //            const int matchStart = block.position() + result.matchStart;
    //            if (!inFindScope(matchStart, matchStart + result.matchLength))
    //                continue;
    //            results << SearchResult{matchStart, result.matchLength};
    //        }
    //    }
    //    m_searchResults << results;
    //    addSearchResultsToScrollBar(results);
}

void QmlCodeEditor::paintCurrentLineHighlight(const PaintEventData &data, QPainter &painter) const
{
    if (data.block != data.textCursorBlock)
        return;

    const QRectF blockRect = blockBoundingRect(data.block).translated(data.offset);
    QRectF lineRect = data.block.layout()->lineForTextPosition(data.textCursor.positionInBlock()).rect();
    lineRect.moveTop(lineRect.top() + blockRect.top());
    lineRect.setLeft(0);
    lineRect.setRight(data.viewportRect.width() - data.offset.x());
    QColor color = CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_CURRENT_LINE).background().color();
    // set alpha, otherwise we cannot see block highlighting and find scope underneath
    color.setAlpha(128);
    if (!data.isEditable && !data.eventRect.contains(lineRect.toRect())) {
        QRect updateRect = data.eventRect;
        updateRect.setLeft(0);
        updateRect.setRight(data.viewportRect.width() - int(data.offset.x()));
        viewport()->update(updateRect);
    }
    painter.fillRect(lineRect, color);
}

void QmlCodeEditor::highlightSearchResultsSlot(const QString &txt, FindFlags findFlags)
{
    if (m_searchExpr.pattern() == txt)
        return;
    m_searchExpr.setPattern(txt);
    m_searchExpr.setPatternSyntax((findFlags & FindRegularExpression) ?
                                      QRegExp::RegExp : QRegExp::FixedString);
    m_searchExpr.setCaseSensitivity((findFlags & FindCaseSensitively) ?
                                        Qt::CaseSensitive : Qt::CaseInsensitive);
    m_findFlags = findFlags;

    viewport()->update(viewport()->visibleRegion());
}

void QmlCodeEditor::updateUses()
{
    if (codeDocument()->isSemanticInfoOutdated()) // will be updated when info is updated
        return;

    QList<QTextEdit::ExtraSelection> selections;
    foreach (const AST::SourceLocation &loc,
             codeDocument()->semanticInfo().idLocations.value(wordUnderCursor())) {
        if (!loc.isValid())
            continue;

        QTextEdit::ExtraSelection sel;
        sel.format = CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_OCCURRENCES);
        sel.cursor = textCursor();
        sel.cursor.setPosition(loc.begin());
        sel.cursor.setPosition(loc.end(), QTextCursor::KeepAnchor);
        selections.append(sel);
    }

    setExtraSelections("CodeSemanticsSelection", selections);
}

void QmlCodeEditor::updateTabStops()
{
    // Although the tab stop is stored as qreal the API from QPlainTextEdit only allows it
    // to be set as an int. A work around is to access directly the QTextOption.
    qreal charWidth = fontMetrics().horizontalAdvance(QLatin1Char(' '));
    QTextOption option = codeDocument()->defaultTextOption();
    option.setTabStopDistance(charWidth * codeDocument()->tabSettings().m_tabSize);
    codeDocument()->setDefaultTextOption(option);
}

void QmlCodeEditor::semanticInfoUpdated(const SemanticInfo &semanticInfo)
{
    if (isVisible()) {
        // trigger semantic highlighting and model update if necessary
        codeDocument()->triggerPendingUpdates();
    }

    if (m_contextPane) {
        Node *newNode = semanticInfo.declaringMemberNoProperties(position());
        if (newNode) {
            m_contextPane->apply(this, semanticInfo.document, 0, newNode, true);
            m_contextPaneTimer->start(); //update text marker
        }
    }

    updateUses();
}

void QmlCodeEditor::animateUpdate(const QTextCursor& cursor, QPointF lastPos, QRectF rect)
{
    viewport()->update(QRectF(cursorRect(cursor).topLeft() + rect.topLeft(), rect.size()).toAlignedRect());
    if (!lastPos.isNull())
        viewport()->update(QRectF(lastPos + rect.topLeft(), rect.size()).toAlignedRect());
}

void QmlCodeEditor::indent()
{
    doSetTextCursor(codeDocument()->indent(textCursor()));
}

void QmlCodeEditor::unindent()
{
    doSetTextCursor(codeDocument()->unindent(textCursor()));
}

QString QmlCodeEditor::wordUnderCursor() const
{
    QTextCursor tc = textCursor();
    const QChar ch = document()->characterAt(tc.position() - 1);
    // make sure that we're not at the start of the next word.
    if (ch.isLetterOrNumber() || ch == QLatin1Char('_'))
        tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::StartOfWord);
    tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    const QString word = tc.selectedText();
    return word;
}

void QmlCodeEditor::gotoLine(int line, int column, bool centerLine, bool animate)
{
    //    m_lastCursorChangeWasInteresting = false; // avoid adding the previous position to history
    const int blockNumber = qMin(line - 1, document()->blockCount());
    const QTextBlock &block = document()->findBlockByNumber(blockNumber);
    if (block.isValid()) {
        QTextCursor cursor(block);
        if (column > 0) {
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column - 1);
        } else {
            int pos = cursor.position();
            while (document()->characterAt(pos).category() == QChar::Separator_Space) {
                ++pos;
            }
            cursor.setPosition(pos);
        }

        //        const DisplaySettings &ds = m_displaySettings;
        if (animate /*&& ds.m_animateNavigationWithinFile*/) {
            QScrollBar *scrollBar = verticalScrollBar();
            const int start = scrollBar->value();

            //            ensureBlockIsUnfolded(block); BUG
            setUpdatesEnabled(false);
            setTextCursor(cursor);
            if (centerLine)
                centerCursor();
            else
                ensureCursorVisible();
            const int end = scrollBar->value();
            scrollBar->setValue(start);
            setUpdatesEnabled(true);

            const int delta = end - start;
            // limit the number of steps for the animation otherwise you wont be able to tell
            // the direction of the animantion for large delta values
            const int steps = qMax(-/*ds.m_animateWithinFileTimeMax*/300,
                                   qMin(/*ds.m_animateWithinFileTimeMax*/300, delta));
            // limit the duration of the animation to at least 4 pictures on a 60Hz Monitor and
            // at most to the number of absolute steps
            const int durationMinimum = int (4 // number of pictures
                                             * float(1) / 60 // on a 60 Hz Monitor
                                             * 1000); // milliseconds
            const int duration = qMax(durationMinimum, qAbs(steps));

            auto navigationAnimation = new QSequentialAnimationGroup(this);
            auto startAnimation = new QPropertyAnimation(verticalScrollBar(), "value");
            startAnimation->setEasingCurve(QEasingCurve::InExpo);
            startAnimation->setStartValue(start);
            startAnimation->setEndValue(start + steps / 2);
            startAnimation->setDuration(duration / 2);
            navigationAnimation->addAnimation(startAnimation);
            auto endAnimation = new QPropertyAnimation(verticalScrollBar(), "value");
            endAnimation->setEasingCurve(QEasingCurve::OutExpo);
            endAnimation->setStartValue(end - steps / 2);
            endAnimation->setEndValue(end);
            endAnimation->setDuration(duration / 2);
            navigationAnimation->addAnimation(endAnimation);
            navigationAnimation->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            setTextCursor(cursor);
            if (centerLine)
                centerCursor();
            else
                ensureCursorVisible();
        }
    }
    //    saveCurrentCursorPositionForNavigation();
}

bool QmlCodeEditor::openLink(const Utils::Link &link)
{
    if (!link.hasValidTarget())
        return false;

    if (codeDocument()->filePath() == link.targetFileName) {
        gotoLine(link.targetLine, link.targetColumn, true, true);
        setFocus();
        return true;
    }

    return false;
    // FIXME return EditorManager::openEditorAt(link.targetFileName, link.targetLine, link.targetColumn, Id());
}

bool QmlCodeEditor::cursorMoveKeyEvent(QKeyEvent *e)
{
    QTextCursor cursor = textCursor();

    QTextCursor::MoveMode mode = QTextCursor::MoveAnchor;
    QTextCursor::MoveOperation op = QTextCursor::NoMove;

    if (e == QKeySequence::MoveToNextChar) {
        op = QTextCursor::Right;
    } else if (e == QKeySequence::MoveToPreviousChar) {
        op = QTextCursor::Left;
    } else if (e == QKeySequence::SelectNextChar) {
        op = QTextCursor::Right;
        mode = QTextCursor::KeepAnchor;
    } else if (e == QKeySequence::SelectPreviousChar) {
        op = QTextCursor::Left;
        mode = QTextCursor::KeepAnchor;
    } else if (e == QKeySequence::SelectNextWord) {
        op = QTextCursor::WordRight;
        mode = QTextCursor::KeepAnchor;
    } else if (e == QKeySequence::SelectPreviousWord) {
        op = QTextCursor::WordLeft;
        mode = QTextCursor::KeepAnchor;
    } else if (e == QKeySequence::SelectStartOfLine) {
        op = QTextCursor::StartOfLine;
        mode = QTextCursor::KeepAnchor;
    } else if (e == QKeySequence::SelectEndOfLine) {
        op = QTextCursor::EndOfLine;
        mode = QTextCursor::KeepAnchor;
    } else if (e == QKeySequence::SelectStartOfBlock) {
        op = QTextCursor::StartOfBlock;
        mode = QTextCursor::KeepAnchor;
    } else if (e == QKeySequence::SelectEndOfBlock) {
        op = QTextCursor::EndOfBlock;
        mode = QTextCursor::KeepAnchor;
    } else if (e == QKeySequence::SelectStartOfDocument) {
        op = QTextCursor::Start;
        mode = QTextCursor::KeepAnchor;
    } else if (e == QKeySequence::SelectEndOfDocument) {
        op = QTextCursor::End;
        mode = QTextCursor::KeepAnchor;
    } else if (e == QKeySequence::SelectPreviousLine) {
        op = QTextCursor::Up;
        mode = QTextCursor::KeepAnchor;
    } else if (e == QKeySequence::SelectNextLine) {
        op = QTextCursor::Down;
        mode = QTextCursor::KeepAnchor;
        {
            QTextBlock block = cursor.block();
            QTextLine line = currentTextLine(cursor);
            if (!block.next().isValid()
                    && line.isValid()
                    && line.lineNumber() == block.layout()->lineCount() - 1)
                op = QTextCursor::End;
        }
    } else if (e == QKeySequence::MoveToNextWord) {
        op = QTextCursor::WordRight;
    } else if (e == QKeySequence::MoveToPreviousWord) {
        op = QTextCursor::WordLeft;
    } else if (e == QKeySequence::MoveToEndOfBlock) {
        op = QTextCursor::EndOfBlock;
    } else if (e == QKeySequence::MoveToStartOfBlock) {
        op = QTextCursor::StartOfBlock;
    } else if (e == QKeySequence::MoveToNextLine) {
        op = QTextCursor::Down;
    } else if (e == QKeySequence::MoveToPreviousLine) {
        op = QTextCursor::Up;
    } else if (e == QKeySequence::MoveToStartOfLine) {
        op = QTextCursor::StartOfLine;
    } else if (e == QKeySequence::MoveToEndOfLine) {
        op = QTextCursor::EndOfLine;
    } else if (e == QKeySequence::MoveToStartOfDocument) {
        op = QTextCursor::Start;
    } else if (e == QKeySequence::MoveToEndOfDocument) {
        op = QTextCursor::End;
    } else {
        return false;
    }


    // Except for pageup and pagedown, Mac OS X has very different behavior, we don't do it all, but
    // here's the breakdown:
    // Shift still works as an anchor, but only one of the other keys can be down Ctrl (Command),
    // Alt (Option), or Meta (Control).
    // Command/Control + Left/Right -- Move to left or right of the line
    //                 + Up/Down -- Move to top bottom of the file. (Control doesn't move the cursor)
    // Option + Left/Right -- Move one word Left/right.
    //        + Up/Down  -- Begin/End of Paragraph.
    // Home/End Top/Bottom of file. (usually don't move the cursor, but will select)

    bool visualNavigation = cursor.visualNavigation();
    cursor.setVisualNavigation(true);

    if (/*q->camelCaseNavigationEnabled()*/true && op == QTextCursor::WordRight)
        camelCaseRight(cursor, mode);
    else if (/*q->camelCaseNavigationEnabled()*/true && op == QTextCursor::WordLeft)
        camelCaseLeft(cursor, mode);
    else if (!cursor.movePosition(op, mode) && mode == QTextCursor::MoveAnchor)
        cursor.clearSelection();
    cursor.setVisualNavigation(visualNavigation);

    setTextCursor(cursor);
    ensureCursorVisible();
    return true;
}

void QmlCodeEditor::openLinkUnderCursor()
{
    findLinkAt(textCursor(),
               [self = QPointer<QmlCodeEditor>(this)](const Link &symbolLink) {
        if (self)
            self->openLink(symbolLink);
    });
}

void QmlCodeEditor::findLinkAt(const QTextCursor &cursor,
                               Utils::ProcessLinkCallback &&processLinkCallback)
{
    const SemanticInfo semanticInfo = codeDocument()->semanticInfo();
    if (! semanticInfo.isValid())
        return processLinkCallback(Utils::Link());

    const unsigned cursorPosition = cursor.position();

    AST::Node *node = semanticInfo.astNodeAt(cursorPosition);
    QTC_ASSERT(node, return;);

    if (AST::UiImport *importAst = cast<AST::UiImport *>(node)) {
        // if it's a file import, link to the file
        foreach (const ImportInfo &import, semanticInfo.document->bind()->imports()) {
            if (import.ast() == importAst && import.type() == ImportType::File) {
                Utils::Link link(import.path());
                link.linkTextStart = importAst->firstSourceLocation().begin();
                link.linkTextEnd = importAst->lastSourceLocation().end();
                processLinkCallback(Utils::Link());
                return;
            }
        }
        processLinkCallback(Utils::Link());
        return;
    }

    // string literals that could refer to a file link to them
    if (StringLiteral *literal = cast<StringLiteral *>(node)) {
        const QString &text = literal->value.toString();
        Utils::Link link;
        link.linkTextStart = literal->literalToken.begin();
        link.linkTextEnd = literal->literalToken.end();
        if (semanticInfo.snapshot.document(text)) {
            link.targetFileName = text;
            processLinkCallback(link);
            return;
        }
        const QString relative = QString::fromLatin1("%1/%2").arg(
                    semanticInfo.document->path(),
                    text);
        if (semanticInfo.snapshot.document(relative)) {
            link.targetFileName = relative;
            processLinkCallback(link);
            return;
        }
    }

    const ScopeChain scopeChain = semanticInfo.scopeChain(semanticInfo.rangePath(cursorPosition));
    Evaluate evaluator(&scopeChain);
    const Value *value = evaluator.reference(node);

    QString fileName;
    int line = 0, column = 0;

    if (! (value && value->getSourceLocation(&fileName, &line, &column)))
        return processLinkCallback(Utils::Link());

    Utils::Link link;
    link.targetFileName = fileName;
    link.targetLine = line;
    link.targetColumn = column - 1; // adjust the column

    if (AST::UiQualifiedId *q = AST::cast<AST::UiQualifiedId *>(node)) {
        for (AST::UiQualifiedId *tail = q; tail; tail = tail->next) {
            if (! tail->next && cursorPosition <= tail->identifierToken.end()) {
                link.linkTextStart = tail->identifierToken.begin();
                link.linkTextEnd = tail->identifierToken.end();
                processLinkCallback(link);
                return;
            }
        }

    } else if (AST::IdentifierExpression *id = AST::cast<AST::IdentifierExpression *>(node)) {
        link.linkTextStart = id->firstSourceLocation().begin();
        link.linkTextEnd = id->lastSourceLocation().end();
        processLinkCallback(link);
        return;

    } else if (AST::FieldMemberExpression *mem = AST::cast<AST::FieldMemberExpression *>(node)) {
        link.linkTextStart = mem->lastSourceLocation().begin();
        link.linkTextEnd = mem->lastSourceLocation().end();
        processLinkCallback(link);
        return;
    }

    processLinkCallback(Utils::Link());
}

void QmlCodeEditor::updateLink()
{
    if (m_pendingLinkUpdate.isNull())
        return;
    if (m_pendingLinkUpdate == m_lastLinkUpdate)
        return;

    m_lastLinkUpdate = m_pendingLinkUpdate;
    findLinkAt(m_pendingLinkUpdate,
               [parent = QPointer<QmlCodeEditor>(this), this](const Link &link) {
        if (!parent)
            return;

        if (link.hasValidLinkText())
            showLink(link);
        else
            clearLink();
    });
}

void QmlCodeEditor::showLink(const Utils::Link &link)
{
    if (m_currentLink == link)
        return;

    QTextEdit::ExtraSelection sel;
    sel.cursor = textCursor();
    sel.cursor.setPosition(link.linkTextStart);
    sel.cursor.setPosition(link.linkTextEnd, QTextCursor::KeepAnchor);
    sel.format = CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_LINK);
    sel.format.setFontUnderline(true);
    setExtraSelections("OtherSelection", QList<QTextEdit::ExtraSelection>() << sel);
    viewport()->setCursor(Qt::PointingHandCursor);
    m_currentLink = link;
    m_linkPressed = false;
}

void QmlCodeEditor::clearLink()
{
    m_pendingLinkUpdate = QTextCursor();
    m_lastLinkUpdate = QTextCursor();
    if (!m_currentLink.hasValidLinkText())
        return;

    setExtraSelections("OtherSelection", QList<QTextEdit::ExtraSelection>());
    viewport()->setCursor(Qt::IBeamCursor);
    m_currentLink = Utils::Link();
    m_linkPressed = false;
}

void QmlCodeEditor::requestUpdateLink(QMouseEvent *e, bool immediate)
{
    //    if (!mouseNavigationEnabled())
    //        return;
    if (e->modifiers() & Qt::ControlModifier) {
        // Link emulation behaviour for 'go to definition'
        const QTextCursor cursor = cursorForPosition(e->pos());

        // Avoid updating the link we already found
        if (cursor.position() >= m_currentLink.linkTextStart
                && cursor.position() <= m_currentLink.linkTextEnd)
            return;

        // Check that the mouse was actually on the text somewhere
        bool onText = cursorRect(cursor).right() >= e->x();
        if (!onText) {
            QTextCursor nextPos = cursor;
            nextPos.movePosition(QTextCursor::Right);
            onText = cursorRect(nextPos).right() >= e->x();
        }

        if (onText) {
            m_pendingLinkUpdate = cursor;

            if (immediate)
                updateLink();
            else
                QTimer::singleShot(0, this, &QmlCodeEditor::updateLink);

            return;
        }
    }

    clearLink();
}

void QmlCodeEditor::createToolBar()
{
    // BUG
    //    m_outlineCombo = new QComboBox;
    //    m_outlineCombo->setMinimumContentsLength(22);
    //    m_outlineCombo->setModel(m_qmlJsEditorDocument->outlineModel());

    //    QTreeView *treeView = new QTreeView;

    //    Utils::AnnotatedItemDelegate *itemDelegate = new Utils::AnnotatedItemDelegate(this);
    //    itemDelegate->setDelimiter(QLatin1String(" "));
    //    itemDelegate->setAnnotationRole(QmlOutlineModel::AnnotationRole);
    //    treeView->setItemDelegateForColumn(0, itemDelegate);

    //    treeView->header()->hide();
    //    treeView->setItemsExpandable(false);
    //    treeView->setRootIsDecorated(false);
    //    m_outlineCombo->setView(treeView);
    //    treeView->expandAll();

    //    //m_outlineCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    //    // Make the combo box prefer to expand
    //    QSizePolicy policy = m_outlineCombo->sizePolicy();
    //    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    //    m_outlineCombo->setSizePolicy(policy);

    //    connect(m_outlineCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
    //            this, &QmlCodeEditor::jumpToOutlineElement);
    //    connect(m_qmlJsEditorDocument->outlineModel(), &QmlOutlineModel::updated,
    //            static_cast<QTreeView *>(m_outlineCombo->view()), &QTreeView::expandAll);

    //    connect(this, &QmlCodeEditor::cursorPositionChanged,
    //            &m_updateOutlineIndexTimer, static_cast<void (QTimer::*)()>(&QTimer::start));

    //    insertExtraToolBarWidget(QmlCodeEditor::Left, m_outlineCombo);
}

void QmlCodeEditor::updateContextPane()
{
    const SemanticInfo info = codeDocument()->semanticInfo();
    if (m_contextPane && document() && info.isValid()
            && document()->revision() == info.document->editorRevision())
    {
        Node *oldNode = info.declaringMemberNoProperties(m_oldCursorPosition);
        Node *newNode = info.declaringMemberNoProperties(position());
        if (oldNode != newNode && m_oldCursorPosition != -1)
            m_contextPane->apply(this, info.document, 0, newNode, false);

        if (m_contextPane->isAvailable(this, info.document, newNode) &&
                !m_contextPane->widget()->isVisible()) {
            QList<RefactorMarker> markers = removeMarkersOfType<QtQuickToolbarMarker>(refactorMarkers());
            if (UiObjectMember *m = newNode->uiObjectMemberCast()) {
                const int start = qualifiedTypeNameId(m)->identifierToken.begin();
                for (UiQualifiedId *q = qualifiedTypeNameId(m); q; q = q->next) {
                    if (!q->next) {
                        const int end = q->identifierToken.end();
                        if (position() >= start && position() <= end) {
                            RefactorMarker marker;
                            QTextCursor tc(document());
                            tc.setPosition(end);
                            marker.cursor = tc;
                            marker.tooltip = tr("Show Qt Quick ToolBar");
                            marker.data = QVariant::fromValue(QtQuickToolbarMarker());
                            markers.append(marker);
                        }
                    }
                }
            }
            setRefactorMarkers(markers);
        } else if (oldNode != newNode) {
            setRefactorMarkers(removeMarkersOfType<QtQuickToolbarMarker>(refactorMarkers()));
        }
        m_oldCursorPosition = position();

        //                setSelectedElements();
    }
}

void QmlCodeEditor::slotSelectionChanged()
{
    //    if (!textCursor().hasSelection() && !m_selectBlockAnchor.isNull())
    //        m_selectBlockAnchor = QTextCursor();
    // Clear any link which might be showing when the selection changes
    clearLink();
}

void QmlCodeEditor::showContextPane()
{
    const SemanticInfo info = codeDocument()->semanticInfo();
    if (m_contextPane && info.isValid()) {
        Node *newNode = info.declaringMemberNoProperties(position());
        ScopeChain scopeChain = info.scopeChain(info.rangePath(position()));
        m_contextPane->apply(this, info.document,
                             &scopeChain,
                             newNode, false, true);
        m_oldCursorPosition = position();
        setRefactorMarkers(removeMarkersOfType<QtQuickToolbarMarker>(refactorMarkers()));
    }
}

bool QmlCodeEditor::hideContextPane()
{
    bool b = (m_contextPane) && m_contextPane->widget()->isVisible();
    if (b)
        m_contextPane->apply(this, codeDocument()->semanticInfo().document, 0, 0, false);
    return b;
}

void QmlCodeEditor::showTextMarker()
{
    m_oldCursorPosition = -1;
    updateContextPane();
}

void QmlCodeEditor::updateCodeWarnings(Document::Ptr doc)
{
    if (doc->ast()) {
        setExtraSelections("CodeWarningsSelection", QList<QTextEdit::ExtraSelection>());
    } else if (doc->language().isFullySupportedLanguage()) {
        // show parsing errors
        QList<QTextEdit::ExtraSelection> selections;
        appendExtraSelectionsForMessages(&selections, doc->diagnosticMessages(), codeDocument());
        setExtraSelections("CodeWarningsSelection", selections);
    } else {
        setExtraSelections("CodeWarningsSelection", QList<QTextEdit::ExtraSelection>());
    }
}

void QmlCodeEditor::applyFontSettingsDelayed()
{
    m_fontSettingsNeedsApply = true;
    if (isVisible())
        triggerPendingUpdates();
}

void QmlCodeEditor::triggerPendingUpdates()
{
    if (m_fontSettingsNeedsApply)
        applyFontSettings();
    codeDocument()->triggerPendingUpdates();
}

bool QmlCodeEditor::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::ShortcutOverride:
        if (static_cast<QKeyEvent*>(e)->key() == Qt::Key_Escape && m_contextPane) {
            if (hideContextPane()) {
                e->accept();
                return true;
            }
        }
        break;
    case QEvent::ApplicationPaletteChange: {
        // slight hack: ignore palette changes
        // at this point the palette has changed already,
        // so undo it by re-setting the palette:
        applyFontSettings();
        return true;
    }
    case QEvent::Paint: {
        QRectF r(rect());
        QPainter p(this);
        p.setPen("#b6b6b6");
        p.drawLine(r.topLeft(), r.bottomLeft());
        p.drawLine(r.topRight(), r.bottomRight());
        return true;
    }
    default:
        break;
    }

    return QPlainTextEdit::event(e);
}

void QmlCodeEditor::wheelEvent(QWheelEvent *event)
{
    bool visible = false;
    if (m_contextPane && m_contextPane->widget()->isVisible())
        visible = true;

    QPlainTextEdit::wheelEvent(event);

    if (visible)
        m_contextPane->apply(this, codeDocument()->semanticInfo().document, 0,
                             codeDocument()->semanticInfo().declaringMemberNoProperties(m_oldCursorPosition),
                             false, true);
}

void QmlCodeEditor::paintEvent(QPaintEvent* e)
{
    PaintEventData data(this, e, contentOffset());
    QTC_ASSERT(data.documentLayout, return);

    QPainter painter(viewport());
    // Set a brush origin so that the WaveUnderline knows where the wave started
    painter.setBrushOrigin(data.offset);
    painter.setRenderHint(QPainter::Antialiasing);

    data.block = firstVisibleBlock();
    data.context = getPaintContext();
    // clear the back ground of the normal selection when in block selection mode
    clearSelectionBackground(data);

    { // paint background
        // draw backgrond to the right of the wrap column before everything else
        //        paintRightMarginArea(data, painter); // BUG
        // paint a blended background color depending on scope depth
        // We don't need this feature, paints block indentations
        //        paintBlockHighlight(data, painter);
        // paint background of if defed out blocks in bigger chunks
        //        paintIfDefedOutBlocks(data, painter);
        //        paintRightMarginLine(data, painter);
        // paint find scope on top of ifdefed out blocks and right margin
        paintFindScope(data, painter);
        // paint search results on top of the find scope
        paintSearchResultOverlay(data, painter);
    }

    while (data.block.isValid()) {

        PaintEventBlockData blockData;
        blockData.boundingRect = blockBoundingRect(data.block).translated(data.offset);

        if (blockData.boundingRect.bottom() >= data.eventRect.top()
                && blockData.boundingRect.top() <= data.eventRect.bottom()) {

            setupBlockLayout(data, painter, blockData);
            blockData.position = data.block.position();
            blockData.length = data.block.length();
            setupSelections(data, blockData);

            paintCurrentLineHighlight(data, painter);
            //            paintBlockSelection(data, painter, blockData);

            bool drawCursor = data.context.cursorPosition >= blockData.position
                    && data.context.cursorPosition < blockData.position + blockData.length;

            bool drawCursorAsBlock = drawCursor && overwriteMode() /*&& !m_inBlockSelectionMode*/;

            if (drawCursorAsBlock)
                paintCursorAsBlock(data, painter, blockData);

            if (data.block.isValid() && data.block.isVisible())
                paintBlock(&painter, data.block, data.offset, blockData.selections, data.eventRect);

            if ((drawCursor && !drawCursorAsBlock)
                    || (data.isEditable && data.context.cursorPosition < -1
                        && !blockData.layout->preeditAreaText().isEmpty())) {
                setupCursorPosition(data, painter, blockData);
            }

            //            if ((!HostOsInfo::isMacHost()
            //                 || m_blockSelection.positionColumn == m_blockSelection.anchorColumn)
            //                    && blockData.blockSelectionCursorRect.isValid()) {
            //                painter.fillRect(blockData.blockSelectionCursorRect, palette().text());
            //            }

            //            paintAdditionalVisualWhitespaces(data, painter, blockData.boundingRect.top());
            if (data.block.isValid() && data.block.isVisible())
                paintReplacement(data, painter, blockData.boundingRect.top());
        }
        //        updateLineAnnotation(data, blockData, painter);

        data.offset.ry() += blockData.boundingRect.height();

        if (data.offset.y() > data.viewportRect.height())
            break;

        data.block = data.block.next();

        if (!data.block.isVisible()) {
            if (data.block.blockNumber() == m_visibleFoldedBlockNumber) {
                data.visibleCollapsedBlock = data.block;
                data.visibleCollapsedBlockOffset = data.offset;
            }

            // invisible blocks do have zero line count
            data.block = data.doc->findBlockByLineNumber(data.block.firstLineNumber());
        }
    }

    //    cleanupAnnotationCache();

    painter.setPen(data.context.palette.text().color());

    // paint background of the widget that is not covered by the document
    paintWidgetBackground(data, painter);

    updateAnimator(m_bracketsAnimator, painter);
    updateAnimator(m_autocompleteAnimator, painter);

    paintOverlays(data, painter);

    // draw the cursor last, on top of everything
    paintCursor(data, painter);

    // paint a popup with the content of the collapsed block
    drawCollapsedBlockPopup(painter, data.visibleCollapsedBlock,
                            data.visibleCollapsedBlockOffset, data.eventRect);
}

void QmlCodeEditor::paintCursorAsBlock(const PaintEventData &data, QPainter &painter,
                                       PaintEventBlockData &blockData) const
{
    const QFontMetricsF fontMetrics(blockData.layout->font());
    int relativePos = data.context.cursorPosition - blockData.position;
    bool doSelection = true;
    QTextLine line = blockData.layout->lineForTextPosition(relativePos);
    qreal x = line.cursorToX(relativePos);
    qreal w = 0;
    if (relativePos < line.textLength() - line.textStart()) {
        w = line.cursorToX(relativePos + 1) - x;
        if (data.doc->characterAt(data.context.cursorPosition) == QLatin1Char('\t')) {
            doSelection = false;
            qreal space = fontMetrics.horizontalAdvance(QLatin1Char(' '));
            if (w > space) {
                x += w-space;
                w = space;
            }
        }
    } else
        w = fontMetrics.horizontalAdvance(QLatin1Char(' ')); // in sync with QTextLine::draw()

    QRectF lineRect = line.rect();
    lineRect.moveTop(lineRect.top() + blockData.boundingRect.top());
    lineRect.moveLeft(blockData.boundingRect.left() + x);
    lineRect.setWidth(w);
    painter.fillRect(lineRect, palette().text());
    if (doSelection)
        blockData.selections.append(createBlockCursorCharFormatRange(relativePos, palette()));
}

void QmlCodeEditor::setupBlockLayout(const PaintEventData &data,
                                     QPainter &painter,
                                     PaintEventBlockData &blockData) const

{
    blockData.layout = data.block.layout();

    QTextOption option = blockData.layout->textOption();
    if (data.suppressSyntaxInIfdefedOutBlock && false/*TextDocumentLayout::ifdefedOut(data.block)*/) {
        option.setFlags(option.flags() | QTextOption::SuppressColors);
        painter.setPen(data.ifdefedOutFormat.foreground().color());
    } else {
        option.setFlags(option.flags() & ~QTextOption::SuppressColors);
        painter.setPen(data.context.palette.text().color());
    }
    blockData.layout->setTextOption(option);
    blockData.layout->setFont(data.doc->defaultFont());
}

void QmlCodeEditor::setupSelections(const PaintEventData &data,
                                    PaintEventBlockData &blockData) const
{
    for (int i = 0; i < data.context.selections.size(); ++i) {
        const QAbstractTextDocumentLayout::Selection &range = data.context.selections.at(i);
        const int selStart = range.cursor.selectionStart() - blockData.position;
        const int selEnd = range.cursor.selectionEnd() - blockData.position;
        if (selStart < blockData.length && selEnd >= 0
                && selEnd >= selStart) {
            QTextLayout::FormatRange o;
            o.start = selStart;
            o.length = selEnd - selStart;
            o.format = range.format;
            //            if (i == data.blockSelectionIndex) { BUG ??
            //                QString text = data.block.text();
            //                const TabSettings &ts = codeDocument()->tabSettings();
            //                o.start = ts.positionAtColumn(text, m_blockSelection.firstVisualColumn());
            //                o.length = ts.positionAtColumn(text, m_blockSelection.lastVisualColumn()) - o.start;
            //            }
            if ((data.textCursor.hasSelection() && i == data.context.selections.size() - 1)
                    || (o.format.foreground().style() == Qt::NoBrush
                        && o.format.underlineStyle() != QTextCharFormat::NoUnderline
                        && o.format.background() == Qt::NoBrush)) {
                if (selectionVisible(data.block.blockNumber()))
                    blockData.prioritySelections.append(o);
            } else {
                blockData.selections.append(o);
            }
        }
    }
    blockData.selections += blockData.prioritySelections;
}

void QmlCodeEditor::paintBlock(QPainter *painter,
                               const QTextBlock &block,
                               const QPointF &offset,
                               const QVector<QTextLayout::FormatRange> &selections,
                               const QRect &clipRect) const
{
    block.layout()->draw(painter, offset, selections, clipRect);
}

void QmlCodeEditor::setupCursorPosition(PaintEventData &data,
                                        QPainter &painter,
                                        PaintEventBlockData &blockData) const
{
    int cursorPos = data.context.cursorPosition;
    if (cursorPos < -1)
        cursorPos = blockData.layout->preeditAreaPosition() - (cursorPos + 2);
    else
        cursorPos -= blockData.position;
    data.cursorLayout = blockData.layout;
    data.cursorOffset = data.offset;
    data.cursorPos = cursorPos;
    data.cursorPen = painter.pen();
}

void QmlCodeEditor::paintReplacement(PaintEventData &data, QPainter &painter,
                                     qreal top) const
{
    QTextBlock nextBlock = data.block.next();
    //    QTextBlock nextVisibleBlock = QmlCodeEditor::nextVisibleBlock(data.block, data.doc);

    if (nextBlock.isValid() && !nextBlock.isVisible() && replacementVisible(data.block.blockNumber())) {
        const bool selectThis = (data.textCursor.hasSelection()
                                 && nextBlock.position() >= data.textCursor.selectionStart()
                                 && nextBlock.position() < data.textCursor.selectionEnd());
        painter.save();
        if (selectThis) {
            painter.setBrush(palette().highlight());
        } else {
            QColor rc = replacementPenColor(data.block.blockNumber());
            if (rc.isValid())
                painter.setPen(rc);
        }

        QTextLayout *layout = data.block.layout();
        QTextLine line = layout->lineAt(layout->lineCount()-1);
        QRectF lineRect = line.naturalTextRect().translated(data.offset.x(), top);
        lineRect.adjust(0, 0, -1, -1);

        QString replacement = foldReplacementText(data.block);
        QString rectReplacement = QLatin1String(" {") + replacement + QLatin1String("}; ");

        QRectF collapseRect(lineRect.right() + 12,
                            lineRect.top(),
                            fontMetrics().horizontalAdvance(rectReplacement),
                            lineRect.height());
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.translate(.5, .5);
        painter.drawRoundedRect(collapseRect.adjusted(0, 0, 0, -1), 3, 3);
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.translate(-.5, -.5);

        //        if (BlockData *nextBlockUserData = QmlCodeDocument::testUserData(nextBlock)) {
        //            if (nextBlockUserData->foldingStartIncluded())
        //                replacement.prepend(nextBlock.text().trimmed().left(1));
        //        }

        //        data.block = nextVisibleBlock.previous();
        //        if (!data.block.isValid())
        //            data.block = data.doc->lastBlock();

        //        if (BlockData *blockUserData = QmlCodeDocument::testUserData(data.block)) {
        //            if (blockUserData->foldingEndIncluded()) {
        //                QString right = data.block.text().trimmed();
        //                if (right.endsWith(QLatin1Char(';'))) {
        //                    right.chop(1);
        //                    right = right.trimmed();
        //                    replacement.append(right.rightRef(right.endsWith('/') ? 2 : 1));
        //                    replacement.append(QLatin1Char(';'));
        //                } else { BUG
        //                    replacement.append(right.rightRef(right.endsWith('/') ? 2 : 1));
        //                }
        //            }
        //        }

        if (selectThis)
            painter.setPen(palette().highlightedText().color());
        painter.drawText(collapseRect, Qt::AlignCenter, replacement);
        painter.restore();
    }
}

QColor QmlCodeEditor::replacementPenColor(int blockNumber) const
{
    Q_UNUSED(blockNumber)
    return {};
}

QString QmlCodeEditor::foldReplacementText(const QTextBlock &) const
{
    return QString("· · ·");
}

bool QmlCodeEditor::selectionVisible(int blockNumber) const
{
    Q_UNUSED(blockNumber)
    return true;
}

bool QmlCodeEditor::replacementVisible(int blockNumber) const
{
    Q_UNUSED(blockNumber)
    return true;
}

void QmlCodeEditor::clearSelectionBackground(PaintEventData &data) const
{
    if (/*m_inBlockSelectionMode*/ false && data.context.selections.count()
            && data.context.selections.last().cursor == data.textCursor) {
        data.blockSelectionIndex = data.context.selections.size() - 1;
        data.context.selections[data.blockSelectionIndex].format.clearBackground();
    }
}

void QmlCodeEditor::paintWidgetBackground(const PaintEventData &data,
                                          QPainter &painter) const
{
    if (backgroundVisible()
            && !data.block.isValid()
            && data.offset.y() <= data.eventRect.bottom()
            && (centerOnScroll() || verticalScrollBar()->maximum() == verticalScrollBar()->minimum())) {
        const QRect backGroundRect(QPoint(data.eventRect.left(), int(data.offset.y())),
                                   data.eventRect.bottomRight());
        painter.fillRect(backGroundRect, palette().window());
    }
}

void QmlCodeEditor::paintCursor(const PaintEventData &data, QPainter &painter) const
{
    if (!data.cursorLayout /* BUG || m_inBlockSelectionMode*/)
        return;
    painter.setPen(data.cursorPen);
    data.cursorLayout->drawCursor(&painter, data.cursorOffset,
                                  data.cursorPos, cursorWidth());
}

void QmlCodeEditor::scrollContentsBy(int dx, int dy)
{
    QPlainTextEdit::scrollContentsBy(dx, dy);
    hideContextPane();
}

void QmlCodeEditor::showEvent(QShowEvent* e)
{
    triggerPendingUpdates();
    QPlainTextEdit::showEvent(e);
}

void QmlCodeEditor::resizeEvent(QResizeEvent* e)
{
    QPlainTextEdit::resizeEvent(e);

    int rcw = m_rowBar->calculatedWidth();
    QRect vg = viewport()->geometry();
    m_toolBar->setGeometry(0, 0, rcw + vg.width() + 3, m_toolBar->height());
    m_rowBar->setGeometry(1, vg.top(), rcw, vg.height());
    m_noDocsLabel->setGeometry(0, vg.top(), rcw + vg.width() + 3, vg.height());

    hideContextPane();
}

void QmlCodeEditor::keyPressEvent(QKeyEvent *e)
{
    //    if (!isModifier(e) && mouseHidingEnabled())
    //        viewport()->setCursor(Qt::BlankCursor);
    ToolTip::hide();

    //    d->m_moveLineUndoHack = false;
    /*//        d->*/clearVisibleFoldedBlock();

    //    if (e->key() == Qt::Key_Alt
    //            && d->m_behaviorSettings.m_keyboardTooltips) {
    //        d->m_maybeFakeTooltipEvent = true;
    //    } else {
    //        d->m_maybeFakeTooltipEvent = false;
    //        if (e->key() == Qt::Key_Escape
    //                && d->m_snippetOverlay->isVisible()) {
    //            e->accept();
    //            d->m_snippetOverlay->hide();
    //            d->m_snippetOverlay->mangle();
    //            d->m_snippetOverlay->clear();
    //            QTextCursor cursor = textCursor();
    //            cursor.clearSelection();
    //            setTextCursor(cursor);
    //            return;
    //        }
    //    }

    bool ro = isReadOnly();
    const bool inOverwriteMode = overwriteMode();

    //    if (!ro && d->m_inBlockSelectionMode) {
    //        if (e == QKeySequence::Cut) {
    //            cut();
    //            e->accept();
    //            return;
    //        } else if (e == QKeySequence::Delete || e->key() == Qt::Key_Backspace) {
    //            if (d->m_blockSelection.positionColumn == d->m_blockSelection.anchorColumn) {
    //                if (e == QKeySequence::Delete)
    //                    ++d->m_blockSelection.positionColumn;
    //                else if (d->m_blockSelection.positionColumn > 0)
    //                    --d->m_blockSelection.positionColumn;
    //            }
    //            d->removeBlockSelection();
    //            e->accept();
    //            return;
    //        } else if (e == QKeySequence::Paste) {
    //            d->removeBlockSelection();
    //            // continue
    //        }
    //    }


    if (!ro
            && (e == QKeySequence::InsertParagraphSeparator
                || (/*!d->m_lineSeparatorsAllowed*/true && e == QKeySequence::InsertLineSeparator))) {
        //        if (d->m_inBlockSelectionMode) {
        //            d->disableBlockSelection(TextEditorWidgetPrivate::CursorUpdateClearSelection);
        //            e->accept();
        //            return;
        //        }
        //        if (d->m_snippetOverlay->isVisible()) {
        //            e->accept();
        //            d->m_snippetOverlay->hide();
        //            d->m_snippetOverlay->mangle();
        //            d->m_snippetOverlay->clear();
        //            QTextCursor cursor = textCursor();
        //            cursor.movePosition(QTextCursor::EndOfBlock);
        //            setTextCursor(cursor);
        //            return;
        //        }

        QTextCursor cursor = textCursor();
        const TabSettings &ts = codeDocument()->tabSettings();
        const TypingSettings &tps = codeDocument()->typingSettings();
        cursor.beginEditBlock();

        int extraBlocks = m_autoCompleter->paragraphSeparatorAboutToBeInserted(cursor);

        QString previousIndentationString;
        if (tps.m_autoIndent) {
            cursor.insertBlock();
            codeDocument()->autoIndent(cursor);
        } else {
            cursor.insertBlock();

            // After inserting the block, to avoid duplicating whitespace on the same line
            const QString &previousBlockText = cursor.block().previous().text();
            previousIndentationString = ts.indentationString(previousBlockText);
            if (!previousIndentationString.isEmpty())
                cursor.insertText(previousIndentationString);
        }
        cursor.endEditBlock();
        e->accept();

        if (extraBlocks > 0) {
            QTextCursor ensureVisible = cursor;
            while (extraBlocks > 0) {
                --extraBlocks;
                ensureVisible.movePosition(QTextCursor::NextBlock);
                if (tps.m_autoIndent)
                    codeDocument()->autoIndent(ensureVisible);
                else if (!previousIndentationString.isEmpty())
                    ensureVisible.insertText(previousIndentationString);
                if (m_animateAutoComplete || m_highlightAutoComplete) {
                    QTextCursor tc = ensureVisible;
                    tc.movePosition(QTextCursor::EndOfBlock);
                    tc.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
                    tc.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
                    autocompleterHighlight(tc);
                }
            }
            setTextCursor(ensureVisible);
        }

        setTextCursor(cursor);
        return;
    } else if (!ro
               && (e == QKeySequence::MoveToStartOfBlock
                   || e == QKeySequence::SelectStartOfBlock)){
        if ((e->modifiers() & (Qt::AltModifier | Qt::ShiftModifier)) == (Qt::AltModifier | Qt::ShiftModifier)) {
            e->accept();
            return;
        }
        handleHomeKey(e == QKeySequence::SelectStartOfBlock);
        e->accept();
        return;
    } else if (!ro
               && (e == QKeySequence::MoveToStartOfLine
                   || e == QKeySequence::SelectStartOfLine)){
        if ((e->modifiers() & (Qt::AltModifier | Qt::ShiftModifier)) == (Qt::AltModifier | Qt::ShiftModifier)) {
            e->accept();
            return;
        }
        QTextCursor cursor = textCursor();
        if (QTextLayout *layout = cursor.block().layout()) {
            if (layout->lineForTextPosition(cursor.position() - cursor.block().position()).lineNumber() == 0) {
                handleHomeKey(e == QKeySequence::SelectStartOfLine);
                e->accept();
                return;
            }
        }
    } else if (!ro
               && e == QKeySequence::DeleteStartOfWord
               && codeDocument()->typingSettings().m_autoIndent
               && !textCursor().hasSelection()){
        e->accept();
        QTextCursor c = textCursor();
        int pos = c.position();
        if (/*camelCaseNavigationEnabled()*/true)
            camelCaseLeft(c, QTextCursor::MoveAnchor);
        else
            c.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
        int targetpos = c.position();
        forever {
            handleBackspaceKey();
            int cpos = textCursor().position();
            if (cpos == pos || cpos <= targetpos)
                break;
            pos = cpos;
        }
        return;
    } else if (!ro && e == QKeySequence::DeleteStartOfWord && !textCursor().hasSelection()) {
        e->accept();
        QTextCursor c = textCursor();
        if (/*camelCaseNavigationEnabled()*/true)
            camelCaseLeft(c, QTextCursor::KeepAnchor);
        else
            c.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
        c.removeSelectedText();
        return;
    } else if (!ro && e == QKeySequence::DeleteEndOfWord && !textCursor().hasSelection()) {
        e->accept();
        QTextCursor c = textCursor();
        if (/*camelCaseNavigationEnabled()*/true)
            camelCaseRight(c, QTextCursor::KeepAnchor);
        else
            c.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        c.removeSelectedText();
        return;
    }/* else if (!ro && (e == QKeySequence::MoveToNextPage || e == QKeySequence::MoveToPreviousPage)
               && d->m_inBlockSelectionMode) {
        d->disableBlockSelection(TextEditorWidgetPrivate::CursorUpdateClearSelection);
        QPlainTextEdit::keyPressEvent(e);
        return;
    } else if (!ro && (e == QKeySequence::SelectNextPage || e == QKeySequence::SelectPreviousPage)
               && d->m_inBlockSelectionMode) {
        QPlainTextEdit::keyPressEvent(e);
        d->m_blockSelection.positionBlock = QPlainTextEdit::textCursor().blockNumber();
        doSetTextCursor(d->m_blockSelection.selection(codeDocument().data()), true);
        viewport()->update();
        e->accept();
        return;
    } */else switch (e->key()) {


#if 0
    case Qt::Key_Dollar: {
        d->m_overlay->setVisible(!d->m_overlay->isVisible());
        d->m_overlay->setCursor(textCursor());
        e->accept();
        return;

    } break;
#endif
    case Qt::Key_Tab:
    case Qt::Key_Backtab: {
        if (ro) break;
        //        if (d->m_snippetOverlay->isVisible() && !d->m_snippetOverlay->isEmpty()) {
        //            d->snippetTabOrBacktab(e->key() == Qt::Key_Tab);
        //            e->accept();
        //            return;
        //        }
        QTextCursor cursor = textCursor();
        int newPosition;
        if (codeDocument()->typingSettings().tabShouldIndent(document(), cursor, &newPosition)) {
            if (newPosition != cursor.position() && !cursor.hasSelection()) {
                cursor.setPosition(newPosition);
                setTextCursor(cursor);
            }
            codeDocument()->autoIndent(cursor);
        } else {
            /*if (d->m_inBlockSelectionMode
                    && d->m_blockSelection.firstVisualColumn() != d->m_blockSelection.lastVisualColumn()) {
                d->removeBlockSelection();
            } else */{
                if (e->key() == Qt::Key_Tab)
                    indent();
                else
                    unindent();
            }
        }
        e->accept();
        return;
    } break;
    case Qt::Key_Backspace:
        if (ro) break;
        if ((e->modifiers() & (Qt::ControlModifier
                               | Qt::ShiftModifier
                               | Qt::AltModifier
                               | Qt::MetaModifier)) == Qt::NoModifier
                && !textCursor().hasSelection()) {
            handleBackspaceKey();
            e->accept();
            return;
        }
        break;
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Right:
    case Qt::Key_Left:
        if (HostOsInfo::isMacHost())
            break;
        if ((e->modifiers()
             & (Qt::AltModifier | Qt::ShiftModifier)) == (Qt::AltModifier | Qt::ShiftModifier)) {
            //            if (!d->m_inBlockSelectionMode)
            //                d->enableBlockSelection(textCursor());
            //            switch (e->key()) {
            //            case Qt::Key_Up:
            //                if (d->m_blockSelection.positionBlock > 0)
            //                    --d->m_blockSelection.positionBlock;
            //                break;
            //            case Qt::Key_Down:
            //                if (d->m_blockSelection.positionBlock < document()->blockCount() - 1)
            //                    ++d->m_blockSelection.positionBlock;
            //                break;
            //            case Qt::Key_Left:
            //                if (d->m_blockSelection.positionColumn > 0)
            //                    --d->m_blockSelection.positionColumn;
            //                break;
            //            case Qt::Key_Right:
            //                ++d->m_blockSelection.positionColumn;
            //                break;
            //            default:
            //                break;
            //            }
            //            d->resetCursorFlashTimer();
            //            doSetTextCursor(d->m_blockSelection.selection(codeDocument().data()), true);
            //            viewport()->update();
            //            e->accept();
            //            return;
        }/* else if (d->m_inBlockSelectionMode) { // leave block selection mode
            d->disableBlockSelection(TextEditorWidgetPrivate::NoCursorUpdate);
        }*/
        break;
    case Qt::Key_Insert:
        if (ro) break;
        if (e->modifiers() == Qt::NoModifier) {
            setOverwriteMode(!inOverwriteMode);
            e->accept();
            return;
        }
        break;

    default:
        break;
    }

    const QString eventText = e->text();
    //    if (!ro && d->m_inBlockSelectionMode) {
    //        if (isPrintableText(eventText)) {
    //            d->insertIntoBlockSelection(eventText);
    //            goto skip_event;
    //        }
    //    }

    //    if (e->key() == Qt::Key_H
    //            && e->modifiers() == Qt::KeyboardModifiers(HostOsInfo::controlModifier())) {
    //        d->universalHelper();
    //        e->accept();
    //        return;
    //    }

    if (ro || !isPrintableText(eventText)) {
        if (!cursorMoveKeyEvent(e)) {
            QTextCursor cursor = textCursor();
            bool cursorWithinSnippet = false;
            //            if (d->m_snippetOverlay->isVisible()
            //                    && (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace)) {
            //                cursorWithinSnippet = d->snippetCheckCursor(cursor);
            //            }
            if (cursorWithinSnippet)
                cursor.beginEditBlock();

            QPlainTextEdit::keyPressEvent(e);

            if (cursorWithinSnippet) {
                cursor.endEditBlock();
                //                d->m_snippetOverlay->updateEquivalentSelections(textCursor());
            }
        }
    } else if ((e->modifiers() & (Qt::ControlModifier|Qt::AltModifier)) != Qt::ControlModifier){
        // only go here if control is not pressed, except if also alt is pressed
        // because AltGr maps to Alt + Ctrl
        QTextCursor cursor = textCursor();
        QString autoText;
        if (!inOverwriteMode) {
            const bool skipChar = m_skipAutoCompletedText
                    && !m_autoCompleteHighlightPos.isEmpty()
                    && cursor == m_autoCompleteHighlightPos.last();
            autoText = m_autoCompleter->autoComplete(cursor, eventText, skipChar);
        }
        const bool cursorWithinSnippet = /*d->snippetCheckCursor(cursor)*/false;

        QChar electricChar;
        if (codeDocument()->typingSettings().m_autoIndent) {
            foreach (QChar c, eventText) {
                if (codeDocument()->indenter()->isElectricCharacter(c)) {
                    electricChar = c;
                    break;
                }
            }
        }

        bool doEditBlock = !electricChar.isNull() || !autoText.isEmpty() || cursorWithinSnippet;
        if (doEditBlock)
            cursor.beginEditBlock();

        if (inOverwriteMode) {
            if (!doEditBlock)
                cursor.beginEditBlock();
            QTextBlock block = cursor.block();
            int eolPos = block.position() + block.length() - 1;
            int selEndPos = qMin(cursor.position() + eventText.length(), eolPos);
            cursor.setPosition(selEndPos, QTextCursor::KeepAnchor);
            cursor.insertText(eventText);
            if (!doEditBlock)
                cursor.endEditBlock();
        } else {
            cursor.insertText(eventText);
        }

        if (!autoText.isEmpty()) {
            int pos = cursor.position();
            cursor.insertText(autoText);
            cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
            autocompleterHighlight(cursor);
            //Select the inserted text, to be able to re-indent the inserted text
            cursor.setPosition(pos, QTextCursor::KeepAnchor);
        }
        if (!electricChar.isNull() && m_autoCompleter->contextAllowsElectricCharacters(cursor))
            codeDocument()->autoIndent(cursor, electricChar);
        if (!autoText.isEmpty())
            cursor.setPosition(autoText.length() == 1 ? cursor.position() : cursor.anchor());

        if (doEditBlock) {
            cursor.endEditBlock();
            //            if (cursorWithinSnippet)
            //                d->m_snippetOverlay->updateEquivalentSelections(textCursor());
        }

        setTextCursor(cursor);
    }

    //skip_event:
    if (!ro && e->key() == Qt::Key_Delete && /*d->m_parenthesesMatchingEnabled*/true)
        m_parenthesesMatchingTimer->start(50);

    if (!ro && /*d->m_contentsChanged*/codeDocument()->isModified() && isPrintableText(eventText) && !inOverwriteMode)
        m_codeAssistant->process();
}
