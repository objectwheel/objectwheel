#ifndef PARSERUTILS_H
#define PARSERUTILS_H

#include <utils_global.h>
#include <QString>

class QTextDocument;

class UTILS_EXPORT ParserUtils final
{
    Q_DISABLE_COPY(ParserUtils)

    friend class SaveManager; // For setId and setProperty
    friend class CentralWidget; // For setProperty
    friend class ControlSaveFilter; // For setProperty and property

public:
    static bool canParse(const QString& url);
    static bool canParse(QTextDocument* document, const QString& url);
    static bool exists(const QString& url, const QString& property);

    static QString id(const QString& url);
    static QString property(const QString& url, const QString& property);
    static QString property(QTextDocument* document, const QString& url, const QString& property);

    static int addMethod(QTextDocument* document, const QString& url, const QString& method);
    static int methodPosition(QTextDocument* document, const QString& url, const QString& methodSign);

private:
    static void setId(const QString& url, const QString& id);
    static void setProperty(const QString& url, const QString& property, const QString& value);
    static void setProperty(QTextDocument* document, const QString& url, const QString& property,
                            const QString& value);

private:
    ParserUtils() {}

};

#endif // PARSERUTILS_H
