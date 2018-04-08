#ifndef PREVIEWRESULT_H
#define PREVIEWRESULT_H

#include <utils_global.h>
#include <QMap>
#include <QImage>
#include <QVariant>
#include <QQmlError>

class QQmlComponent;
class QQuickWindow;

struct Enum {
    QString name;
    QString scope;
    QString value;
    QHash<QString, int> keys;
};
Q_DECLARE_METATYPE(Enum)

struct PropertyNode {
    QString cleanClassName;
    QMap<QString, QVariant> properties;
    QList<Enum> enums;
};

struct UTILS_EXPORT PreviewResult {
    bool hasError() const;
    QVariant property(const QString& name) const;

    /* Transferred */
    bool gui;
    QString uid;
    QImage preview;
    bool repreviewed;
    QList<QString> events;
    QList<QQmlError> errors;
    QList<PropertyNode> propertyNodes;
    QList<QString> dirtyUids;

    /* Locally needed */
    QString url;
    QObject* object = nullptr;
};

UTILS_EXPORT QDataStream& operator>>(QDataStream& in, Enum& e);
UTILS_EXPORT QDataStream& operator<<(QDataStream& out, const Enum& e);
UTILS_EXPORT QDataStream& operator>>(QDataStream& in, PropertyNode& node);
UTILS_EXPORT QDataStream& operator<<(QDataStream& out, const PropertyNode& node);
UTILS_EXPORT QDataStream& operator>>(QDataStream& in, PreviewResult& result);
UTILS_EXPORT QDataStream& operator<<(QDataStream& out, const PreviewResult& result);

#endif // PREVIEWRESULT_H
