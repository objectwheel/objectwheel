#include <parserbackend.h>
#include <filemanager.h>

#include <qmljs/qmljsdocument.h>
#include <qmljs/qmljsbind.h>
#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/qmljspropertyreader.h>
#include <QtWidgets>

using namespace QmlJS;
using namespace AST;

namespace {
    bool propertyExists(const UiObjectMemberList* list, const QString& property);
    void addProperty(QString& source, const UiObjectInitializer* initializer, const QString& property, const QString& value);
    void changeProperty(QString& source, const UiObjectMemberList* list, const QString& property, const QString& value);
}

ParserBackend* ParserBackend::instance()
{
    static ParserBackend instance;
    return &instance;
}

void ParserBackend::init(const QString& fileName)
{
    _source = rdfile(fileName);
}

const QString& ParserBackend::source() const
{
    return _source;
}

void ParserBackend::setProperty(const QString& property, const QString& value)
{
    Dialect dialect(Dialect::Qml);
    QSharedPointer<Document> document = Document::create(_fileName, dialect);
    document->setSource(_source);

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
        /*return*/ changeProperty(_source, uiObjectInitializer->members, property, value);
    else
        /*return*/ addProperty(_source, uiObjectInitializer, property, value);

     wrfile("/users/omergoktas/desktop/deneme.qml", _source.toUtf8());
}

namespace {
    bool propertyExists(const UiObjectMemberList* list, const QString& property)
    {
        while(list) {
            UiPublicMember* publicMember;
            UiArrayBinding* arrayBinding;
            UiObjectBinding* objectBinding;
            UiScriptBinding* scriptBinding;

            if ((scriptBinding = cast<UiScriptBinding*>(list->member))) {
                if (scriptBinding->qualifiedId->name == property)
                    return true;
            } else if ((arrayBinding = cast<UiArrayBinding*>(list->member))) {
                if (arrayBinding->qualifiedId->name == property)
                    return true;
            } else if ((objectBinding = cast<UiObjectBinding*>(list->member))) {
                if (objectBinding->qualifiedId->name == property)
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
        quint32 begin, end;

        while(list) {
            UiPublicMember* publicMember;
            UiArrayBinding* arrayBinding;
            UiObjectBinding* objectBinding;
            UiScriptBinding* scriptBinding;

            if ((scriptBinding = cast<UiScriptBinding*>(list->member))) {
                if (scriptBinding->qualifiedId->name == property) {
                    begin = scriptBinding->firstSourceLocation().begin();
                    end = scriptBinding->lastSourceLocation().end();
                    break;
                }
            } else if ((arrayBinding = cast<UiArrayBinding*>(list->member))) {
                if (arrayBinding->qualifiedId->name == property) {
                    begin = arrayBinding->firstSourceLocation().begin();
                    end = arrayBinding->lastSourceLocation().end();
                    break;
                }
            } else if ((objectBinding = cast<UiObjectBinding*>(list->member))) {
                if (objectBinding->qualifiedId->name == property) {
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

        QTextDocument document(source);
        QTextCursor cursor(&document);

        cursor.setPosition(begin);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        cursor.insertText(property + ": " + value);

        source = document.toPlainText();
    }
}