#ifndef QMLFORMATTER_H
#define QMLFORMATTER_H

#include <QObject>

class QPlainTextEdit;

class QmlFormatter : public QObject
{
        Q_OBJECT

    public:
        explicit QmlFormatter(QObject *parent = nullptr);
        static void format(QString& text);
};

#endif // QMLFORMATTER_H
