#ifndef PARSERUTILS_H
#define PARSERUTILS_H

#include <QString>

class QTextDocument;

namespace ParserUtils {

bool exists(const QString& controlDir, const QString& property);

int moduleVersionMinor(const QString& module);
int moduleVersionMajor(const QString& module);
QString moduleBody(const QString& module); // without version
QString moduleTypeName(const QString& module);
QString moduleToMajorModule(const QString& module);
QString moduleBodyPlusMajorVersion(const QString& module);
QByteArray mockSource(const QString& url, const QString& module);

QString id(const QString& controlDir);
QString module(const QString& controlDir);
QString property(const QString& controlDir, const QString& property);
QString property(QTextDocument* document, const QString& controlDir, const QString& property);

void setId(const QString& controlDir, const QString& id);
void setProperty(const QString& controlDir, const QString& property, const QString& value);
void setProperty(QTextDocument* document, const QString& controlDir, const QString& property, const QString& value);

int addMethod(QTextDocument* document, const QString& url, const QString& method);
int methodLine(QTextDocument* document, const QString& url, const QString& methodSign);
void addConnection(QTextDocument* document, const QString& url, const QString& loaderSign, const QString& connection);

} // ParserUtils

#endif // PARSERUTILS_H
