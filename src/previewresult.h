#ifndef PREVIEWRESULT_H
#define PREVIEWRESULT_H

#include <QMap>
#include <QImage>
#include <QVariant>
#include <QQmlError>
#include <QMetaEnum>

class QQmlComponent;

struct Enum {
    QString name;
    QString scope;
    QString value;
    QHash<QString, int> keys;
    QMetaEnum metaEnum;
};
Q_DECLARE_METATYPE(Enum)

struct PropertyNode
{
    QString cleanClassName;
    QMap<QString, QVariant> properties;
    QList<Enum> enums;
};

class PreviewResult
{
    public:
        bool hasError() const;
        QVariant property(const QString& name) const;

    public:
        bool gui;
        QString uid;
        QImage preview;
        QObject* object;
        QQmlComponent* component;
        QList<QString> events;
        QList<QQmlError> errors;
        QList<PropertyNode> propertyNodes;
};

QDataStream& operator<<(QDataStream& out, const Enum& e);
QDataStream& operator<<(QDataStream& out, const PropertyNode& node);
QDataStream& operator<<(QDataStream& out, const PreviewResult& result);
QDataStream& operator>>(QDataStream& in, Enum& e);
QDataStream& operator>>(QDataStream& in, PropertyNode& node);
QDataStream& operator>>(QDataStream& in, PreviewResult& result);

#endif // PREVIEWRESULT_H