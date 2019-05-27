#ifndef QMLCODEEDITOR_H
#define QMLCODEEDITOR_H

#include <QPlainTextEdit>
#include <QTimeLine>
#include <QPointer>
#include <QBasicTimer>

#include <utils/link.h>
#include <utils/filesearch.h>
#include <texteditor/codeassist/assistenums.h>
#include <texteditor/texteditorconstants.h>
#include <qmljs/qmljsdocument.h>
#include <qmljseditor/qmljsautocompleter.h>
#include <coreplugin/id.h>
#include <coreplugin/find/textfindconstants.h>

class QLabel;
class QBasicTimer;
class RowBar;
class QmlCodeDocument;
class HoverHandlerRunner;
struct PaintEventData;
struct PaintEventBlockData;
class QmlCodeEditorToolBar;

namespace QmlJSTools {
class SemanticInfo;
}

namespace TextEditor {
class TypingSettings;
class CompletionSettings;
class ColorPreviewHoverHandler;
class BaseHoverHandler;
class AssistInterface;
class CodeAssistant;
class CompletionAssistProvider;
class IAssistProvider;
class ICodeStylePreferences;
namespace Internal {
class TextEditorOverlay;
}
}

namespace QmlJSEditor {
class QuickToolBar;
namespace Internal {
class QmlJSHoverHandler;
}
}

class TextEditorAnimator : public QObject
{
    Q_OBJECT

public:
    TextEditorAnimator(QObject *parent);

    void init(const QTextCursor &cursor, const QFont &f, const QPalette &pal);
    inline QTextCursor cursor() const { return m_cursor; }

    void draw(QPainter *p, const QPointF &pos);
    QRectF rect() const;

    inline qreal value() const { return m_value; }
    inline QPointF lastDrawPos() const { return m_lastDrawPos; }

    void finish();

    bool isRunning() const;

signals:
    void updateRequest(const QTextCursor &cursor, QPointF lastPos, QRectF rect);

private:
    void step(qreal v);

    QTimeLine m_timeline;
    qreal m_value;
    QTextCursor m_cursor;
    QPointF m_lastDrawPos;
    QFont m_font;
    QPalette m_palette;
    QString m_text;
    QSizeF m_size;
};

struct TextEditorPrivateHighlightBlocks
{
    QList<int> open;
    QList<int> close;
    QList<int> visualIndent;
    inline int count() const { return visualIndent.size(); }
    inline bool isEmpty() const { return open.isEmpty() || close.isEmpty() || visualIndent.isEmpty(); }
    inline bool operator==(const TextEditorPrivateHighlightBlocks &o) const {
        return (open == o.open && close == o.close && visualIndent == o.visualIndent);
    }
    inline bool operator!=(const TextEditorPrivateHighlightBlocks &o) const { return !(*this == o); }
};

class QmlCodeEditor : public QPlainTextEdit
{
    Q_OBJECT

    friend class RowBar;
    friend class MarkBand;
    friend class BracketBand;
    friend class LineNumberBand;
    friend class TextEditor::Internal::TextEditorOverlay;

public:
    struct SearchResult {
        int start;
        int length;
    };

public:
    explicit QmlCodeEditor(QWidget* parent = nullptr);
    ~QmlCodeEditor();

    void setCodeDocument(QmlCodeDocument* document);
    QmlCodeDocument* codeDocument() const;

    RowBar* rowBar() const;
    QmlCodeEditorToolBar* toolBar() const;

    static TextEditor::CompletionAssistProvider* completionAssistProvider();
    static TextEditor::IAssistProvider* quickFixAssistProvider();
    TextEditor::AssistInterface* createAssistInterface(TextEditor::AssistKind kind,
                                                       TextEditor::AssistReason reason);
    void setVisibleFoldedBlockNumber(int visibleFoldedBlockNumber);
    void setExtraSelections(const QString& kind, const QList<QTextEdit::ExtraSelection>& selections);
    QList<QTextEdit::ExtraSelection> extraSelections(const QString& kind) const;
    void convertPosition(int pos, int *line, int *column) const;
    QRegion translatedLineRegion(int lineStart, int lineEnd) const;
    int position(TextEditor::TextPositionOperation posOp = TextEditor::CurrentPosition, int at = -1) const;
    void addHoverHandler(TextEditor::BaseHoverHandler *handler);
    QString extraSelectionTooltip(int pos) const;
    void setCursorPosition(int pos);
    void replace(int length, const QString& string);
    QRect cursorRect(int pos) const;
    using QPlainTextEdit::cursorRect;
    void setAutoCompleteSkipPosition(const QTextCursor& cursor);
    void updateCurrentLineHighlight();
    void setCodeStyle(TextEditor::ICodeStylePreferences* preferences);
    void setLanguageSettingsId(Core::Id settingsId);
    Core::Id languageSettingsId() const;
    bool inFindScope(int selectionStart, int selectionEnd);
    bool isValid() const;

public slots:
    void discharge();
    void indent();
    void unindent();
    void updateUses();
    void updateTabStops();
    void slotSelectionChanged();
    void updateAutoCompleteHighlight();
    void setNoDocsVisible(bool visible);
    void editorContentsChange(int, int, int);
    void highlightSearchResultsSlot(const QString& txt, Core::FindFlags findFlags);
    void semanticInfoUpdated(const QmlJSTools::SemanticInfo& semanticInfo);
    void animateUpdate(const QTextCursor &cursor, QPointF lastPos, QRectF rect);
    void slotCodeStyleSettingsChanged(const QVariant&);
    void slotUpdateBlockNotify(const QTextBlock& block);
    void searchResultsReady(int beginIndex, int endIndex);
    void gotoLine(int line, int column = 0, bool centerLine = true, bool animate = true);

private:
    void updateViewportMargins();
    void cancelCurrentAnimations();
    bool cursorMoveKeyEvent(QKeyEvent* e);
    void updateRowBar(const QRect &rect, int dy);
    void paintOverlays(const PaintEventData &data, QPainter &painter) const;
    void paintFindScope(const PaintEventData& data, QPainter& painter);
    void processTooltipRequest(const QTextCursor &c);
    QPoint toolTipPosition(const QTextCursor &c) const;
    void drawCollapsedBlockPopup(QPainter &painter, const QTextBlock &block, QPointF offset,
                                 const QRect &clip);
    void paintSearchResultOverlay(const PaintEventData& data, QPainter& painter);
    QTextBlock nextVisibleBlock(const QTextBlock& block, const QTextDocument* doc) const;
    void paintCurrentLineHighlight(const PaintEventData& data, QPainter& painter) const;
    void highlightSearchResults(const QTextBlock& block, TextEditor::Internal::TextEditorOverlay* overlay);
    QString wordUnderCursor() const;
    bool openLink(const Utils::Link& link);
    void openLinkUnderCursor();
    void findLinkAt(const QTextCursor &cursor, Utils::ProcessLinkCallback &&processLinkCallback);
    void showLink(const Utils::Link& link);
    void clearLink();
    void updateLink();
    void requestUpdateLink(QMouseEvent* e, bool immediate = false);
    void createToolBar();
    void updateContextPane();
    void showContextPane();
    bool hideContextPane();
    void showTextMarker();
    void updateCodeWarnings(QmlJS::Document::Ptr doc);
    void applyFontSettingsDelayed();
    void triggerPendingUpdates();
    void applyFontSettings();
    void updateHighlights();
    void slotCursorPositionChanged();
    void matchParentheses();
    void autocompleterHighlight(const QTextCursor& cursor = QTextCursor());
    void updateAnimator(QPointer<TextEditorAnimator> animator, QPainter& painter);
    void setCompletionSettings(const TextEditor::CompletionSettings& completionSettings);
    void handleBackspaceKey();
    void handleHomeKey(bool anchor);
    bool camelCaseLeft(QTextCursor &cursor, QTextCursor::MoveMode mode);
    bool camelCaseRight(QTextCursor &cursor, QTextCursor::MoveMode mode);
    void invokeAssist(TextEditor::AssistKind kind, TextEditor::IAssistProvider* provider = nullptr);
    void abortAssist();
    void setFindScope(const QTextCursor& start, const QTextCursor& end, int verticalBlockSelectionFirstColumn, int verticalBlockSelectionLastColumn);
    void paintCursor(const PaintEventData& data, QPainter& painter) const;
    void paintWidgetBackground(const PaintEventData& data, QPainter& painter) const;
    void clearSelectionBackground(PaintEventData& data) const;
    void paintReplacement(PaintEventData& data, QPainter& painter, qreal top) const;
    void setupCursorPosition(PaintEventData& data, QPainter& painter, PaintEventBlockData& blockData) const;
    void paintBlock(QPainter* painter, const QTextBlock& block, const QPointF& offset, const QVector<QTextLayout::FormatRange>& selections, const QRect& clipRect) const;
    void setupSelections(const PaintEventData& data, PaintEventBlockData& blockData) const;
    void setupBlockLayout(const PaintEventData& data, QPainter& painter, PaintEventBlockData& blockData) const;
    void paintCursorAsBlock(const PaintEventData& data, QPainter& painter, PaintEventBlockData& blockData) const;
    bool selectionVisible(int blockNumber) const;
    bool replacementVisible(int blockNumber) const;
    QString foldReplacementText(const QTextBlock&) const;
    QColor replacementPenColor(int blockNumber) const;
    void clearVisibleFoldedBlock();
    QTextBlock foldedBlockAt(const QPoint& pos, QRect* box = nullptr) const;

signals:
    void requestBlockUpdate(const QTextBlock &);
    void documentChanged();

private:
    bool event(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    bool viewportEvent(QEvent* event) override;
    void leaveEvent(QEvent* e) override;
    void wheelEvent(QWheelEvent* event) override;
    void scrollContentsBy(int dx, int dy) override;
    void showEvent(QShowEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;
    void focusInEvent(QFocusEvent* e) override;
    void changeEvent(QEvent* e) override;
    void timerEvent(QTimerEvent* e) override;

private:
    QmlCodeDocument* m_initialEmptyDocument;
    QLabel* m_noDocsLabel;
    RowBar* m_rowBar;
    QmlCodeEditorToolBar* m_toolBar;
    QRegExp m_searchExpr;
    Core::FindFlags m_findFlags;
    Core::Id m_tabSettingsId;
    bool m_linkPressed;
    bool m_fontSettingsNeedsApply;
    bool m_parenthesesMatchingEnabled;
    Utils::Link m_currentLink;
    QTextCursor m_pendingLinkUpdate;
    QTextCursor m_lastLinkUpdate;
    int m_oldCursorPosition;
    int m_visibleFoldedBlockNumber;
    QTimer* m_updateUsesTimer;
    QTimer* m_contextPaneTimer;
    QTimer* m_parenthesesMatchingTimer;
    QBasicTimer m_foldedBlockTimer;
    QBasicTimer m_cursorFlashTimer;
    bool m_animateAutoComplete = true;
    bool m_highlightAutoComplete = true;
    bool m_skipAutoCompletedText = true;
    bool m_removeAutoCompletedText = true;
    bool m_mouseOnFoldedMarker = false;
    QTextCursor m_findScopeStart;
    QTextCursor m_findScopeEnd;
    int m_suggestedVisibleFoldedBlockNumber = -1;
    QVector<SearchResult> m_searchResults;
    QFutureWatcher<Utils::FileSearchResultList> *m_searchWatcher = nullptr;
    int m_findScopeVerticalBlockSelectionFirstColumn = -1;
    int m_findScopeVerticalBlockSelectionLastColumn = -1;
    QList<QTextCursor> m_autoCompleteHighlightPos;
    QList<TextEditor::BaseHoverHandler*> m_hoverHandlers;
    HoverHandlerRunner* m_hoverHandlerRunner;
    TextEditor::CodeAssistant* m_codeAssistant;
    QPointer<TextEditorAnimator> m_bracketsAnimator;
    QPointer<TextEditorAnimator> m_autocompleteAnimator;
    TextEditor::Internal::TextEditorOverlay *m_overlay;
    TextEditor::ICodeStylePreferences *m_codeStylePreferences = nullptr;
    TextEditor::Internal::TextEditorOverlay *m_searchResultOverlay;
    QHash<QString, QList<QTextEdit::ExtraSelection>> m_extraSelections;
    QScopedPointer<QmlJSEditor::Internal::AutoCompleter> m_autoCompleter;
    static TextEditor::CompletionAssistProvider* m_completionAssistProvider;
    static QmlJSEditor::Internal::QmlJSHoverHandler* m_qmlJsHoverHandler;
    static TextEditor::ColorPreviewHoverHandler* m_colorPreviewHoverHandler;
    static QmlJSEditor::QuickToolBar* m_contextPane;
};

#endif // QMLCODEEDITOR_H