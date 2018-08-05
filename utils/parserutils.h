#ifndef PARSERUTILS_H
#define PARSERUTILS_H

#include <utils_global.h>
#include <QString>

class QTextDocument;

class UTILS_EXPORT ParserUtils final
{
    Q_DISABLE_COPY(ParserUtils)

public:
    static bool exists(const QString& url, const QString& property);
    static bool contains(const QString& url, const QString& property);

    static qreal x(const QString& url);
    static qreal y(const QString& url);
    static qreal z(const QString& url);
    static qreal width(const QString& url);
    static qreal height(const QString& url);
    static QString id(const QString& url);
    static QString property(const QString& url, const QString& property);
    static QString property(QTextDocument* document, const QString& url, const QString& property);

    static void setX(const QString& url, qreal x);
    static void setY(const QString& url, qreal y);
    static void setZ(const QString& url, qreal z);
    static void setWidth(const QString& url, qreal width);
    static void setHeight(const QString& url, qreal height);
    static void setId(const QString& url, const QString& id);
    static void setProperty(const QString& url, const QString& property, const QString& value);
    static void setProperty(QTextDocument* document, const QString& url, const QString& property, const QString& value);

private:
    ParserUtils() {}

};

#endif // PARSERUTILS_H
