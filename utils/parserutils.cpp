#include <parserutils.h>
#include <saveutils.h>

#include <qmljs/qmljsdocument.h>
#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/qmljsmodelmanagerinterface.h>
#include <qmljs/qmljsscopechain.h>
#include <qmljs/qmljslink.h>

#include <QTextDocument>
#include <QTextCursor>
#include <QRegularExpression>
#include <QDir>

using namespace QmlJS;
using namespace AST;

namespace ParserUtils {

namespace Internal {

QString getModuleName(const Document::Ptr &qmlDocument, const QString& typeName, const QStringList& modules)
{
    static QSharedPointer<Document> doc;
    if (doc.isNull()) {
        const QString ff = ":/shared/moduleresolver/moduleresolver.qml";
        QFile file(ff);
        if (!file.open(QFile::ReadOnly)) {
            qWarning("ParserUtils: Cannot open file");
            return {};
        }
        doc = Document::create(ff, Dialect::Qml);
        doc->setSource(file.readAll());
        file.close();
        if (!doc->parse()) {
            qWarning() << "Property couldn't read. Unable to parse qml file.";
            return {};
        }
    }

    static ModelManagerInterface* manager = ModelManagerInterface::instance();
    static Link link(manager->snapshot(), manager->defaultVContext(doc->language(), doc), manager->builtins(doc));
    static ContextPtr context(link(doc, nullptr));

    QStringList moduleNames;
    const Imports *imports = context->imports(doc.data());
    const QList<ImportInfo>& importInfos = imports->infos(typeName, context.data());
    for (const ImportInfo& importInfo : importInfos) {
        if (importInfo.isValid() && importInfo.type() == ImportType::Library) {
            moduleNames.append(importInfo.name() + QString::number(importInfo.version().majorVersion()));
        } else if (importInfo.isValid() && importInfo.type() == ImportType::Directory) {
            const QString path = importInfo.path();
            const QDir dir(qmlDocument->path());
            // should probably try to make it relatve to some import path, not to the document path
            QString relativeDir = dir.relativeFilePath(path);
            moduleNames.append(relativeDir.replace(QLatin1Char('/'), QLatin1Char('.')));
        } else if (importInfo.isValid() && importInfo.type() == ImportType::QrcDirectory) {
            QString path = QrcParser::normalizedQrcDirectoryPath(importInfo.path());
            path = path.mid(1, path.size() - ((path.size() > 1) ? 2 : 1));
            moduleNames.append(path.replace(QLatin1Char('/'), QLatin1Char('.')));
        }
    }

    int max = -1, index = -1;
    for (int i = 0; i < moduleNames.size(); ++i) {
        int x = modules.lastIndexOf(moduleNames.at(i));
        if (x > max) {
            max = x;
            index = i;
        }
    }

    if (index > -1)
        return moduleNames.at(index);

    return QString();
}

QString cleanPropertyValue(const QString& value)
{
    QString val(value);
    while (val.left(1).contains(QRegularExpression("[\\r\\n\\t\\f\\v ]")))
        val.remove(0, 1);
    while (val.right(1).contains(QRegularExpression("[\\r\\n\\t\\f\\v ]")))
        val.remove(val.size() - 1, 1);
    return val;
}

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

QString fullPropertyValue(const QString& source, const QString& property, const UiObjectMemberList* list)
{
    quint32 begin = 0, end = 0;

    while(list) {
        UiPublicMember* publicMember;
        UiArrayBinding* arrayBinding;
        UiObjectBinding* objectBinding;
        UiScriptBinding* scriptBinding;

        if ((scriptBinding = cast<UiScriptBinding*>(list->member))) {
            if (fullPropertyName(scriptBinding->qualifiedId) == property) {
                begin = scriptBinding->colonToken.end();
                end = scriptBinding->lastSourceLocation().end();
                break;
            }
        } else if ((arrayBinding = cast<UiArrayBinding*>(list->member))) {
            if (fullPropertyName(arrayBinding->qualifiedId) == property) {
                begin = arrayBinding->colonToken.end();
                end = arrayBinding->lastSourceLocation().end();
                break;
            }
        } else if ((objectBinding = cast<UiObjectBinding*>(list->member))) {
            if (fullPropertyName(objectBinding->qualifiedId) == property) {
                begin = objectBinding->colonToken.end();
                end = objectBinding->lastSourceLocation().end();
                break;
            }
        } else if ((publicMember = cast<UiPublicMember*>(list->member))) {
            if (publicMember->name == property) {
                begin = publicMember->colonToken.end();
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
        return cursor.selectedText();
    }

    return QString();
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

} // Internal

bool exists(const QString& controlDir, const QString& property)
{
    const QString& mainQmlFilePath = SaveUtils::toControlMainQmlFile(controlDir);
    QFile file(mainQmlFilePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("ParserUtils: Cannot open file");
        return false;
    }

    Dialect dialect(Dialect::Qml);
    QSharedPointer<Document> document = Document::create(mainQmlFilePath, dialect);
    document->setSource(file.readAll());
    file.close();

    if (!document->parse()) {
        qWarning() << "Property couldn't set. Unable to parse qml file.";
        return false;
    }

    auto uiProgram = document->qmlProgram();

    if (!uiProgram) {
        qWarning() << "Property couldn't set. Corrupted ui program.";
        return false;
    }

    auto uiObjectMemberList = uiProgram->members;

    if (!uiObjectMemberList) {
        qWarning() << "Property couldn't set. Empty source file.";
        return false;
    }

    auto uiObjectDefinition = cast<UiObjectDefinition *>(uiObjectMemberList->member);

    if (!uiObjectDefinition) {
        qWarning() << "Property couldn't set. Bad file format 0x1.";
        return false;
    }

    auto uiObjectInitializer = uiObjectDefinition->initializer;

    if (!uiObjectInitializer) {
        qWarning() << "Property couldn't set. Bad file format 0x2.";
        return false;
    }

    return Internal::propertyExists(uiObjectInitializer->members, property);
}

bool canParse(const QString& controlDir)
{
    const QString& mainQmlFilePath = SaveUtils::toControlMainQmlFile(controlDir);
    QFile file(mainQmlFilePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("ParserUtils: Cannot open file");
        return false;
    }
    Dialect dialect(Dialect::Qml);
    QSharedPointer<Document> document = Document::create(mainQmlFilePath, dialect);
    document->setSource(file.readAll());
    file.close();
    return document->parse();
}

QString id(const QString& controlDir)
{
    if (canParse(controlDir))
        return property(controlDir, "id");
    return SaveUtils::controlId(controlDir);
}

QString moduleName(const QString& controlDir)
{
    const QString& mainQmlFilePath = SaveUtils::toControlMainQmlFile(controlDir);
    QFile file(mainQmlFilePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("ParserUtils: Cannot open file");
        return {};
    }
    const QString& source = file.readAll();
    file.close();
    Dialect dialect(Dialect::Qml);
    QSharedPointer<Document> doc = Document::create(mainQmlFilePath, dialect);
    doc->setSource(source);

    if (!doc->parse()) {
        qWarning() << "Property couldn't read. Unable to parse qml file.";
        return QString();
    }

    auto uiProgram = doc->qmlProgram();

    if (!uiProgram) {
        qWarning() << "Property couldn't read. Corrupted ui program.";
        return QString();
    }

    auto uiObjectMemberList = uiProgram->members;

    if (!uiObjectMemberList) {
        qWarning() << "Property couldn't read. Empty source file.";
        return QString();
    }

    auto uiHeaderItemList = uiProgram->headers;

    if (!uiHeaderItemList) {
        qWarning() << "Property couldn't read. Empty source file.";
        return QString();
    }

    auto uiObjectDefinition = cast<UiObjectDefinition *>(uiObjectMemberList->member);

    if (!uiObjectDefinition) {
        qWarning() << "Property couldn't read. Bad file format 0x1.";
        return QString();
    }

    auto qualifiedId = cast<UiQualifiedId *>(uiObjectDefinition->qualifiedTypeNameId);

    if (!qualifiedId) {
        qWarning() << "Property couldn't read. Bad file format 0x1.";
        return QString();
    }

    UiHeaderItemList* header = uiHeaderItemList;

    QStringList modules;
    do {
        auto import = cast<UiImport*>(header->headerItem);
        if (!import) {
            header = header->next;
            continue;
        }
        QStringRef version(&source, import->versionToken.offset, import->versionToken.length);
        if (UiQualifiedId* uri = import->importUri) {
            QStringList pieces;
            do {
                pieces.append(uri->name.toString());
                uri = uri->next;
            } while(uri);
            if (version.isEmpty())
                modules.append(pieces.join('.'));
            else
                modules.append(pieces.join('.') + QString::number(int(version.toFloat())));
        }
        header = header->next;
    } while(header);

    QString typeName(qualifiedId->name.toString());
    QString moduleName(typeName);
    moduleName.prepend('.');
    moduleName.prepend(Internal::getModuleName(doc, typeName, modules));
    return moduleName;
}

QString property(const QString& controlDir, const QString& property)
{
    const QString& mainQmlFilePath = SaveUtils::toControlMainQmlFile(controlDir);
    QFile file(mainQmlFilePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("ParserUtils: Cannot open file");
        return {};
    }
    const QString& source = file.readAll();
    file.close();
    Dialect dialect(Dialect::Qml);
    QSharedPointer<Document> document = Document::create(mainQmlFilePath, dialect);
    document->setSource(source);

    if (!document->parse()) {
        qWarning() << "Property couldn't read. Unable to parse qml file.";
        return QString();
    }

    auto uiProgram = document->qmlProgram();

    if (!uiProgram) {
        qWarning() << "Property couldn't read. Corrupted ui program.";
        return QString();
    }

    auto uiObjectMemberList = uiProgram->members;

    if (!uiObjectMemberList) {
        qWarning() << "Property couldn't read. Empty source file.";
        return QString();
    }

    auto uiObjectDefinition = cast<UiObjectDefinition *>(uiObjectMemberList->member);

    if (!uiObjectDefinition) {
        qWarning() << "Property couldn't read. Bad file format 0x1.";
        return QString();
    }

    auto uiObjectInitializer = uiObjectDefinition->initializer;

    if (!uiObjectInitializer) {
        qWarning() << "Property couldn't read. Bad file format 0x2.";
        return QString();
    }

    if (!Internal::propertyExists(uiObjectInitializer->members, property))
        return QString();

    return Internal::cleanPropertyValue(Internal::fullPropertyValue(source, property, uiObjectInitializer->members));
}

QString property(QTextDocument* doc, const QString& controlDir, const QString& property)
{
    const QString& source = doc->toPlainText();
    Dialect dialect(Dialect::Qml);
    QSharedPointer<Document> document = Document::create(SaveUtils::toControlMainQmlFile(controlDir), dialect);
    document->setSource(source);

    if (!document->parse()) {
        qWarning() << "Property couldn't read. Unable to parse qml file.";
        return QString();
    }

    auto uiProgram = document->qmlProgram();

    if (!uiProgram) {
        qWarning() << "Property couldn't read. Corrupted ui program.";
        return QString();
    }

    auto uiObjectMemberList = uiProgram->members;

    if (!uiObjectMemberList) {
        qWarning() << "Property couldn't read. Empty source file.";
        return QString();
    }

    auto uiObjectDefinition = cast<UiObjectDefinition *>(uiObjectMemberList->member);

    if (!uiObjectDefinition) {
        qWarning() << "Property couldn't read. Bad file format 0x1.";
        return QString();
    }

    auto uiObjectInitializer = uiObjectDefinition->initializer;

    if (!uiObjectInitializer) {
        qWarning() << "Property couldn't read. Bad file format 0x2.";
        return QString();
    }

    if (!Internal::propertyExists(uiObjectInitializer->members, property))
        return QString();

    return Internal::cleanPropertyValue(Internal::fullPropertyValue(source, property, uiObjectInitializer->members));
}

void setId(const QString& controlDir, const QString& id)
{
    if (canParse(controlDir))
        setProperty(controlDir, "id", id);
    SaveUtils::setProperty(controlDir, SaveUtils::ControlId, id);
}

void setProperty(const QString& controlDir, const QString& property, const QString& value)
{
    const QString& mainQmlFilePath = SaveUtils::toControlMainQmlFile(controlDir);
    QFile file(mainQmlFilePath);
    if (!file.open(QFile::ReadWrite)) {
        qWarning("ParserUtils: Cannot open file");
        return;
    }
    QString source = file.readAll();

    Dialect dialect(Dialect::Qml);
    QSharedPointer<Document> document = Document::create(mainQmlFilePath, dialect);
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

    if (Internal::propertyExists(uiObjectInitializer->members, property))
        Internal::changeProperty(source, uiObjectInitializer->members, property, value);
    else
        Internal::addProperty(source, uiObjectInitializer, property, value);

    file.resize(0);
    file.write(source.toUtf8());
    file.close();
}

void setProperty(QTextDocument* doc, const QString& controlDir, const QString& property, const QString& value)
{
    Dialect dialect(Dialect::Qml);
    QSharedPointer<Document> document = Document::create(SaveUtils::toControlMainQmlFile(controlDir), dialect);
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

    if (Internal::propertyExists(uiObjectInitializer->members, property))
        Internal::changeProperty(doc, uiObjectInitializer->members, property, value);
    else
        Internal::addProperty(doc, uiObjectInitializer, property, value);
}

int addMethod(QTextDocument* document, const QString& url, const QString& method)
{
    quint32 begin = 0;
    const QString& source = document->toPlainText();
    Dialect dialect(Dialect::JavaScript);
    QSharedPointer<Document> doc = Document::create(url, dialect);
    doc->setSource(source);

    if (!doc->parse()) {
        qWarning() << "JS File couldn't read. Unable to parse js file.";
        return -1;
    }

    auto jsProgram = doc->jsProgram();

    if (!jsProgram) {
        qWarning() << "JS File couldn't read. Corrupted js program.";
        return -1;
    }

    auto jsElements = jsProgram->elements;

    if (jsElements) {
        begin = jsElements->lastSourceLocation().end();
    } else {
        qWarning() << "JS File error. Empty source file.";
        return -1;
    }

    if (begin > 0) {
        QTextCursor cursor(document);
        cursor.beginEditBlock();
        cursor.setPosition(begin);
        cursor.insertText(method);
        cursor.endEditBlock();
    }

    return jsElements->lastSourceLocation().startLine;
}

int methodLine(QTextDocument* document, const QString& url, const QString& methodSign)
{
    QString source = document->toPlainText();
    Dialect dialect(Dialect::JavaScript);
    QSharedPointer<Document> doc = Document::create(url, dialect);
    doc->setSource(source);

    if (!doc->parse()) {
        qWarning() << "JS File couldn't read. Unable to parse js file.";
        return -1;
    }

    auto jsProgram = doc->jsProgram();

    if (!jsProgram) {
        qWarning() << "JS File couldn't read. Corrupted js program.";
        return -1;
    }

    auto jsElements = jsProgram->elements;

    if (jsElements) {
        while (jsElements) {
            Q_ASSERT(jsElements->element);
            if (jsElements->element->kind == Node::Kind_FunctionSourceElement) {
                auto element = static_cast<FunctionSourceElement*>(jsElements->element);
                Q_ASSERT(element->declaration);
                if (element->declaration->name == methodSign) {
                    int lbrace = element->declaration->lbraceToken.startLine;
                    int rbrace = element->declaration->rbraceToken.startLine;
                    if (lbrace == rbrace || lbrace + 1 == rbrace)
                        return lbrace;
                    else
                        return lbrace + 1;
                }

            }

            jsElements = jsElements->next;
        }
    } else {
        qWarning() << "JS File error. Empty source file.";
        return -1;
    }

    return -1;
}

int methodPosition(QTextDocument* document, const QString& url, const QString& methodSign, bool lbrace)
{
    QString source = document->toPlainText();
    Dialect dialect(Dialect::JavaScript);
    QSharedPointer<Document> doc = Document::create(url, dialect);
    doc->setSource(source);

    if (!doc->parse()) {
        qWarning() << "JS File couldn't read. Unable to parse js file.";
        return -1;
    }

    auto jsProgram = doc->jsProgram();

    if (!jsProgram) {
        qWarning() << "JS File couldn't read. Corrupted js program.";
        return -1;
    }

    auto jsElements = jsProgram->elements;

    if (jsElements) {
        while (jsElements) {
            Q_ASSERT(jsElements->element);
            if (jsElements->element->kind == Node::Kind_FunctionSourceElement) {
                auto element = static_cast<FunctionSourceElement*>(jsElements->element);
                Q_ASSERT(element->declaration);
                if (element->declaration->name == methodSign) {
                    if (lbrace)
                        return element->declaration->lbraceToken.end();
                    else
                        return element->declaration->rbraceToken.begin();
                }

            }

            jsElements = jsElements->next;
        }
    } else {
        qWarning() << "JS File error. Empty source file.";
        return -1;
    }

    return -1;
}

void addConnection(QTextDocument* document, const QString& url, const QString& loaderSign, const QString& connection)
{
    int i = methodPosition(document, url, loaderSign, true);
    if (i > 0) {
        QTextCursor cursor(document);
        cursor.beginEditBlock();
        cursor.setPosition(i);
        cursor.insertText("\n    " + connection);
        cursor.endEditBlock();
    }
}

} // ParserUtils