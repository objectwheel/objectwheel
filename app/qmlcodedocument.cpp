#include <qmlcodedocument.h>

#include <qmljs/qmljsmodelmanagerinterface.h>
#include <qmljseditor/qmljssemanticinfoupdater.h>
#include <qmljseditor/qmljssemantichighlighter.h>
#include <qmljseditor/qmljshighlighter.h>
#include <qmljs/parser/qmljsast_p.h>
#include <qmljstools/qmljsindenter.h>
#include <utils/textutils.h>
#include <texteditor/tabsettings.h>
#include <qtcassert.h>
#include <documentmanager.h>

#include <QPlainTextEdit>
#include <QTimer>
#include <QTextCursor>

using namespace QmlJS;
using namespace QmlJS::AST;
using namespace QmlJSTools;
using namespace QmlJSEditor;
using namespace QmlJSEditor::Internal;

namespace {

enum {
    UPDATE_DOCUMENT_DEFAULT_INTERVAL = 100
};

struct Declaration
{
    QString text;
    int startLine;
    int startColumn;
    int endLine;
    int endColumn;

    Declaration()
        : startLine(0),
          startColumn(0),
          endLine(0),
          endColumn(0)
    { }
};

class FindIdDeclarations: protected Visitor
{
public:
    typedef QHash<QString, QList<AST::SourceLocation> > Result;

    Result operator()(Document::Ptr doc)
    {
        _ids.clear();
        _maybeIds.clear();
        if (doc && doc->qmlProgram())
            doc->qmlProgram()->accept(this);
        return _ids;
    }

protected:
    QString asString(AST::UiQualifiedId *id)
    {
        QString text;
        for (; id; id = id->next) {
            if (!id->name.isEmpty())
                text += id->name;
            else
                text += QLatin1Char('?');

            if (id->next)
                text += QLatin1Char('.');
        }

        return text;
    }

    void accept(AST::Node *node)
    { AST::Node::acceptChild(node, this); }

    using Visitor::visit;
    using Visitor::endVisit;

    virtual bool visit(AST::UiScriptBinding *node)
    {
        if (asString(node->qualifiedId) == QLatin1String("id")) {
            if (AST::ExpressionStatement *stmt = AST::cast<AST::ExpressionStatement*>(node->statement)) {
                if (AST::IdentifierExpression *idExpr = AST::cast<AST::IdentifierExpression *>(stmt->expression)) {
                    if (!idExpr->name.isEmpty()) {
                        const QString &id = idExpr->name.toString();
                        QList<AST::SourceLocation> *locs = &_ids[id];
                        locs->append(idExpr->firstSourceLocation());
                        locs->append(_maybeIds.value(id));
                        _maybeIds.remove(id);
                        return false;
                    }
                }
            }
        }

        accept(node->statement);

        return false;
    }

    virtual bool visit(AST::IdentifierExpression *node)
    {
        if (!node->name.isEmpty()) {
            const QString &name = node->name.toString();

            if (_ids.contains(name))
                _ids[name].append(node->identifierToken);
            else
                _maybeIds[name].append(node->identifierToken);
        }
        return false;
    }

private:
    Result _ids;
    Result _maybeIds;
};

class FindDeclarations: protected Visitor
{
    QList<Declaration> _declarations;
    int _depth;

public:
    QList<Declaration> operator()(AST::Node *node)
    {
        _depth = -1;
        _declarations.clear();
        accept(node);
        return _declarations;
    }

protected:
    using Visitor::visit;
    using Visitor::endVisit;

    QString asString(AST::UiQualifiedId *id)
    {
        QString text;
        for (; id; id = id->next) {
            if (!id->name.isEmpty())
                text += id->name;
            else
                text += QLatin1Char('?');

            if (id->next)
                text += QLatin1Char('.');
        }

        return text;
    }

    void accept(AST::Node *node)
    { AST::Node::acceptChild(node, this); }

    void init(Declaration *decl, AST::UiObjectMember *member)
    {
        const SourceLocation first = member->firstSourceLocation();
        const SourceLocation last = member->lastSourceLocation();
        decl->startLine = first.startLine;
        decl->startColumn = first.startColumn;
        decl->endLine = last.startLine;
        decl->endColumn = last.startColumn + last.length;
    }

    void init(Declaration *decl, AST::ExpressionNode *expressionNode)
    {
        const SourceLocation first = expressionNode->firstSourceLocation();
        const SourceLocation last = expressionNode->lastSourceLocation();
        decl->startLine = first.startLine;
        decl->startColumn = first.startColumn;
        decl->endLine = last.startLine;
        decl->endColumn = last.startColumn + last.length;
    }

    virtual bool visit(AST::UiObjectDefinition *node)
    {
        ++_depth;

        Declaration decl;
        init(&decl, node);

        decl.text.fill(QLatin1Char(' '), _depth);
        if (node->qualifiedTypeNameId)
            decl.text.append(asString(node->qualifiedTypeNameId));
        else
            decl.text.append(QLatin1Char('?'));

        _declarations.append(decl);

        return true; // search for more bindings
    }

    virtual void endVisit(AST::UiObjectDefinition *)
    {
        --_depth;
    }

    virtual bool visit(AST::UiObjectBinding *node)
    {
        ++_depth;

        Declaration decl;
        init(&decl, node);

        decl.text.fill(QLatin1Char(' '), _depth);

        decl.text.append(asString(node->qualifiedId));
        decl.text.append(QLatin1String(": "));

        if (node->qualifiedTypeNameId)
            decl.text.append(asString(node->qualifiedTypeNameId));
        else
            decl.text.append(QLatin1Char('?'));

        _declarations.append(decl);

        return true; // search for more bindings
    }

    virtual void endVisit(AST::UiObjectBinding *)
    {
        --_depth;
    }

    virtual bool visit(AST::UiScriptBinding *)
    {
        ++_depth;

#if 0 // ### ignore script bindings for now.
        Declaration decl;
        init(&decl, node);

        decl.text.fill(QLatin1Char(' '), _depth);
        decl.text.append(asString(node->qualifiedId));

        _declarations.append(decl);
#endif

        return false; // more more bindings in this subtree.
    }

    virtual void endVisit(AST::UiScriptBinding *)
    {
        --_depth;
    }

    virtual bool visit(AST::FunctionExpression *)
    {
        return false;
    }

    virtual bool visit(AST::FunctionDeclaration *ast)
    {
        if (ast->name.isEmpty())
            return false;

        Declaration decl;
        init(&decl, ast);

        decl.text.fill(QLatin1Char(' '), _depth);
        decl.text += ast->name;

        decl.text += QLatin1Char('(');
        for (FormalParameterList *it = ast->formals; it; it = it->next) {
            if (!it->name.isEmpty())
                decl.text += it->name;

            if (it->next)
                decl.text += QLatin1String(", ");
        }

        decl.text += QLatin1Char(')');

        _declarations.append(decl);

        return false;
    }

    virtual bool visit(AST::VariableDeclaration *ast)
    {
        if (ast->name.isEmpty())
            return false;

        Declaration decl;
        decl.text.fill(QLatin1Char(' '), _depth);
        decl.text += ast->name;

        const SourceLocation first = ast->identifierToken;
        decl.startLine = first.startLine;
        decl.startColumn = first.startColumn;
        decl.endLine = first.startLine;
        decl.endColumn = first.startColumn + first.length;

        _declarations.append(decl);

        return false;
    }

    bool visit(AST::BinaryExpression *ast)
    {
        AST::FieldMemberExpression *field = AST::cast<AST::FieldMemberExpression *>(ast->left);
        AST::FunctionExpression *funcExpr = AST::cast<AST::FunctionExpression *>(ast->right);

        if (field && funcExpr && funcExpr->body && (ast->op == QSOperator::Assign)) {
            Declaration decl;
            init(&decl, ast);

            decl.text.fill(QLatin1Char(' '), _depth);
            decl.text += field->name;

            decl.text += QLatin1Char('(');
            for (FormalParameterList *it = funcExpr->formals; it; it = it->next) {
                if (!it->name.isEmpty())
                    decl.text += it->name;

                if (it->next)
                    decl.text += QLatin1String(", ");
            }
            decl.text += QLatin1Char(')');

            _declarations.append(decl);
        }

        return true;
    }
};

class CreateRanges: protected AST::Visitor
{
    QTextDocument *_textDocument;
    QList<Range> _ranges;

public:
    QList<Range> operator()(QTextDocument *textDocument, Document::Ptr doc)
    {
        _textDocument = textDocument;
        _ranges.clear();
        if (doc && doc->ast() != 0)
            doc->ast()->accept(this);
        return _ranges;
    }

protected:
    using AST::Visitor::visit;

    virtual bool visit(AST::UiObjectBinding *ast)
    {
        if (ast->initializer && ast->initializer->lbraceToken.length)
            _ranges.append(createRange(ast, ast->initializer));
        return true;
    }

    virtual bool visit(AST::UiObjectDefinition *ast)
    {
        if (ast->initializer && ast->initializer->lbraceToken.length)
            _ranges.append(createRange(ast, ast->initializer));
        return true;
    }

    virtual bool visit(AST::FunctionExpression *ast)
    {
        _ranges.append(createRange(ast));
        return true;
    }

    virtual bool visit(AST::FunctionDeclaration *ast)
    {
        _ranges.append(createRange(ast));
        return true;
    }

    bool visit(AST::BinaryExpression *ast)
    {
        auto field = AST::cast<AST::FieldMemberExpression *>(ast->left);
        auto funcExpr = AST::cast<AST::FunctionExpression *>(ast->right);

        if (field && funcExpr && funcExpr->body && (ast->op == QSOperator::Assign))
            _ranges.append(createRange(ast, ast->firstSourceLocation(), ast->lastSourceLocation()));
        return true;
    }

    virtual bool visit(AST::UiScriptBinding *ast)
    {
        if (AST::Block *block = AST::cast<AST::Block *>(ast->statement))
            _ranges.append(createRange(ast, block));
        return true;
    }

    Range createRange(AST::UiObjectMember *member, AST::UiObjectInitializer *ast)
    {
        return createRange(member, member->firstSourceLocation(), ast->rbraceToken);
    }

    Range createRange(AST::FunctionExpression *ast)
    {
        return createRange(ast, ast->lbraceToken, ast->rbraceToken);
    }

    Range createRange(AST::UiScriptBinding *ast, AST::Block *block)
    {
        return createRange(ast, block->lbraceToken, block->rbraceToken);
    }

    Range createRange(AST::Node *ast, AST::SourceLocation start, AST::SourceLocation end)
    {
        Range range;

        range.ast = ast;

        range.begin = QTextCursor(_textDocument);
        range.begin.setPosition(start.begin());

        range.end = QTextCursor(_textDocument);
        range.end.setPosition(end.end());

        return range;
    }
};

}

QmlCodeDocument::QmlCodeDocument(QPlainTextEdit* editor) : m_editor(editor)
  , m_fontSettingsNeedsApply(false)
  , m_indenter(new Indenter)
  , m_syntaxHighlighter(new QmlJSHighlighter(this))
  , m_updateDocumentTimer(new QTimer(this))
  , m_reupdateSemanticInfoTimer(new QTimer(this))
  , m_semanticInfoDocRevision(-1)
  , m_semanticInfoUpdater(new SemanticInfoUpdater(this))
  , m_semanticHighlighter(new QmlJSEditor::Internal::SemanticHighlighter(this))
  , m_semanticHighlightingNecessary(false)
{
    DocumentManager::addDocument(this);

    // set new document layout
    QTextOption opt = defaultTextOption();
    opt.setTextDirection(Qt::LeftToRight);
    opt.setFlags(opt.flags()
                 | QTextOption::IncludeTrailingSpaces
                 | QTextOption::AddSpaceForLineAndParagraphSeparators);
    setDefaultTextOption(opt);
    setDocumentLayout(new QPlainTextDocumentLayout(this));

    ModelManagerInterface *modelManager = ModelManagerInterface::instance();

    // code model
    m_updateDocumentTimer->setInterval(UPDATE_DOCUMENT_DEFAULT_INTERVAL);
    m_updateDocumentTimer->setSingleShot(true);
    connect(this, &QTextDocument::contentsChanged,
            m_updateDocumentTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(m_updateDocumentTimer, &QTimer::timeout,
            this, &QmlCodeDocument::reparseDocument);
    connect(modelManager, &ModelManagerInterface::documentUpdated,
            this, &QmlCodeDocument::onDocumentUpdated);

    // semantic info
    m_semanticInfoUpdater = new SemanticInfoUpdater(this);
    connect(m_semanticInfoUpdater, &SemanticInfoUpdater::updated,
            this, &QmlCodeDocument::acceptNewSemanticInfo);
    m_semanticInfoUpdater->start();

    // library info changes
    m_reupdateSemanticInfoTimer->setInterval(UPDATE_DOCUMENT_DEFAULT_INTERVAL);
    m_reupdateSemanticInfoTimer->setSingleShot(true);
    connect(m_reupdateSemanticInfoTimer, &QTimer::timeout,
            this, &QmlCodeDocument::reupdateSemanticInfo);
    connect(modelManager, &ModelManagerInterface::libraryInfoUpdated,
            m_reupdateSemanticInfoTimer, static_cast<void (QTimer::*)()>(&QTimer::start));

    modelManager->updateSourceFiles(QStringList(filePath()), false);
}

QmlCodeDocument::~QmlCodeDocument()
{
    DocumentManager::removeDocument(this);
    m_semanticInfoUpdater->abort();
    m_semanticInfoUpdater->wait();
    cleanDiagnosticMarks();
    cleanSemanticMarks();
}

void QmlCodeDocument::reparseDocument()
{
    ModelManagerInterface::instance()->updateSourceFiles(QStringList(filePath()), false);
}

void QmlCodeDocument::onDocumentUpdated(Document::Ptr doc)
{
    if (filePath() != doc->fileName())
        return;

    // text document has changed, simply wait for the next onDocumentUpdated
    if (doc->editorRevision() != revision())
        return;

    cleanDiagnosticMarks();
    if (doc->ast()) {
        // got a correctly parsed (or recovered) file.
        m_semanticInfoDocRevision = doc->editorRevision();
        m_semanticInfoUpdater->update(doc, ModelManagerInterface::instance()->snapshot());
    } else if (doc->language().isFullySupportedLanguage()) {
        createMarks(doc->diagnosticMessages());
    }
    emit updateCodeWarnings(doc);
}

void QmlCodeDocument::acceptNewSemanticInfo(const SemanticInfo& semanticInfo)
{
    if (semanticInfo.revision() != revision()) {
        // ignore outdated semantic infos
        return;
    }

    m_semanticInfo = semanticInfo;
    Document::Ptr doc = semanticInfo.document;

    // create the ranges
    CreateRanges createRanges;
    m_semanticInfo.ranges = createRanges(this, doc);

    // Refresh the ids
    FindIdDeclarations updateIds;
    m_semanticInfo.idLocations = updateIds(doc);

    m_semanticHighlightingNecessary = true;

    createMarks(m_semanticInfo);
    emit semanticInfoUpdated(m_semanticInfo); // calls triggerPendingUpdates as necessary
}

void QmlCodeDocument::reupdateSemanticInfo()
{
    // If the editor is newer than the semantic info (possibly with update in progress),
    // new semantic infos won't be accepted anyway. We'll get a onDocumentUpdated anyhow.
    if (revision() != m_semanticInfoDocRevision)
        return;

    m_semanticInfoUpdater->reupdate(ModelManagerInterface::instance()->snapshot());
}

QTextCursor QmlCodeDocument::indentOrUnindent(const QTextCursor &textCursor, bool doIndent,
                                              bool blockSelection, int columnIn, int *offset)
{
    QTextCursor cursor = textCursor;
    cursor.beginEditBlock();

    TextEditor::TabSettings &ts = m_tabSettings;

    // Indent or unindent the selected lines
    int pos = cursor.position();
    int column = blockSelection ? columnIn
                                : ts.columnAt(cursor.block().text(), cursor.positionInBlock());
    int anchor = cursor.anchor();
    int start = qMin(anchor, pos);
    int end = qMax(anchor, pos);
    bool modified = true;

    QTextBlock startBlock = findBlock(start);
    QTextBlock endBlock = findBlock(blockSelection ? end : qMax(end - 1, 0)).next();
    const bool cursorAtBlockStart = (textCursor.position() == startBlock.position());
    const bool anchorAtBlockStart = (textCursor.anchor() == startBlock.position());
    const bool oneLinePartial = (startBlock.next() == endBlock)
            && (start > startBlock.position() || end < endBlock.position() - 1);

    // Make sure one line selection will get processed in "for" loop
    if (startBlock == endBlock)
        endBlock = endBlock.next();

    if (cursor.hasSelection() && !blockSelection && !oneLinePartial) {
        for (QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            const QString text = block.text();
            int indentPosition = ts.lineIndentPosition(text);
            if (!doIndent && !indentPosition)
                indentPosition = ts.firstNonSpace(text);
            int targetColumn = ts.indentedColumn(ts.columnAt(text, indentPosition), doIndent);
            cursor.setPosition(block.position() + indentPosition);
            cursor.insertText(ts.indentationString(0, targetColumn, 0, block));
            cursor.setPosition(block.position());
            cursor.setPosition(block.position() + indentPosition, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        }
        // make sure that selection that begins in first column stays at first column
        // even if we insert text at first column
        if (cursorAtBlockStart) {
            cursor = textCursor;
            cursor.setPosition(startBlock.position(), QTextCursor::KeepAnchor);
        } else if (anchorAtBlockStart) {
            cursor = textCursor;
            cursor.setPosition(startBlock.position(), QTextCursor::MoveAnchor);
            cursor.setPosition(textCursor.position(), QTextCursor::KeepAnchor);
        } else {
            modified = false;
        }
    } else if (cursor.hasSelection() && !blockSelection && oneLinePartial) {
        // Only one line partially selected.
        cursor.removeSelectedText();
    } else {
        // Indent or unindent at cursor position
        for (QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            QString text = block.text();

            int blockColumn = ts.columnAt(text, text.size());
            if (blockColumn < column) {
                cursor.setPosition(block.position() + text.size());
                cursor.insertText(ts.indentationString(blockColumn, column, 0, block));
                text = block.text();
            }

            int indentPosition = ts.positionAtColumn(text, column, nullptr, true);
            int spaces = ts.spacesLeftFromPosition(text, indentPosition);
            int startColumn = ts.columnAt(text, indentPosition - spaces);
            int targetColumn = ts.indentedColumn(ts.columnAt(text, indentPosition), doIndent);
            cursor.setPosition(block.position() + indentPosition);
            cursor.setPosition(block.position() + indentPosition - spaces, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            cursor.insertText(ts.indentationString(startColumn, targetColumn, 0, block));
        }
        // Preserve initial anchor of block selection
        if (blockSelection) {
            end = cursor.position();
            if (offset)
                *offset = ts.columnAt(cursor.block().text(), cursor.positionInBlock()) - column;
            cursor.setPosition(start);
            cursor.setPosition(end, QTextCursor::KeepAnchor);
        }
    }

    cursor.endEditBlock();

    return modified ? cursor : textCursor;
}


QmlJSEditor::Internal::Indenter* QmlCodeDocument::indenter() const
{
    return m_indenter;
}

TextEditor::FontSettings QmlCodeDocument::fontSettings() const
{
    return m_fontSettings;
}

void QmlCodeDocument::setFontSettings(const TextEditor::FontSettings& fontSettings)
{
    if (fontSettings == m_fontSettings)
        return;
    m_fontSettings = fontSettings;
    m_fontSettingsNeedsApply = true;
    emit fontSettingsChanged();
}

void QmlCodeDocument::setStorageSettings(const TextEditor::StorageSettings& storageSettings)
{
    m_storageSettings = storageSettings;
}

const TextEditor::TypingSettings& QmlCodeDocument::typingSettings() const
{
    return m_typingSettings;
}

void QmlCodeDocument::setTypingSettings(const TextEditor::TypingSettings& typingSettings)
{
    m_typingSettings = typingSettings;
}

void QmlCodeDocument::autoIndent(const QTextCursor &cursor, QChar typedChar)
{
    m_indenter->indent(this, cursor, typedChar, m_tabSettings);
}

void QmlCodeDocument::autoReindent(const QTextCursor &cursor)
{
    m_indenter->reindent(this, cursor, m_tabSettings);
}

QTextCursor QmlCodeDocument::indent(const QTextCursor &cursor, bool blockSelection, int column,
                                    int *offset)
{
    return indentOrUnindent(cursor, true, blockSelection, column, offset);
}

QTextCursor QmlCodeDocument::unindent(const QTextCursor &cursor, bool blockSelection, int column,
                                      int *offset)
{
    return indentOrUnindent(cursor, false, blockSelection, column, offset);
}

void QmlCodeDocument::setLexerState(const QTextBlock& block, int state)
{
    if (state == 0) {
        if (BlockData* userData = QmlCodeDocument::testUserData(block))
            userData->lexerState = 0;
    } else {
        userData(block)->lexerState = qMax(0,state);
    }
}

int QmlCodeDocument::lexerState(const QTextBlock& block)
{
    if (BlockData* userData = QmlCodeDocument::userData(block))
        return userData->lexerState;
    return 0;
}

void QmlCodeDocument::setParentheses(const QTextBlock& block, const Parentheses& parentheses)
{
    if (parentheses.isEmpty()) {
        if (BlockData *userData = testUserData(block))
            userData->parentheses.clear();
    } else {
        userData(block)->parentheses = parentheses;
    }
}

MatchType QmlCodeDocument::matchCursorBackward(QTextCursor *cursor)
{
    cursor->clearSelection();
    const QTextBlock block = cursor->block();

    if (!QmlCodeDocument::hasParentheses(block))
        return NoMatch;

    const int relPos = cursor->position() - block.position();

    Parentheses parentheses = QmlCodeDocument::parentheses(block);
    const Parentheses::const_iterator cend = parentheses.constEnd();
    for (Parentheses::const_iterator it = parentheses.constBegin();it != cend; ++it) {
        const Parenthesis &paren = *it;
        if (paren.pos == relPos - 1
            && paren.type == Parenthesis::Closed) {
            return checkClosedParenthesis(cursor, paren.chr);
        }
    }
    return NoMatch;
}

MatchType QmlCodeDocument::checkOpenParenthesis(QTextCursor *cursor, QChar c)
{
    QTextBlock block = cursor->block();
    if (!QmlCodeDocument::hasParentheses(block))
        return NoMatch;

    Parentheses parenList = QmlCodeDocument::parentheses(block);
    Parenthesis openParen, closedParen;
    QTextBlock closedParenParag = block;

    const int cursorPos = cursor->position() - closedParenParag.position();
    int i = 0;
    int ignore = 0;
    bool foundOpen = false;
    for (;;) {
        if (!foundOpen) {
            if (i >= parenList.count())
                return NoMatch;
            openParen = parenList.at(i);
            if (openParen.pos != cursorPos) {
                ++i;
                continue;
            } else {
                foundOpen = true;
                ++i;
            }
        }

        if (i >= parenList.count()) {
            for (;;) {
                closedParenParag = closedParenParag.next();
                if (!closedParenParag.isValid())
                    return NoMatch;
                if (QmlCodeDocument::hasParentheses(closedParenParag)) {
                    parenList = QmlCodeDocument::parentheses(closedParenParag);
                    break;
                }
            }
            i = 0;
        }

        closedParen = parenList.at(i);
        if (closedParen.type == Parenthesis::Opened) {
            ignore++;
            ++i;
            continue;
        } else {
            if (ignore > 0) {
                ignore--;
                ++i;
                continue;
            }

            cursor->clearSelection();
            cursor->setPosition(closedParenParag.position() + closedParen.pos + 1, QTextCursor::KeepAnchor);

            if ((c == QLatin1Char('{') && closedParen.chr != QLatin1Char('}'))
                || (c == QLatin1Char('(') && closedParen.chr != QLatin1Char(')'))
                || (c == QLatin1Char('[') && closedParen.chr != QLatin1Char(']'))
                || (c == QLatin1Char('+') && closedParen.chr != QLatin1Char('-'))
               )
                return Mismatch;

            return Match;
        }
    }
}

MatchType QmlCodeDocument::checkClosedParenthesis(QTextCursor *cursor, QChar c)
{
    QTextBlock block = cursor->block();
    if (!QmlCodeDocument::hasParentheses(block))
        return NoMatch;

    Parentheses parenList = QmlCodeDocument::parentheses(block);
    Parenthesis openParen, closedParen;
    QTextBlock openParenParag = block;

    const int cursorPos = cursor->position() - openParenParag.position();
    int i = parenList.count() - 1;
    int ignore = 0;
    bool foundClosed = false;
    for (;;) {
        if (!foundClosed) {
            if (i < 0)
                return NoMatch;
            closedParen = parenList.at(i);
            if (closedParen.pos != cursorPos - 1) {
                --i;
                continue;
            } else {
                foundClosed = true;
                --i;
            }
        }

        if (i < 0) {
            for (;;) {
                openParenParag = openParenParag.previous();
                if (!openParenParag.isValid())
                    return NoMatch;

                if (QmlCodeDocument::hasParentheses(openParenParag)) {
                    parenList = QmlCodeDocument::parentheses(openParenParag);
                    break;
                }
            }
            i = parenList.count() - 1;
        }

        openParen = parenList.at(i);
        if (openParen.type == Parenthesis::Closed) {
            ignore++;
            --i;
            continue;
        } else {
            if (ignore > 0) {
                ignore--;
                --i;
                continue;
            }

            cursor->clearSelection();
            cursor->setPosition(openParenParag.position() + openParen.pos, QTextCursor::KeepAnchor);

            if ((c == QLatin1Char('}') && openParen.chr != QLatin1Char('{'))    ||
                 (c == QLatin1Char(')') && openParen.chr != QLatin1Char('('))   ||
                 (c == QLatin1Char(']') && openParen.chr != QLatin1Char('['))   ||
                 (c == QLatin1Char('-') && openParen.chr != QLatin1Char('+')))
                return Mismatch;

            return Match;
        }
    }
}

bool QmlCodeDocument::findPreviousBlockOpenParenthesis(QTextCursor *cursor, bool checkStartPosition)
{
    QTextBlock block = cursor->block();
    int position = cursor->position();
    int ignore = 0;
    while (block.isValid()) {
        Parentheses parenList = QmlCodeDocument::parentheses(block);
        if (!parenList.isEmpty() /*&& !TextDocumentLayout::ifdefedOut(block)*/) {
            for (int i = parenList.count()-1; i >= 0; --i) {
                Parenthesis paren = parenList.at(i);
                if (paren.chr != QLatin1Char('+') && paren.chr != QLatin1Char('-'))
                    continue;
                if (block == cursor->block()) {
                    if (position - block.position() <= paren.pos + (paren.type == Parenthesis::Closed ? 1 : 0))
                        continue;
                    if (checkStartPosition && paren.type == Parenthesis::Opened && paren.pos== cursor->position())
                        return true;
                }
                if (paren.type == Parenthesis::Closed) {
                    ++ignore;
                } else if (ignore > 0) {
                    --ignore;
                } else {
                    cursor->setPosition(block.position() + paren.pos);
                    return true;
                }
            }
        }
        block = block.previous();
    }
    return false;
}

bool QmlCodeDocument::findNextBlockClosingParenthesis(QTextCursor *cursor)
{
    QTextBlock block = cursor->block();
    int position = cursor->position();
    int ignore = 0;
    while (block.isValid()) {
        Parentheses parenList = QmlCodeDocument::parentheses(block);
        if (!parenList.isEmpty() /*&& !TextDocumentLayout::ifdefedOut(block)*/) {
            for (int i = 0; i < parenList.count(); ++i) {
                Parenthesis paren = parenList.at(i);
                if (paren.chr != QLatin1Char('+') && paren.chr != QLatin1Char('-'))
                    continue;
                if (block == cursor->block() &&
                    (position - block.position() > paren.pos - (paren.type == Parenthesis::Opened ? 1 : 0)))
                    continue;
                if (paren.type == Parenthesis::Opened) {
                    ++ignore;
                } else if (ignore > 0) {
                    --ignore;
                } else {
                    cursor->setPosition(block.position() + paren.pos+1);
                    return true;
                }
            }
        }
        block = block.next();
    }
    return false;
}

int QmlCodeDocument::braceDepth(const QTextBlock& block)
{
    int state = block.userState();
    if (state == -1)
        return 0;
    return state >> 8;
}

bool QmlCodeDocument::hasParentheses(const QTextBlock& block)
{
    if (BlockData *userData = testUserData(block))
        return !userData->parentheses.isEmpty();
    return false;
}

Parentheses QmlCodeDocument::parentheses(const QTextBlock& block)
{
    if (BlockData *userData = testUserData(block))
        return userData->parentheses;
    return Parentheses();
}

MatchType QmlCodeDocument::matchCursorForward(QTextCursor *cursor)
{
    cursor->clearSelection();
    const QTextBlock block = cursor->block();

    if (!QmlCodeDocument::hasParentheses(block))
        return NoMatch;

    const int relPos = cursor->position() - block.position();

    Parentheses parentheses = QmlCodeDocument::parentheses(block);
    const Parentheses::const_iterator cend = parentheses.constEnd();
    for (Parentheses::const_iterator it = parentheses.constBegin();it != cend; ++it) {
        const Parenthesis &paren = *it;
        if (paren.pos == relPos
            && paren.type == Parenthesis::Opened) {
            return checkOpenParenthesis(cursor, paren.chr);
        }
    }
    return NoMatch;
}

const TextEditor::TabSettings& QmlCodeDocument::tabSettings() const
{
    return m_tabSettings;
}

void QmlCodeDocument::setTabSettings(const TextEditor::TabSettings &tabSettings)
{
    if (tabSettings == m_tabSettings)
        return;
    m_tabSettings = tabSettings;

//    if (Highlighter *highlighter = qobject_cast<Highlighter *>(d->m_highlighter))
//        highlighter->setTabSettings(tabSettings);

    emit tabSettingsChanged();
}


void QmlCodeDocument::setFilePath(const QString& filePath)
{
    m_filePath = filePath;
}

const QString& QmlCodeDocument::filePath() const
{
    return m_filePath;
}

const QmlJSTools::SemanticInfo& QmlCodeDocument::semanticInfo() const
{
    return m_semanticInfo;
}

QmlJSHighlighter* QmlCodeDocument::syntaxHighlighter() const
{
    return m_syntaxHighlighter;
}

bool QmlCodeDocument::isSemanticInfoOutdated() const
{
    return m_semanticInfo.revision() != revision();
}

QString QmlCodeDocument::textAt(int pos, int length)
{
    return Utils::Text::textAt(QTextCursor(this), pos, length);
}

QVector<QTextLayout::FormatRange> QmlCodeDocument::diagnosticRanges() const
{
    return m_diagnosticRanges;
}

void QmlCodeDocument::setDiagnosticRanges(const QVector<QTextLayout::FormatRange> &ranges)
{
    m_diagnosticRanges = ranges;
}

void QmlCodeDocument::applyFontSettings()
{
    m_fontSettingsNeedsApply = false;

    setDefaultFont(m_fontSettings.font());

    if (m_syntaxHighlighter) {
        m_syntaxHighlighter->setFontSettings(m_fontSettings);
        m_syntaxHighlighter->rehighlight();
    }

    m_semanticHighlighter->updateFontSettings(m_fontSettings);
    if (!isSemanticInfoOutdated() && m_semanticInfo.isValid()) {
        m_semanticHighlightingNecessary = false;
        m_semanticHighlighter->rerun(m_semanticInfo);
    }
}

void QmlCodeDocument::triggerPendingUpdates()
{
    if (m_fontSettingsNeedsApply) // calls applyFontSettings if necessary
        applyFontSettings();

    // might still need to rehighlight if font settings did not change
    if (m_semanticHighlightingNecessary && !isSemanticInfoOutdated()) {
        m_semanticHighlightingNecessary = false;
        m_semanticHighlighter->rerun(m_semanticInfo);
    }
}

//Marks QmlCodeDocument::marks() const
//{
//    return d->m_marksCache;
//}

//bool QmlCodeDocument::addMark(Mark *mark)
//{
//    if (mark->baseTextDocument())
//        return false;
//    QTC_ASSERT(mark->lineNumber() >= 1, return false);
//    int blockNumber = mark->lineNumber() - 1;
//    auto documentLayout = qobject_cast<TextDocumentLayout*>(d->m_document.documentLayout());
//    QTC_ASSERT(documentLayout, return false);
//    QTextBlock block = d->m_document.findBlockByNumber(blockNumber);

//    if (block.isValid()) {
//        QmlCodeDocument *userData = TextDocumentLayout::userData(block);
//        userData->addMark(mark);
//        d->m_marksCache.append(mark);
//        mark->updateLineNumber(blockNumber + 1);
//        QTC_CHECK(mark->lineNumber() == blockNumber + 1); // Checks that the base class is called
//        mark->updateBlock(block);
//        mark->setBaseTextDocument(this);
//        if (!mark->isVisible())
//            return true;
//        // Update document layout
//        double newMaxWidthFactor = qMax(mark->widthFactor(), documentLayout->maxMarkWidthFactor);
//        bool fullUpdate =  newMaxWidthFactor > documentLayout->maxMarkWidthFactor || !documentLayout->hasMarks;
//        documentLayout->hasMarks = true;
//        documentLayout->maxMarkWidthFactor = newMaxWidthFactor;
//        if (fullUpdate)
//            documentLayout->requestUpdate();
//        else
//            documentLayout->requestExtraAreaUpdate();
//        return true;
//    }
//    return false;
//}

//Marks QmlCodeDocument::marksAt(int line) const
//{
//    QTC_ASSERT(line >= 1, return Marks());
//    int blockNumber = line - 1;
//    QTextBlock block = d->m_document.findBlockByNumber(blockNumber);

//    if (block.isValid()) {
//        if (QmlCodeDocument *userData = TextDocumentLayout::testUserData(block))
//            return userData->marks();
//    }
//    return Marks();
//}

//void QmlCodeDocument::removeMarkFromMarksCache(Mark *mark)
//{
//    auto documentLayout = qobject_cast<TextDocumentLayout*>(d->m_document.documentLayout());
//    QTC_ASSERT(documentLayout, return);
//    d->m_marksCache.removeAll(mark);

//    auto scheduleLayoutUpdate = [documentLayout](){
//        // make sure all destructors that may directly or indirectly call this function are
//        // completed before updating.
//        QTimer::singleShot(0, documentLayout, &QPlainTextDocumentLayout::requestUpdate);
//    };

//    if (d->m_marksCache.isEmpty()) {
//        documentLayout->hasMarks = false;
//        documentLayout->maxMarkWidthFactor = 1.0;
//        scheduleLayoutUpdate();
//        return;
//    }

//    if (!mark->isVisible())
//        return;

//    if (documentLayout->maxMarkWidthFactor == 1.0
//            || mark->widthFactor() == 1.0
//            || mark->widthFactor() < documentLayout->maxMarkWidthFactor) {
//        // No change in width possible
//        documentLayout->requestExtraAreaUpdate();
//    } else {
//        double maxWidthFactor = 1.0;
//        foreach (const Mark *mark, marks()) {
//            if (!mark->isVisible())
//                continue;
//            maxWidthFactor = qMax(mark->widthFactor(), maxWidthFactor);
//            if (maxWidthFactor == documentLayout->maxMarkWidthFactor)
//                break; // Still a mark with the maxMarkWidthFactor
//        }

//        if (maxWidthFactor != documentLayout->maxMarkWidthFactor) {
//            documentLayout->maxMarkWidthFactor = maxWidthFactor;
//            scheduleLayoutUpdate();
//        } else {
//            documentLayout->requestExtraAreaUpdate();
//        }
//    }
//}

//void QmlCodeDocument::removeMark(Mark *mark)
//{
//    QTextBlock block = d->m_document.findBlockByNumber(mark->lineNumber() - 1);
//    if (QmlCodeDocument *data = static_cast<QmlCodeDocument *>(block.userData())) {
//        if (!data->removeMark(mark))
//            qDebug() << "Could not find mark" << mark << "on line" << mark->lineNumber();
//    }

//    removeMarkFromMarksCache(mark);
//    emit markRemoved(mark);
//    mark->setBaseTextDocument(nullptr);
//    updateLayout();
//}

//void QmlCodeDocument::updateMark(Mark *mark)
//{
//    QTextBlock block = d->m_document.findBlockByNumber(mark->lineNumber() - 1);
//    if (block.isValid()) {
//        QmlCodeDocument *userData = TextDocumentLayout::userData(block);
//        // re-evaluate priority
//        userData->removeMark(mark);
//        userData->addMark(mark);
//    }
//    updateLayout();
//}

//void QmlCodeDocument::moveMark(Mark *mark, int previousLine)
//{
//    QTextBlock block = d->m_document.findBlockByNumber(previousLine - 1);
//    if (QmlCodeDocument *data = TextDocumentLayout::testUserData(block)) {
//        if (!data->removeMark(mark))
//            qDebug() << "Could not find mark" << mark << "on line" << previousLine;
//    }
//    removeMarkFromMarksCache(mark);
//    mark->setBaseTextDocument(nullptr);
//    addMark(mark);
//}

static bool isWarning(QmlJS::Severity::Enum kind)
{
    switch (kind) {
    case QmlJS::Severity::Hint:
    case QmlJS::Severity::MaybeWarning:
    case QmlJS::Severity::Warning:
    case QmlJS::Severity::ReadingTypeInfoWarning:
        return true;
    case QmlJS::Severity::MaybeError:
    case QmlJS::Severity::Error:
        break;
    }
    return false;
}

void QmlCodeDocument::createMarks(const QList<DiagnosticMessage> &diagnostics)
{
    for (const DiagnosticMessage &diagnostic : diagnostics) {
        QPointer<QmlCodeDocument> ptr(this);
        const auto onMarkRemoved = [ptr] (Mark* mark) {
            if (ptr)
                ptr->m_diagnosticMarks.removeAll(mark);
        };
        auto block = findBlockByLineNumber(diagnostic.loc.startLine - 1);
        auto blockData = QmlCodeDocument::userData(block);
        blockData->mark.type = isWarning(diagnostic.kind) ? Mark::CodeModelWarning
                                                          : Mark::CodeModelError;
        blockData->mark.message = diagnostic.message;
        blockData->mark.removalCallback = onMarkRemoved;
        m_diagnosticMarks.append(&blockData->mark);
    }
}

void QmlCodeDocument::createMarks(const SemanticInfo &info)
{
    cleanSemanticMarks();
    QPointer<QmlCodeDocument> ptr(this);
    const auto onMarkRemoved = [ptr] (Mark* mark) {
        if (ptr)
            ptr->m_semanticMarks.removeAll(mark);
    };
    for (const DiagnosticMessage &diagnostic : qAsConst(info.semanticMessages)) {
        auto block = findBlockByLineNumber(diagnostic.loc.startLine - 1);
        auto blockData = QmlCodeDocument::userData(block);
        blockData->mark.type = isWarning(diagnostic.kind) ? Mark::CodeModelWarning
                                                          : Mark::CodeModelError;
        blockData->mark.message = diagnostic.message;
        blockData->mark.removalCallback = onMarkRemoved;
        m_semanticMarks.append(&blockData->mark);
    }
    for (const QmlJS::StaticAnalysis::Message &message : qAsConst(info.staticAnalysisMessages)) {
        auto block = findBlockByLineNumber(message.location.startLine - 1);
        auto blockData = QmlCodeDocument::userData(block);
        blockData->mark.type = isWarning(message.severity) ? Mark::CodeModelWarning
                                                           : Mark::CodeModelError;
        blockData->mark.message = message.message;
        blockData->mark.removalCallback = onMarkRemoved;
        m_semanticMarks.append(&blockData->mark);
    }
}

static void cleanMarks(QVector<Mark*>* marks)
{
    for (Mark* mark : *marks)
        mark->type = Mark::NoMark;
    marks->clear();
}

void QmlCodeDocument::cleanSemanticMarks()
{
    cleanMarks(&m_semanticMarks);
}

void QmlCodeDocument::cleanDiagnosticMarks()
{
    cleanMarks(&m_diagnosticMarks);
}
