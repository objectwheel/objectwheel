#ifndef RENDERRESULT_H
#define RENDERRESULT_H

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
    QList<Enum> enums;
    QMap<QString, QVariant> properties;
};

struct RenderResult {
    bool gui;
    bool popup;
    bool window;
    bool codeChanged;
    QString id;
    QString uid;
    QImage image;
    QRectF boundingRect;
    QList<QmlError> errors;
    QList<QString> events;
    QList<PropertyNode> properties;
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

inline QDataStream& operator>> (QDataStream& in, RenderResult& result)
{
    in >> result.gui;
    in >> result.id;
    in >> result.uid;
    in >> result.image;
    in >> result.boundingRect;
    in >> result.popup;
    in >> result.window;
    in >> result.codeChanged;
    in >> result.errors;
    in >> result.events;
    in >> result.properties;
    return in;
}

inline QDataStream& operator<< (QDataStream& out, const RenderResult& result)
{
    out << result.gui;
    out << result.id;
    out << result.uid;
    out << result.image;
    out << result.boundingRect;
    out << result.popup;
    out << result.window;
    out << result.codeChanged;
    out << result.errors;
    out << result.events;
    out << result.properties;
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

#endif // RENDERRESULT_H
