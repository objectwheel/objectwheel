#ifndef PARSERUTILS_H
#define PARSERUTILS_H

#include <QString>

class QVariant;
class QTextDocument;

class ParserUtils
{
        Q_DISABLE_COPY(ParserUtils)

    public:
        static void setProperty(const QString& fileName, const QString& property, const QString& value);
        static void setProperty(QTextDocument* document, const QString& fileName, const QString& property, const QString& value);
        static void setProperty(const QString& fileName, const QString& property, const QVariant& value);
        static void setProperty(QTextDocument* document, const QString& fileName, const QString& property, const QVariant& value);

    private:
        ParserUtils() {}

};

#endif // PARSERUTILS_H