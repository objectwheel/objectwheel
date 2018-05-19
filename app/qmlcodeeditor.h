#ifndef QMLCODEEDITOR_H
#define QMLCODEEDITOR_H

#include <QPlainTextEdit>
#include <QTimeLine>
#include <QPointer>

#include <utils/link.h>
#include <texteditor/codeassist/assistenums.h>
#include <texteditor/texteditorconstants.h>
#include <qmljs/qmljsdocument.h>
#include <qmljseditor/qmljsautocompleter.h>
#include <coreplugin/id.h>
#include <texteditor/behaviorsettings.h>

class RowBar;
class QmlCodeDocument;
class HoverHandlerRunner;
struct PaintEventData;

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


class QmlCodeEditor : public QPlainTextEdit
{
    Q_OBJECT

    friend class RowBar;
    friend class MarkBand;
    friend class BracketBand;
    friend class BreakpointBand;
    friend class LineNumberBand;
    friend class TextEditor::Internal::TextEditorOverlay;

public:
    enum FindFlag {
        FindBackward = 0x01,
        FindCaseSensitively = 0x02,
        FindWholeWords = 0x04,
        FindRegularExpression = 0x08,
        FindPreserveCase = 0x10
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)

public:
    explicit QmlCodeEditor(QWidget* parent = nullptr);
    ~QmlCodeEditor();

    void setCodeDocument(QmlCodeDocument* document);
    QmlCodeDocument* codeDocument() const;

    RowBar* rowBar() const;

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
    void setMouseNavigationEnabled(bool b);
    bool mouseNavigationEnabled() const;
    void setMouseHidingEnabled(bool b);
    bool mouseHidingEnabled() const;
    void setScrollWheelZoomingEnabled(bool b);
    bool scrollWheelZoomingEnabled() const;
    void setConstrainTooltips(bool b);
    bool constrainTooltips() const;
    void setCamelCaseNavigationEnabled(bool b);
    bool camelCaseNavigationEnabled() const;
    void setBehaviorSettings(const TextEditor::BehaviorSettings& bs);
    const TextEditor::BehaviorSettings& behaviorSettings() const;

public slots:
    void indent();
    void unindent();
    void updateUses();
    void updateTabStops();
    void slotSelectionChanged();
    void updateAutoCompleteHighlight();
    void editorContentsChange(int, int, int);
    void highlightSearchResultsSlot(const QString& txt, FindFlags findFlags);
    void semanticInfoUpdated(const QmlJSTools::SemanticInfo& semanticInfo);
    void animateUpdate(const QTextCursor &cursor, QPointF lastPos, QRectF rect);
    void slotCodeStyleSettingsChanged(const QVariant&);

private:
    void updateRowBarWidth();
    void cancelCurrentAnimations();
    bool cursorMoveKeyEvent(QKeyEvent* e);
    void updateRowBar(const QRect &rect, int dy);
    void paintOverlays(const PaintEventData &data, QPainter &painter) const;
    void processTooltipRequest(const QTextCursor &c);
    QPoint toolTipPosition(const QTextCursor &c) const;
    void drawCollapsedBlockPopup(QPainter &painter, const QTextBlock &block, QPointF offset,
                                 const QRect &clip);
    void paintSearchResultOverlay(const PaintEventData& data, QPainter& painter);
    QTextBlock nextVisibleBlock(const QTextBlock& block, const QTextDocument* doc);
    void paintCurrentLineHighlight(const PaintEventData& data, QPainter& painter) const;
    void highlightSearchResults(const QTextBlock& block, TextEditor::Internal::TextEditorOverlay* overlay);
    QString wordUnderCursor() const;
    void gotoLine(int line, int column = 0, bool centerLine = true, bool animate = true);
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

private:
    RowBar* m_rowBar;
    QRegExp m_searchExpr;
    FindFlags m_findFlags;
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
    bool m_animateAutoComplete = true;
    bool m_highlightAutoComplete = true;
    bool m_skipAutoCompletedText = true;
    bool m_removeAutoCompletedText = true;
    TextEditor::BehaviorSettings m_behaviorSettings;
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