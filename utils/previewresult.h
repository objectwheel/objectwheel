#ifndef PREVIEWRESULT_H
#define PREVIEWRESULT_H

#include <QMap>
#include <QHash>
#include <QImage>
#include <QQmlError>

class QVariant;

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

struct PreviewResult {
    bool gui;
    bool window;
    QString uid;
    QImage preview;
    QList<QQmlError> errors;
    QList<PropertyNode> properties;
};

inline QDataStream& operator>>(QDataStream& in, QQmlError& error)
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

inline QDataStream& operator<<(QDataStream& out, const QQmlError& error)
{
    out << error.column();
    out << error.description();
    out << error.line();
    out << int(error.messageType());
    out << error.url();
    return out;
}

inline QDataStream& operator>>(QDataStream& in, PropertyNode& node)
{
    in >> node.cleanClassName;
    in >> node.properties;
    in >> node.enums;
    return in;
}

inline QDataStream& operator<<(QDataStream& out, const PropertyNode& node)
{
    out << node.cleanClassName;
    out << node.properties;
    out << node.enums;
    return out;
}

inline QDataStream& operator>>(QDataStream& in, PreviewResult& result)
{
    in >> result.gui;
    in >> result.uid;
    in >> result.preview;
    in >> result.window;
    in >> result.errors;
    in >> result.properties;
    return in;
}

inline QDataStream& operator<<(QDataStream& out, const PreviewResult& result)
{
    out << result.gui;
    out << result.uid;
    out << result.preview;
    out << result.window;
    out << result.errors;
    out << result.properties;
    return out;
}

inline QDataStream& operator>>(QDataStream& in, Enum& e)
{
    in >> e.name;
    in >> e.scope;
    in >> e.value;
    in >> e.keys;
    return in;
}

inline QDataStream& operator<<(QDataStream& out, const Enum& e)
{
    out << e.name;
    out << e.scope;
    out << e.value;
    out << e.keys;
    return out;
}

#endif // PREVIEWRESULT_H
