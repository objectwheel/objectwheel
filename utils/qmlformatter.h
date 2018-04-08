#ifndef QMLFORMATTER_H
#define QMLFORMATTER_H

#include <utils_global.h>
#include <QObject>

class QPlainTextEdit;

class UTILS_EXPORT QmlFormatter : public QObject
{
        Q_OBJECT

    public:
        explicit QmlFormatter(QObject *parent = nullptr);
        static void format(QString& text);
};

#endif // QMLFORMATTER_H
