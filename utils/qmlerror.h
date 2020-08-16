#ifndef QMLERROR_H
#define QMLERROR_H

#include <utils_global.h>
#include <QUrl>
#include <QDataStream>

class UTILS_EXPORT QmlError
{
public:
    QUrl url;
    QString description;
    int line = -1;
    int column = -1;
    int messageType = QtInfoMsg;

    QString toString() const {
        QString rv;
        QUrl u(url);
        int l(line);
        if (u.isEmpty() || (u.isLocalFile() && u.path().isEmpty()))
            rv += QLatin1String("<Unknown File>");
        else
            rv += u.toString();
        if (l != -1) {
            rv += QLatin1Char(':') + QString::number(l);
            int c(column);
            if (c != -1)
                rv += QLatin1Char(':') + QString::number(c);
        }
        rv += QLatin1String(": ") + description;
        return rv;
    }
};

UTILS_EXPORT inline QDataStream& operator>> (QDataStream& in, QmlError& error)
{
    in >> error.url;
    in >> error.line;
    in >> error.column;
    in >> error.description;
    in >> error.messageType;
    return in;
}

UTILS_EXPORT inline QDataStream& operator<< (QDataStream& out, const QmlError& error)
{
    out << error.url;
    out << error.line;
    out << error.column;
    out << error.description;
    out << error.messageType;
    return out;
}

#endif // QMLERROR_H
