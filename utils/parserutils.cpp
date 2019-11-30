#include <parserutils.h>
#include <saveutils.h>

#include <qmljs/qmljsdocument.h>
#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/qmljsmodelmanagerinterface.h>
#include <qmljs/qmljsscopechain.h>
#include <qmljs/qmljslink.h>
#include <qmljs/qmljsutils.h>
#include <qmljs/qmljsbind.h>

#include <QTextDocument>
#include <QTextCursor>
#include <QRegularExpression>
#include <QDir>

using namespace QmlJS;
using namespace AST;

namespace ParserUtils {

namespace Internal {

QString moduleForImportInfo(const QString& typeName, const ImportInfo& importInfo, const Document* document)
{
    if (importInfo.isValid() && importInfo.type() == ImportType::Library) {
        return importInfo.name() + QStringLiteral("/") +
                importInfo.version().toString() + QStringLiteral("/") + typeName;
    } else if (importInfo.isValid() && importInfo.type() == ImportType::Directory) {
        const QString path = importInfo.path();
        const QDir dir(document->path());
        // should probably try to make it relatve to some import path, not to the document path
        QString relativeDir = dir.relativeFilePath(path);
        const QString name = relativeDir.replace(QLatin1Char('/'), QLatin1Char('.'));
        return name + QStringLiteral("/") + typeName;
    } else if (importInfo.isValid() && importInfo.type() == ImportType::QrcDirectory) {
        QString path = QrcParser::normalizedQrcDirectoryPath(importInfo.path());
        path = path.mid(1, path.size() - ((path.size() > 1) ? 2 : 1));
        const QString name = path.replace(QLatin1Char('/'), QLatin1Char('.'));
        return name + QStringLiteral("/") + typeName;
    }
    return QString();
}

QStringList importsToModules(const QString& typeName, const Document* document, const QList<ImportInfo>& importInfos)
{
    QStringList modules;
    foreach (const ImportInfo& importInfo, importInfos) {
        const QString& module = moduleForImportInfo(typeName, importInfo, document);
        if (!module.isEmpty())
            modules.append(module);
    }
    return modules;
}

QString moduleMatch(const QStringList& originalModules, const QStringList& genericModules)
{
    // Match against exact version numbers first (major + minor)
    {
        int index = -1;
        for (const QString& genericModule : genericModules) {
            int pos = originalModules.lastIndexOf(genericModule);
            if (pos > index)
                index = pos;
        }
        if (index > -1)
            return originalModules.at(index);
    }

    // If can't find, try only major version
    {
        QStringList originalModulesMajor;
        for (const QString& module : originalModules)
            originalModulesMajor.append(ParserUtils::moduleToMajorModule(module));
        int index = -1;
        for (const QString& genericModule : genericModules) {
            int pos = originalModulesMajor.lastIndexOf(ParserUtils::moduleToMajorModule(genericModule));
            if (pos > index)
                index = pos;
        }
        if (index > -1)
            return originalModulesMajor.at(index);
    }

    return QString();
}

QString resolveModule(const Document::Ptr document, const QString& typeName, const QStringList& originalModules)
{
    static Document::MutablePtr genericDocument;
    static ContextPtr genericContext;
    static ModelManagerInterface* manager = ModelManagerInterface::instance();

    // Initialize generic context
    if (genericDocument.isNull()) {
        const QString ff = ":/moduleresolver/moduleresolver.qml";
        QFile file(ff);
        if (!file.open(QFile::ReadOnly)) {
            qWarning("ParserUtils: Cannot open file");
            return {};
        }
        genericDocument = Document::create(ff, Dialect::Qml);
        genericDocument->setSource(file.readAll());
        file.close();
        if (!genericDocument->parse()) {
            qWarning() << "Property couldn't read. Unable to parse qml file.";
            return {};
        }
        Link link(manager->snapshot(),
                  manager->defaultVContext(genericDocument->language(),
                                           genericDocument),
                  manager->builtins(genericDocument));
        genericContext = link(genericDocument, nullptr);
    }

    const Imports* genericImports = genericContext->imports(genericDocument.data());
    const QList<ImportInfo>& allGenericImportInfos = genericImports->allInfos();
    const QList<ImportInfo>& appropriateGenericImportInfos = genericImports->infos(typeName, genericContext.data());
    const QStringList& allGenericModules = importsToModules(typeName, genericDocument.data(), allGenericImportInfos);
    const QStringList& appropriateGenericModules = importsToModules(typeName, genericDocument.data(), appropriateGenericImportInfos);

    Q_UNUSED(document)
    Q_UNUSED(allGenericModules)

    // Make sure original qml files doesn't contain
    // An import that is not available within generic
    // document; otherwise skip generic matching trial
    bool skipGenericMatch = false;
    // {
    //     QStringList originalModulesBodyMajor;
    //     for (const QString& module : originalModules)
    //         originalModulesBodyMajor.append(ParserUtils::moduleBodyPlusMajorVersion(module));
    //     QStringList genericModulesBodyMajor;
    //     for (const QString& module : allGenericModules)
    //         genericModulesBodyMajor.append(ParserUtils::moduleBodyPlusMajorVersion(module));
    //     for (const QString& originalModuleMajor : qAsConst(originalModulesBodyMajor)) {
    //         bool found = false;
    //         for (const QString& genericModuleMajor : genericModulesBodyMajor) {
    //             if (originalModuleMajor == genericModuleMajor) {
    //                 found = true;
    //                 break;
    //             }
    //         }
    //         if (!found) {
    //             skipGenericMatch = true;
    //             break;
    //         }
    //     }
    // }

    // Try finding a generic qml module match
    if (!skipGenericMatch) {
        const QString& genericModuleMatch = moduleMatch(originalModules, appropriateGenericModules);
        if (!genericModuleMatch.isEmpty()) // Match succeed
            return genericModuleMatch;
    }

    // If we can't find the type within generic qml module imports
    // continue with original qml file and its own module imports
    // Expensive operation, approx 170 ms each
    // Link link(manager->snapshot(), manager->defaultVContext(document->language(), document), manager->builtins(document));
    // ContextPtr context(link(document, nullptr));
    // const QList<ImportInfo>& importInfos = context->imports(document.data())->infos(typeName, context.data());
    // const QStringList& moduleNames = importsToModules(typeName, document.data(), importInfos);
    // if (!moduleNames.isEmpty())
    //     return moduleNames.last();

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

    if (begin > 0 && end > 0)
        return source.mid(begin, end - begin);

    return QString();
}

void removeProperty(QTextDocument* document, quint32 begin, quint32 end)
{
    bool modified = document->isModified();
    QTextCursor cursor(document);

    cursor.setPosition(end);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    bool forwardEmpty = !cursor.selectedText().contains(QRegularExpression("[^\\r\\n\\t\\f\\v ]"));
    cursor.clearSelection();

    if (forwardEmpty) {
        cursor.setPosition(begin);
        cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::KeepAnchor);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        bool backwardEmpty = !cursor.selectedText().contains(QRegularExpression("[^\\r\\n\\t\\f\\v ]"));
        cursor.clearSelection();

        if (!backwardEmpty)
            cursor.setPosition(begin);
    } else {
        cursor.setPosition(begin);
    }

    cursor.setPosition(end, QTextCursor::KeepAnchor);
    cursor.beginEditBlock();
    cursor.removeSelectedText();
    cursor.endEditBlock();

    if (!modified)
        document->setModified(false);
}

void removeProperty(QString& source, quint32 begin, quint32 end)
{
    QTextDocument document(source);
    removeProperty(&document, begin, end);
    source = document.toPlainText();
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

    const QString& rightBlock = cursor.selectedText();
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
        if (value.isEmpty())
            removeProperty(source, begin, end);
        else
            source.replace(begin, end - begin, property + ": " + value);
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
        if (value.isEmpty()) {
            removeProperty(document, begin, end);
        } else {
            bool modified = document->isModified();
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

int methodPosition(QTextDocument* document, const QString& url, const QString& methodSign, bool lbrace)
{
    QString source = document->toPlainText();
    QSharedPointer<Document> doc = Document::create(url, Dialect::JavaScript);
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

bool canParse(const QString& controlDir)
{
    const QString& mainQmlFilePath = SaveUtils::toControlMainQmlFile(controlDir);
    QFile file(mainQmlFilePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("ParserUtils: Cannot open file");
        return false;
    }
    QSharedPointer<Document> document = Document::create(mainQmlFilePath, Dialect::Qml);
    document->setSource(file.readAll());
    file.close();
    return document->parse();
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

    QSharedPointer<Document> document = Document::create(mainQmlFilePath, Dialect::Qml);
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

int moduleVersionMinor(const QString& module)
{
    const QStringList& pieces = module.split(QStringLiteral("/"));
    if (pieces.size() == 3) {
        const QStringList& versionPieces = pieces.at(1).split(QStringLiteral("."));
        if (versionPieces.size() == 2)
            return versionPieces.last().toInt();
    }
    return -1;
}

int moduleVersionMajor(const QString& module)
{
    const QStringList& pieces = module.split(QStringLiteral("/"));
    if (pieces.size() == 3) {
        const QStringList& versionPieces = pieces.at(1).split(QStringLiteral("."));
        if (versionPieces.size() == 2)
            return versionPieces.first().toInt();
    }
    return -1;
}

QString moduleBody(const QString& module)
{
    const QStringList& pieces = module.split(QStringLiteral("/"));
    if (pieces.size() >= 2)
        return pieces.first();
    return QString();
}

QString moduleTypeName(const QString& module)
{
    const QStringList& pieces = module.split(QStringLiteral("/"));
    if (pieces.size() >= 2)
        return pieces.last();
    return QString();
}

QString moduleToMajorModule(const QString& module)
{
    int versionMajor = ParserUtils::moduleVersionMajor(module);
    if (versionMajor >= 0) {
        return ParserUtils::moduleBody(module) + QStringLiteral("/") +
                QString::number(versionMajor) + QStringLiteral("/") +
                ParserUtils::moduleTypeName(module);
    }
    return module;
}

QString moduleBodyPlusMajorVersion(const QString& module)
{
    QString majorModule = module;
    int majorVersion = ParserUtils::moduleVersionMajor(module);
    if (majorVersion >= 0)
        majorModule = ParserUtils::moduleBody(module) + QStringLiteral("/") + QString::number(majorVersion);
    return majorModule;
}

QByteArray mockSource(const QString& url, const QString& module)
{
    static const QString& mockSign = QStringLiteral("Objectwheel_MOCK_FILES_54");
    static const QString& mockBase = QStringLiteral(":/mockfiles");
    static const QStringList& mockFiles = QDir(mockBase).entryList(QDir::Files);
    static const QString& importStatement = QStringLiteral(";import \"qrc%1\" as %2;");
    const QString& typeName = moduleTypeName(module);

    QFile file(url);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("ParserUtils: Cannot open file");
        return {};
    }
    QString source = file.readAll();
    file.close();

    if (!mockFiles.contains(typeName + QStringLiteral(".qml"), Qt::CaseInsensitive))
        return source.toUtf8();

    QSharedPointer<Document> doc = Document::create(url, Dialect::Qml);
    doc->setSource(source);

    if (!doc->parse()) {
        qWarning() << "Property couldn't read. Unable to parse qml file.";
        return source.toUtf8();
    }

    auto uiProgram = doc->qmlProgram();

    if (!uiProgram) {
        qWarning() << "Property couldn't read. Corrupted ui program.";
        return source.toUtf8();
    }

    auto uiObjectMemberList = uiProgram->members;

    if (!uiObjectMemberList) {
        qWarning() << "Property couldn't read. Empty source file.";
        return source.toUtf8();
    }

    auto uiHeaderItemList = uiProgram->headers;

    if (!uiHeaderItemList) {
        qWarning() << "Property couldn't read. Empty source file.";
        return source.toUtf8();
    }

    quint32 typeBegin = 0;
    quint32 lastImportEnd = 0;
    UiHeaderItemList* header = uiHeaderItemList;
    do {
        auto import = cast<UiImport*>(header->headerItem);
        if (!import) {
            header = header->next;
            continue;
        }
        lastImportEnd = import->lastSourceLocation().end();
        header = header->next;
    } while(header);

    auto uiObjectDefinition = cast<UiObjectDefinition *>(uiObjectMemberList->member);

    if (!uiObjectDefinition) {
        qWarning() << "Property couldn't read. Bad file format 0x1.";
        return source.toUtf8();
    }

    auto qualifiedId = cast<UiQualifiedId *>(uiObjectDefinition->qualifiedTypeNameId);

    if (!qualifiedId) {
        qWarning() << "Property couldn't read. Bad file format 0x2.";
        return source.toUtf8();
    }

    typeBegin = qualifiedId->firstSourceLocation().begin();

    if (typeBegin > 0 && lastImportEnd > 0) { // We edit lower statement first
        source.insert(typeBegin, mockSign + QStringLiteral("."));
        source.insert(lastImportEnd, importStatement.arg(mockBase).arg(mockSign));
    }

    return source.toUtf8();
}

QString id(const QString& controlDir)
{
    if (Internal::canParse(controlDir))
        return property(controlDir, "id");
    return SaveUtils::controlId(controlDir);
}

QString module(const QString& controlDir)
{
    // NOTE: Exception for spacers
    const QString& toolName = QFileInfo(controlDir).fileName();
    if (toolName.contains("Spacer"))
        return "QtQuick.Layouts/1.12/" + toolName;

    const QString& mainQmlFilePath = SaveUtils::toControlMainQmlFile(controlDir);
    QFile file(mainQmlFilePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("ParserUtils: Cannot open file");
        return {};
    }
    const QString& source = file.readAll();
    file.close();
    QSharedPointer<Document> doc = Document::create(mainQmlFilePath, Dialect::Qml);
    doc->setSource(source);

    if (!doc->parse()) {
        qWarning() << "Property couldn't read. Unable to parse qml file.";
        return QString();
    }

    if (doc->qmlProgram() == 0
            || doc->qmlProgram()->members == 0
            || doc->qmlProgram()->members->member == 0) {
        return QString();
    }

    if (auto type = QmlJS::qualifiedTypeNameId(doc->qmlProgram()->members->member)) {
        const QString& typeName = type->name.toString();
        if (!typeName.isEmpty()) {
            const QString& module = Internal::resolveModule(
                        doc, typeName,
                        Internal::importsToModules(typeName, doc.data(), doc->bind()->imports()));
            if (!module.isEmpty())
                return module;
        }
    }

    return QString();
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
    QSharedPointer<Document> document = Document::create(mainQmlFilePath, Dialect::Qml);
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
    QSharedPointer<Document> document = Document::create(SaveUtils::toControlMainQmlFile(controlDir), Dialect::Qml);
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
    if (Internal::canParse(controlDir))
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

    QSharedPointer<Document> document = Document::create(mainQmlFilePath, Dialect::Qml);
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
    else if (!value.isEmpty())
        Internal::addProperty(source, uiObjectInitializer, property, value);

    file.resize(0);
    file.write(source.toUtf8());
    file.close();
}

void setProperty(QTextDocument* doc, const QString& controlDir, const QString& property, const QString& value)
{
    QSharedPointer<Document> document = Document::create(SaveUtils::toControlMainQmlFile(controlDir), Dialect::Qml);
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
    else if (!value.isEmpty())
        Internal::addProperty(doc, uiObjectInitializer, property, value);
}

int addMethod(QTextDocument* document, const QString& url, const QString& method)
{
    quint32 begin = 0;
    const QString& source = document->toPlainText();
    QSharedPointer<Document> doc = Document::create(url, Dialect::JavaScript);
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
    QSharedPointer<Document> doc = Document::create(url, Dialect::JavaScript);
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

void addConnection(QTextDocument* document, const QString& url, const QString& loaderSign, const QString& connection)
{
    int i = Internal::methodPosition(document, url, loaderSign, true);
    if (i > 0) {
        QTextCursor cursor(document);
        cursor.beginEditBlock();
        cursor.setPosition(i);
        cursor.insertText("\n    " + connection);
        cursor.endEditBlock();
    }
}

} // ParserUtils