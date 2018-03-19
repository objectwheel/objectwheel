#include <previewresult.h>

bool PreviewResult::hasError() const
{
    return !errors.isEmpty();
}

QVariant PreviewResult::property(const QString& name) const
{
    for (const auto& node : propertyNodes)
        if (node.properties.contains(name))
            return node.properties.value(name);

    return QVariant();
}

QDataStream& operator>>(QDataStream& in, QQmlError& error)
{
    int c, l, m;
    QString d;
    QUrl u;

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

QDataStream& operator<<(QDataStream& out, const QQmlError& error)
{
    out << error.column();
    out << error.description();
    out << error.line();
    out << int(error.messageType());
    out << error.url();

    return out;
}

QDataStream& operator>>(QDataStream& in, PropertyNode& node)
{
    in >> node.cleanClassName;
    in >> node.properties;
    in >> node.enums;

    return in;
}

QDataStream& operator<<(QDataStream& out, const PropertyNode& node)
{
    out << node.cleanClassName;
    out << node.properties;
    out << node.enums;

    return out;
}

QDataStream& operator>>(QDataStream& in, PreviewResult& result)
{
    in >> result.gui;
    in >> result.uid;
    in >> result.preview;
    in >> result.events;
    in >> result.errors;
    in >> result.propertyNodes;
    in >> result.dirtyUids;

    return in;
}

QDataStream& operator<<(QDataStream& out, const PreviewResult& result)
{
    out << result.gui;
    out << result.uid;
    out << result.preview;
    out << result.events;
    out << result.errors;
    out << result.propertyNodes;
    out << result.dirtyUids;

    return out;
}

QDataStream& operator>>(QDataStream& in, Enum& e)
{
    in >> e.name;
    in >> e.scope;
    in >> e.value;
    in >> e.keys;

    return in;
}

QDataStream& operator<<(QDataStream& out, const Enum& e)
{
    out << e.name;
    out << e.scope;
    out << e.value;
    out << e.keys;

    return out;
}

QDataStream& operator>>(QDataStream& in, AnchorLine& anchorLine)
{
    in >> anchorLine.id;
    in >> anchorLine.anchor;

    return in;
}

QDataStream& operator<<(QDataStream& out, const AnchorLine& anchorLine)
{
    out << anchorLine.id;
    out << anchorLine.anchor;

    return out;
}

QDataStream& operator>>(QDataStream& in, Anchors& anchors)
{
    in >> anchors.uid;
    in >> anchors.top;
    in >> anchors.left;
    in >> anchors.right;
    in >> anchors.bottom;
    in >> anchors.verticalCenter;
    in >> anchors.horizontalCenter;
    in >> anchors.margins;
    in >> anchors.topMargin;
    in >> anchors.leftMargin;
    in >> anchors.rightMargin;
    in >> anchors.bottomMargin;
    in >> anchors.verticalCenterOffset;
    in >> anchors.horizontalCenterOffset;

    return in;
}

QDataStream& operator<<(QDataStream& out, const Anchors& anchors)
{
    out << anchors.uid;
    out << anchors.top;
    out << anchors.left;
    out << anchors.right;
    out << anchors.bottom;
    out << anchors.verticalCenter;
    out << anchors.horizontalCenter;
    out << anchors.margins;
    out << anchors.topMargin;
    out << anchors.leftMargin;
    out << anchors.rightMargin;
    out << anchors.bottomMargin;
    out << anchors.verticalCenterOffset;
    out << anchors.horizontalCenterOffset;

    return out;
}
