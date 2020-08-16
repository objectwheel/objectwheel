#ifndef PARSERUTILS_H
#define PARSERUTILS_H

#include <utils_global.h>
#include <QString>

class QTextDocument;

namespace ParserUtils {

UTILS_EXPORT bool exists(const QString& controlDir, const QString& property);

UTILS_EXPORT int moduleVersionMinor(const QString& module);
UTILS_EXPORT int moduleVersionMajor(const QString& module);
UTILS_EXPORT QString moduleBody(const QString& module); // without version
UTILS_EXPORT QString moduleTypeName(const QString& module);
UTILS_EXPORT QString moduleToMajorModule(const QString& module);
UTILS_EXPORT QString moduleBodyPlusMajorVersion(const QString& module);
UTILS_EXPORT QByteArray mockSource(const QString& url, const QString& module);

UTILS_EXPORT QString id(const QString& controlDir);
UTILS_EXPORT QString module(const QString& controlDir);
UTILS_EXPORT QString property(const QString& controlDir, const QString& property);
UTILS_EXPORT QString property(QTextDocument* document, const QString& controlDir, const QString& property);

UTILS_EXPORT void setId(const QString& controlDir, const QString& id);
UTILS_EXPORT void setProperty(const QString& controlDir, const QString& property, const QString& value);
UTILS_EXPORT void setProperty(QTextDocument* document, const QString& controlDir, const QString& property, const QString& value);

UTILS_EXPORT int addMethod(QTextDocument* document, const QString& url, const QString& method);
UTILS_EXPORT int methodLine(QTextDocument* document, const QString& url, const QString& methodSign);
UTILS_EXPORT void addConnection(QTextDocument* document, const QString& url, const QString& loaderSign, const QString& connection);

} // ParserUtils

#endif // PARSERUTILS_H
