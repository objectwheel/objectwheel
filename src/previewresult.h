#ifndef PREVIEWRESULT_H
#define PREVIEWRESULT_H

#include <QMap>
#include <QImage>
#include <QVariant>
#include <QQmlError>
#include <QMetaObject>

struct PropertyNode
{
    QString cleanClassName;
    QMap<QString, QVariant> properties;
};

class PreviewResult
{
    public:
        bool hasError() const;
        QVariant property(const QString& name) const;

    public:
        bool gui;
        bool window;
        QString uid;
        QImage preview;
        QObject* object;
        QList<QString> events;
        QList<QQmlError> errors;
        QList<PropertyNode> propertyNodes;
};

QDataStream& operator<<(QDataStream& out, const PropertyNode& node);
QDataStream& operator<<(QDataStream& out, const PreviewResult& result);
QDataStream& operator>>(QDataStream& in, PropertyNode& node);
QDataStream& operator>>(QDataStream& in, PreviewResult& result);

#endif // PREVIEWRESULT_H