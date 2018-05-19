#ifndef QMLCODEDOCUMENT_H
#define QMLCODEDOCUMENT_H

#include <QTextDocument>
#include <qmljstools/qmljssemanticinfo.h>
#include <texteditor/fontsettings.h>
#include <texteditor/tabsettings.h>
#include <blockdata.h>
#include <texteditor/typingsettings.h>
#include <texteditor/storagesettings.h>

class QTimer;
class QPlainTextEdit;

namespace QmlJS { class DiagnosticMessage; }

namespace QmlJSEditor {
class QmlJSHighlighter;

namespace Internal {
class Indenter;
class SemanticInfoUpdater;
class SemanticHighlighter;
}
}

enum MatchType { NoMatch, Match, Mismatch  };

typedef QList<Mark *> Marks;

class QmlCodeDocument : public QTextDocument
{
    Q_OBJECT

    friend class QmlCodeEditor;

public:
    QmlCodeDocument(QPlainTextEdit* editor);
    ~QmlCodeDocument();

    void setFilePath(const QString& filePath);
    const QString& filePath() const;

    const QmlJSTools::SemanticInfo& semanticInfo() const;
    QmlJSEditor::QmlJSHighlighter* syntaxHighlighter() const;

    QmlJSEditor::Internal::Indenter* indenter() const;

    bool isSemanticInfoOutdated() const;
    QString textAt(int pos, int length);

    QVector<QTextLayout::FormatRange> diagnosticRanges() const;
    void setDiagnosticRanges(const QVector<QTextLayout::FormatRange> &ranges);

    const TextEditor::TabSettings& tabSettings() const;
    void setTabSettings(const TextEditor::TabSettings& tabSettings);

    TextEditor::FontSettings fontSettings() const;
    void setFontSettings(const TextEditor::FontSettings& fontSettings);

    const TextEditor::TypingSettings &typingSettings() const;
    void setTypingSettings(const TextEditor::TypingSettings &typingSettings);
    void setStorageSettings(const TextEditor::StorageSettings& storageSettings);

    void autoIndent(const QTextCursor &cursor, QChar typedChar = QChar::Null);
    void autoReindent(const QTextCursor &cursor);
    QTextCursor indent(const QTextCursor &cursor, bool blockSelection = false, int column = 0,
                       int *offset = nullptr);
    QTextCursor unindent(const QTextCursor &cursor, bool blockSelection = false, int column = 0,
                         int *offset = nullptr);

    static BlockData *testUserData(const QTextBlock &block) {
        return static_cast<BlockData*>(block.userData());
    }

    static BlockData* userData(const QTextBlock& block) {
        BlockData *data = static_cast<BlockData*>(block.userData());
        if (!data && block.isValid())
            const_cast<QTextBlock &>(block).setUserData((data = new BlockData));
        return data;
    }

    static int braceDepth(const QTextBlock &block);
    static void setLexerState(const QTextBlock &block, int state);
    static int lexerState(const QTextBlock &block);
    static void setParentheses(const QTextBlock &block, const Parentheses &parentheses);
    static bool hasParentheses(const QTextBlock &block);
    static Parentheses parentheses(const QTextBlock &block);
    static MatchType matchCursorForward(QTextCursor* cursor);
    static MatchType matchCursorBackward(QTextCursor* cursor);
    static MatchType checkOpenParenthesis(QTextCursor* cursor, QChar c);
    static MatchType checkClosedParenthesis(QTextCursor* cursor, QChar c);
    static bool findNextBlockClosingParenthesis(QTextCursor* cursor);
    static bool findPreviousBlockOpenParenthesis(QTextCursor* cursor, bool checkStartPosition = false);

signals:
    void tabSettingsChanged();
    void fontSettingsChanged();
    void updateCodeWarnings(QmlJS::Document::Ptr doc);
    void semanticInfoUpdated(const QmlJSTools::SemanticInfo& semanticInfo);

protected:
    void applyFontSettings();
    void triggerPendingUpdates();

private slots:
    void reparseDocument();
    void onDocumentUpdated(QmlJS::Document::Ptr doc);
    void reupdateSemanticInfo();
    void acceptNewSemanticInfo(const QmlJSTools::SemanticInfo& semanticInfo);

private:
    QTextCursor indentOrUnindent(const QTextCursor &textCursor, bool doIndent, bool blockSelection,
                                 int columnIn, int *offset);
    void cleanDiagnosticMarks();
    void createMarks(const QmlJSTools::SemanticInfo& info);
    void createMarks(const QList<QmlJS::DiagnosticMessage>& diagnostics);
    void cleanSemanticMarks();

private:
    QPlainTextEdit* m_editor;
    QString m_filePath;
    TextEditor::TabSettings m_tabSettings;
    TextEditor::FontSettings m_fontSettings;
    TextEditor::TypingSettings m_typingSettings;
    TextEditor::StorageSettings m_storageSettings;
    bool m_fontSettingsNeedsApply;
    QmlJSEditor::Internal::Indenter* m_indenter;
    QmlJSEditor::QmlJSHighlighter* m_syntaxHighlighter;
    QTimer* m_updateDocumentTimer;
    QTimer* m_reupdateSemanticInfoTimer;
    int m_semanticInfoDocRevision;
    QmlJSEditor::Internal::SemanticInfoUpdater* m_semanticInfoUpdater;
    QmlJSTools::SemanticInfo m_semanticInfo;
    QVector<QTextLayout::FormatRange> m_diagnosticRanges;
    QmlJSEditor::Internal::SemanticHighlighter* m_semanticHighlighter;
    bool m_semanticHighlightingNecessary;
    QVector<Mark *> m_diagnosticMarks;
    QVector<Mark *> m_semanticMarks;
};

#endif // QMLCODEDOCUMENT_H