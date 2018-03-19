#ifndef PREVIEWRESULT_H
#define PREVIEWRESULT_H

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

struct AnchorLine {
    QString id;
    QString anchor;
};

struct Anchors {
    QString uid;
    AnchorLine top;
    AnchorLine left;
    AnchorLine right;
    AnchorLine bottom;
    AnchorLine verticalCenter;
    AnchorLine horizontalCenter;
    AnchorLine fill;
    AnchorLine centerIn;
    qreal margins                = 0.0;
    qreal topMargin              = 0.0;
    qreal leftMargin             = 0.0;
    qreal rightMargin            = 0.0;
    qreal bottomMargin           = 0.0;
    qreal verticalCenterOffset   = 0.0;
    qreal horizontalCenterOffset = 0.0;
};

struct PropertyNode {
    QString cleanClassName;
    QMap<QString, QVariant> properties;
    QList<Enum> enums;
};

struct PreviewResult {
    bool hasError() const;
    QVariant property(const QString& name) const;

    /* Transferred */
    bool gui;
    QString uid;
    QImage preview;
    QList<QString> events;
    QList<QQmlError> errors;
    QList<PropertyNode> propertyNodes;
    QList<QString> dirtyUids;

    /* Locally needed */
    QString id;
    QSizeF size;
    QString url;
    QObject* object;
};

QDataStream& operator>>(QDataStream& in, Enum& e);
QDataStream& operator<<(QDataStream& out, const Enum& e);
QDataStream& operator>>(QDataStream& in, PropertyNode& node);
QDataStream& operator<<(QDataStream& out, const PropertyNode& node);
QDataStream& operator>>(QDataStream& in, PreviewResult& result);
QDataStream& operator<<(QDataStream& out, const PreviewResult& result);
QDataStream& operator>>(QDataStream& in, AnchorLine& anchorLine);
QDataStream& operator<<(QDataStream& out, const AnchorLine& anchorLine);
QDataStream& operator>>(QDataStream& in, Anchors& anchors);
QDataStream& operator<<(QDataStream& out, const Anchors& anchors);

#endif // PREVIEWRESULT_H