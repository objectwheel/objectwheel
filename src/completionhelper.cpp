#include <completionhelper.h>
#include <qmlcodeeditor.h>
#include <fit.h>

#include <QAbstractItemView>



static QStringList generateKeywords();
static QStringList generateProperties(const QString& text);
static QStringList generateFunctions(const QString& text);
static QStringList generateTypes(const QString& text);

CompletionHelper::CompletionHelper(QObject* parent) : QObject(parent)
{
}

void CompletionHelper::extractCompletions(const QString& text)
{
    ExtractionResult result;
    result.keywords = generateKeywords();
    result.properties = generateProperties(text);
    result.functions = generateFunctions(text);
    result.types = generateTypes(text);
    emit extractionReady(result);
}

QStringList generateKeywords()
{
    static QStringList keywords;
    if (keywords.isEmpty()) {
        keywords << "char" << "class" << "const"
                 << "double" << "enum" << "explicit"
                 << "friend" << "inline" << "int"
                 << "long" << "namespace" << "operator"
                 << "private" << "protected" << "public"
                 << "short" << "signals" << "signed"
                 << "slots" << "static" << "struct"
                 << "template" << "typedef" << "typename"
                 << "union" << "unsigned" << "virtual"
                 << "void" << "volatile" << "property"
                 << "vector4d" << "vector3d" << "vector2d"
                 << "var" << "variant" << "url"
                 << "time" << "string" << "size"
                 << "rect" << "real" << "quaternion"
                 << "point" << "matrix4x4" << "list"
                 << "font" << "enumeration" << "date"
                 << "color" << "action" << "on"
                 << "import" << "readonly" << "property"
                 << "signal" << "alias" << "yield"
                 << "try" << "throw" << "super"
                 << "null" << "let" << "in"
                 << "goto" << "finally" << "export"
                 << "break" << "abstract" << "byte"
                 << "default" << "else" << "extends"
                 << "if" << "instanceof" << "package"
                 << "return" << "switch" << "throws"
                 << "typeof" << "while" << "await"
                 << "case" << "delete" << "false"
                 << "for" << "implements" << "native"
                 << "private" << "synchronized" << "transient"
                 << "with" << "boolean" << "catch"
                 << "continue" << "do" << "eval"
                 << "final" << "function" << "interface"
                 << "new" << "this" << "true" ;
        keywords.removeDuplicates();
    }
    return keywords;
}

QStringList generateProperties(const QString& text)
{   
    QRegularExpression regex("^(!?(\\s+)?)[a-z][\\w\\.]+([ \\t ]+)?(?=:)",
                             QRegularExpression::MultilineOption |
                             QRegularExpression::DotMatchesEverythingOption);
    auto i = regex.globalMatch(text);
    QStringList words;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString word = match.captured(0);
        word.remove("\t");
        word.remove(" ");
        if (word.contains("."))
            words << word.split(".");
        else
            words << word;
    }
    words.removeDuplicates();
    return words;
}

QStringList generateFunctions(const QString& text)
{
    QRegularExpression regex("\\b[A-Za-z0-9_]+(?=\\()", QRegularExpression::MultilineOption
                             | QRegularExpression::DotMatchesEverythingOption);
    auto i = regex.globalMatch(text);
    QStringList words;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString word = match.captured(0);
        words << word;
    }
    words.removeDuplicates();
    return words;
}

QStringList generateTypes(const QString& text)
{
    auto final = text;

    QRegularExpression regex("\".*\"", QRegularExpression::MultilineOption);

    auto i = regex.globalMatch(final);
    QStringList exceptions;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString word = match.captured(0);
        exceptions << word;
    }

    for (auto exception : exceptions)
        final.remove(exception);

    QRegularExpression regex2("\\b[A-Z][A-Za-z]+\\b", QRegularExpression::MultilineOption
                              | QRegularExpression::DotMatchesEverythingOption);

    auto j = regex2.globalMatch(final);
    QStringList words;
    while (j.hasNext()) {
        QRegularExpressionMatch match = j.next();
        QString word = match.captured(0);
        words << word;
    }
    words.removeDuplicates();
    return words;
}
