#ifndef PARSERUTILS_H
#define PARSERUTILS_H

#include <utils_global.h>
#include <QString>

class QTextDocument;

class UTILS_EXPORT ParserUtils
{
    Q_DISABLE_COPY(ParserUtils)

public:
    static bool contains(const QString& fileName, const QString& property);
    static bool exists(const QString& fileName, const QString& property);
    static QString property(const QString& fileName, const QString& property);
    static void setProperty(const QString& fileName, const QString& property, const QString& value);
    static void setProperty(QTextDocument* document, const QString& fileName, const QString& property, const QString& value);

private:
    ParserUtils() {}

};

#endif // PARSERUTILS_H
