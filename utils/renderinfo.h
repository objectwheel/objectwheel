#ifndef RENDERINFO_H
#define RENDERINFO_H

#include <QMap>
#include <QHash>
#include <QImage>
#include <QVariant>
#include <qmlerror.h>

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
    bool window;
    bool visible;
    bool codeChanged;
    QString geometryHash;
    QString uid;
    QImage image;
    QRectF boundingRect;
    QVector<QmlError> errors;
    QVector<QString> events;
    QVector<PropertyNode> properties;
};

inline QDataStream& operator>> (QDataStream& in, QmlError& error)
{
    QUrl u;
    QString d;
    int c, l, m;

    in >> c;
    in >> d;
    in >> l;
    in >> m;
    in >> u;

    error.setColumn(c);
    error.setDescription(d);
    error.setLine(l);
    error.setMessageType(QtMsgType(m));
    error.setUrl(u);
    return in;
}

inline QDataStream& operator<< (QDataStream& out, const QmlError& error)
{
    out << error.column();
    out << error.description();
    out << error.line();
    out << int(error.messageType());
    out << error.url();
    return out;
}

inline QDataStream& operator>> (QDataStream& in, PropertyNode& node)
{
    in >> node.cleanClassName;
    in >> node.properties;
    in >> node.enums;
    return in;
}

inline QDataStream& operator<< (QDataStream& out, const PropertyNode& node)
{
    out << node.cleanClassName;
    out << node.properties;
    out << node.enums;
    return out;
}

inline QDataStream& operator>> (QDataStream& in, RenderInfo& info)
{
    in >> info.gui;
    in >> info.uid;
    in >> info.image;
    in >> info.boundingRect;
    in >> info.popup;
    in >> info.window;
    in >> info.visible;
    in >> info.codeChanged;
    in >> info.geometryHash;
    in >> info.errors;
    in >> info.events;
    in >> info.properties;
    return in;
}

inline QDataStream& operator<< (QDataStream& out, const RenderInfo& info)
{
    out << info.gui;
    out << info.uid;
    out << info.image;
    out << info.boundingRect;
    out << info.popup;
    out << info.window;
    out << info.visible;
    out << info.codeChanged;
    out << info.geometryHash;
    out << info.errors;
    out << info.events;
    out << info.properties;
    return out;
}

inline QDataStream& operator>> (QDataStream& in, Enum& e)
{
    in >> e.name;
    in >> e.scope;
    in >> e.value;
    in >> e.keys;
    return in;
}

inline QDataStream& operator<< (QDataStream& out, const Enum& e)
{
    out << e.name;
    out << e.scope;
    out << e.value;
    out << e.keys;
    return out;
}

#endif // RENDERINFO_H
