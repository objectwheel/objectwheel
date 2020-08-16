#ifndef RENDERINFO_H
#define RENDERINFO_H

#include <utils_global.h>
#include <qmlerror.h>

#include <QMap>
#include <QHash>
#include <QImage>
#include <QVariant>

struct Enum {
    QString name;
    QString scope;
    QString value;
    QHash<QString, int> keys;
};
Q_DECLARE_METATYPE(Enum)

struct PropertyNode {
    QString cleanClassName;
    QVector<Enum> enums;
    QMap<QString, QVariant> properties;
};

struct RenderInfo {
    bool gui;
    bool popup;
    bool overlayPopup;
    bool window;
    bool visible;
    bool codeChanged;
    QString geometrySyncKey;
    QString uid;
    QImage image;
    QMarginsF margins;
    QRectF surroundingRect;
    QVariantMap anchors;
    QVector<QmlError> errors;
    QVector<QString> events;
    QVector<PropertyNode> properties;
};

struct InitInfo {
    // Path, Module
    QVector<QPair<QString, QString>> forms;
    // Form path, Children
    QHash<QString, QVector<QPair<QString, QString>>> children;
};

UTILS_EXPORT inline QDataStream& operator>> (QDataStream& in, PropertyNode& node)
{
    in >> node.cleanClassName;
    in >> node.properties;
    in >> node.enums;
    return in;
}

UTILS_EXPORT inline QDataStream& operator<< (QDataStream& out, const PropertyNode& node)
{
    out << node.cleanClassName;
    out << node.properties;
    out << node.enums;
    return out;
}

UTILS_EXPORT inline QDataStream& operator>> (QDataStream& in, RenderInfo& info)
{
    in >> info.gui;
    in >> info.uid;
    in >> info.image;
    in >> info.margins;
    in >> info.surroundingRect;
    in >> info.popup;
    in >> info.overlayPopup;
    in >> info.window;
    in >> info.visible;
    in >> info.codeChanged;
    in >> info.geometrySyncKey;
    in >> info.anchors;
    in >> info.errors;
    in >> info.events;
    in >> info.properties;
    return in;
}

UTILS_EXPORT inline QDataStream& operator<< (QDataStream& out, const RenderInfo& info)
{
    out << info.gui;
    out << info.uid;
    out << info.image;
    out << info.margins;
    out << info.surroundingRect;
    out << info.popup;
    out << info.overlayPopup;
    out << info.window;
    out << info.visible;
    out << info.codeChanged;
    out << info.geometrySyncKey;
    out << info.anchors;
    out << info.errors;
    out << info.events;
    out << info.properties;
    return out;
}

UTILS_EXPORT inline QDataStream& operator>> (QDataStream& in, Enum& e)
{
    in >> e.name;
    in >> e.scope;
    in >> e.value;
    in >> e.keys;
    return in;
}

UTILS_EXPORT inline QDataStream& operator<< (QDataStream& out, const Enum& e)
{
    out << e.name;
    out << e.scope;
    out << e.value;
    out << e.keys;
    return out;
}

UTILS_EXPORT inline QDataStream& operator>> (QDataStream& in, InitInfo& info)
{
    in >> info.forms;
    in >> info.children;
    return in;
}

UTILS_EXPORT inline QDataStream& operator<< (QDataStream& out, const InitInfo& info)
{
    out << info.forms;
    out << info.children;
    return out;
}

#endif // RENDERINFO_H
