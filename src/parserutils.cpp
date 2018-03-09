#include <parserutils.h>
#include <filemanager.h>

#include <qmljs/qmljsdocument.h>
#include <qmljs/parser/qmljsast_p.h>

#include <QTextDocument>
#include <QTextCursor>
#include <QRegularExpression>

using namespace QmlJS;
using namespace AST;

namespace {
    QString fullPropertyName(const UiQualifiedId* qualifiedId);
    bool propertyExists(const UiObjectMemberList* list, const QString& property);
    void addProperty(QString& source, const UiObjectInitializer* initializer, const QString& property, const QString& value);
    void changeProperty(QString& source, const UiObjectMemberList* list, const QString& property, const QString& value);
    void addProperty(QTextDocument* doc, const UiObjectInitializer* initializer, const QString& property, const QString& value);
    void changeProperty(QTextDocument* doc, const UiObjectMemberList* list, const QString& property, const QString& value);
}

void ParserUtils::setProperty(const QString& fileName, const QString& property, const QString& value)
{
    QString source = rdfile(fileName);

    Dialect dialect(Dialect::Qml);
    QSharedPointer<Document> document = Document::create(fileName, dialect);
    document->setSource(source);

    if (!document->parse()) {
        qWarning() << "Property couldn't set. Unable to parse qml file.";
        return;
    }

    auto uiProgram = document->qmlProgram();

    if (!uiProgram) {
        qWarning() << "Property couldn't set. Corrupted ui program.";
        return;
    }

    auto uiObjectMemberList = uiProgram->members;

    if (!uiObjectMemberList) {
        qWarning() << "Property couldn't set. Empty source file.";
        return;
    }

    auto uiObjectDefinition = cast<UiObjectDefinition *>(uiObjectMemberList->member);

    if (!uiObjectDefinition) {
        qWarning() << "Property couldn't set. Bad file format 0x1.";
        return;
    }

    auto uiObjectInitializer = uiObjectDefinition->initializer;

    if (!uiObjectInitializer) {
        qWarning() << "Property couldn't set. Bad file format 0x2.";
        return;
    }

    if (propertyExists(uiObjectInitializer->members, property))
        changeProperty(source, uiObjectInitializer->members, property, value);
    else
        addProperty(source, uiObjectInitializer, property, value);

    wrfile(fileName, source.toUtf8());
}

void ParserUtils::setProperty(QTextDocument* doc, const QString& fileName, const QString& property, const QString& value)
{
    Dialect dialect(Dialect::Qml);
    QSharedPointer<Document> document = Document::create(fileName, dialect);
    document->setSource(doc->toPlainText());

    if (!document->parse()) {
        qWarning() << "Property couldn't set. Unable to parse qml file.";
        return;
    }

    auto uiProgram = document->qmlProgram();

    if (!uiProgram) {
        qWarning() << "Property couldn't set. Corrupted ui program.";
        return;
    }

    auto uiObjectMemberList = uiProgram->members;

    if (!uiObjectMemberList) {
        qWarning() << "Property couldn't set. Empty source file.";
        return;
    }

    auto uiObjectDefinition = cast<UiObjectDefinition *>(uiObjectMemberList->member);

    if (!uiObjectDefinition) {
        qWarning() << "Property couldn't set. Bad file format 0x1.";
        return;
    }

    auto uiObjectInitializer = uiObjectDefinition->initializer;

    if (!uiObjectInitializer) {
        qWarning() << "Property couldn't set. Bad file format 0x2.";
        return;
    }

    if (propertyExists(uiObjectInitializer->members, property))
        changeProperty(doc, uiObjectInitializer->members, property, value);
    else
        addProperty(doc, uiObjectInitializer, property, value);
}

namespace {
    QString fullPropertyName(const UiQualifiedId* qualifiedId)
    {
        QString name;

        while(qualifiedId) {
            if (name.isEmpty())
                name = qualifiedId->name.toString();
            else
                name += "." + qualifiedId->name.toString();

            qualifiedId = qualifiedId->next;
        }

        return name;
    }

    bool propertyExists(const UiObjectMemberList* list, const QString& property)
    {
        while(list) {
            UiPublicMember* publicMember;
            UiArrayBinding* arrayBinding;
            UiObjectBinding* objectBinding;
            UiScriptBinding* scriptBinding;

            if ((scriptBinding = cast<UiScriptBinding*>(list->member))) {
                if (fullPropertyName(scriptBinding->qualifiedId) == property)
                    return true;
            } else if ((arrayBinding = cast<UiArrayBinding*>(list->member))) {
                if (fullPropertyName(arrayBinding->qualifiedId) == property)
                    return true;
            } else if ((objectBinding = cast<UiObjectBinding*>(list->member))) {
                if (fullPropertyName(objectBinding->qualifiedId) == property)
                    return true;
            } else if ((publicMember = cast<UiPublicMember*>(list->member))) {
                if (publicMember->name == property)
                    return true;
            }

            list = list->next;
        }

        return false;
    }

    void addProperty(QString& source, const UiObjectInitializer* initializer, const QString& property, const QString& value)
    {
        QTextDocument document(source);
        QTextCursor cursor(&document);

        cursor.setPosition(initializer->lbraceToken.end());
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);

        auto rightBlock = cursor.selectedText();
        bool addNewLine = rightBlock.contains(QRegularExpression("[^\\r\\n\\t\\f\\v ]"));
        int rightSpaces = 0;
        int depth = 4;

        QRegularExpression regexpSpaces("^ +");
        auto res = regexpSpaces.match(rightBlock);
        if (res.hasMatch())
            rightSpaces = res.captured().size();

        cursor.clearSelection();
        cursor.setPosition(initializer->lbraceToken.end());
        cursor.insertText("\n");

        for (int i = depth; i--;)
            cursor.insertText(" ");

        cursor.insertText(property + ": " + value);

        if (addNewLine) {
            cursor.insertText("\n");

            if (!rightBlock.contains("}"))
                for (int i = depth - rightSpaces; i--;)
                    cursor.insertText(" ");
        }

        source = document.toPlainText();
    }

    void changeProperty(QString& source, const UiObjectMemberList* list, const QString& property, const QString& value)
    {
        quint32 begin = 0, end = 0;

        while(list) {
            UiPublicMember* publicMember;
            UiArrayBinding* arrayBinding;
            UiObjectBinding* objectBinding;
            UiScriptBinding* scriptBinding;

            if ((scriptBinding = cast<UiScriptBinding*>(list->member))) {
                if (fullPropertyName(scriptBinding->qualifiedId) == property) {
                    begin = scriptBinding->firstSourceLocation().begin();
                    end = scriptBinding->lastSourceLocation().end();
                    break;
                }
            } else if ((arrayBinding = cast<UiArrayBinding*>(list->member))) {
                if (fullPropertyName(arrayBinding->qualifiedId) == property) {
                    begin = arrayBinding->firstSourceLocation().begin();
                    end = arrayBinding->lastSourceLocation().end();
                    break;
                }
            } else if ((objectBinding = cast<UiObjectBinding*>(list->member))) {
                if (fullPropertyName(objectBinding->qualifiedId) == property) {
                    begin = objectBinding->firstSourceLocation().begin();
                    end = objectBinding->lastSourceLocation().end();
                    break;
                }
            } else if ((publicMember = cast<UiPublicMember*>(list->member))) {
                if (publicMember->name == property) {
                    begin = publicMember->firstSourceLocation().begin();
                    end = publicMember->lastSourceLocation().end();
                    break;
                }
            }

            list = list->next;
        }

        if (begin > 0 && end > 0) {
            QTextDocument document(source);
            QTextCursor cursor(&document);

            cursor.setPosition(begin);
            cursor.setPosition(end, QTextCursor::KeepAnchor);
            cursor.insertText(property + ": " + value);

            source = document.toPlainText();
        }
    }

    void addProperty(QTextDocument* document, const UiObjectInitializer* initializer, const QString& property, const QString& value)
    {
        bool modified = document->isModified();
        QTextCursor cursor(document);
        cursor.beginEditBlock();
        cursor.setPosition(initializer->lbraceToken.end());
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);

        auto rightBlock = cursor.selectedText();
        bool addNewLine = rightBlock.contains(QRegularExpression("[^\\r\\n\\t\\f\\v ]"));
        int rightSpaces = 0;
        int depth = 4;

        QRegularExpression regexpSpaces("^ +");
        auto res = regexpSpaces.match(rightBlock);
        if (res.hasMatch())
            rightSpaces = res.captured().size();

        cursor.clearSelection();
        cursor.setPosition(initializer->lbraceToken.end());
        cursor.insertText("\n");

        for (int i = depth; i--;)
            cursor.insertText(" ");

        cursor.insertText(property + ": " + value);

        if (addNewLine) {
            cursor.insertText("\n");

            if (!rightBlock.contains("}"))
                for (int i = depth - rightSpaces; i--;)
                    cursor.insertText(" ");
        }

        cursor.endEditBlock();

        if (!modified)
            document->setModified(false);
    }

    void changeProperty(QTextDocument* document, const UiObjectMemberList* list, const QString& property, const QString& value)
    {
        bool modified = document->isModified();
        quint32 begin = 0, end = 0;

        while(list) {
            UiPublicMember* publicMember;
            UiArrayBinding* arrayBinding;
            UiObjectBinding* objectBinding;
            UiScriptBinding* scriptBinding;

            if ((scriptBinding = cast<UiScriptBinding*>(list->member))) {
                if (fullPropertyName(scriptBinding->qualifiedId) == property) {
                    begin = scriptBinding->firstSourceLocation().begin();
                    end = scriptBinding->lastSourceLocation().end();
                    break;
                }
            } else if ((arrayBinding = cast<UiArrayBinding*>(list->member))) {
                if (fullPropertyName(arrayBinding->qualifiedId) == property) {
                    begin = arrayBinding->firstSourceLocation().begin();
                    end = arrayBinding->lastSourceLocation().end();
                    break;
                }
            } else if ((objectBinding = cast<UiObjectBinding*>(list->member))) {
                if (fullPropertyName(objectBinding->qualifiedId) == property) {
                    begin = objectBinding->firstSourceLocation().begin();
                    end = objectBinding->lastSourceLocation().end();
                    break;
                }
            } else if ((publicMember = cast<UiPublicMember*>(list->member))) {
                if (publicMember->name == property) {
                    begin = publicMember->firstSourceLocation().begin();
                    end = publicMember->lastSourceLocation().end();
                    break;
                }
            }

            list = list->next;
        }

        if (begin > 0 && end > 0) {
            QTextCursor cursor(document);
            cursor.beginEditBlock();
            cursor.setPosition(begin);
            cursor.setPosition(end, QTextCursor::KeepAnchor);
            cursor.insertText(property + ": " + value);
            cursor.endEditBlock();

            if (!modified)
                document->setModified(false);
        }
    }
}