#ifndef PARSERUTILS_H
#define PARSERUTILS_H

#include <QString>

class QTextDocument;

namespace ParserUtils
{
bool canParse(const QString& url);
bool canParse(QTextDocument* document, const QString& url);
bool exists(const QString& url, const QString& property);

QString id(const QString& url);
QString property(const QString& url, const QString& property);
QString property(QTextDocument* document, const QString& url, const QString& property);

int addMethod(QTextDocument* document, const QString& url, const QString& method);
int methodLine(QTextDocument* document, const QString& url, const QString& methodSign);
int methodPosition(QTextDocument* document, const QString& url, const QString& methodSign, bool lbrace);
void addConnection(QTextDocument* document, const QString& url, const QString& loaderSign, const QString& connection);

void setId(const QString& url, const QString& id);
void setProperty(const QString& url, const QString& property, const QString& value);
void setProperty(QTextDocument* document, const QString& url, const QString& property, const QString& value);
}

#endif // PARSERUTILS_H
