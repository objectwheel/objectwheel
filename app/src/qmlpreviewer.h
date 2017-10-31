#ifndef QMLPREVIEWER_H
#define QMLPREVIEWER_H

#include <QObject>
#include <QPointer>
#include <QWidget>
#include <QQmlError>
#include <QList>
#include <QMap>
#include <global.h>

#define NONGUI_CONTROL_SIZE (fit(40))

class Control;
class QmlPreviewerPrivate;

typedef QMap<QString, QVariant> PropertyMap;

struct PropertyNode {
        QString cleanClassName;
        const QMetaObject* metaObject = 0;
        PropertyMap propertyMap;
};

typedef QList<PropertyNode> PropertyNodes;

struct PreviewResult {
        bool gui;
        QPixmap preview;
        Control* control;
        QList<QString> events;
        PropertyNodes properties;
        QList<QQmlError> errors;
        inline bool hasError() {
            return !errors.isEmpty();
        }
        inline QVariant property(const QString& name) const {
            for (const auto& node : properties) {
                if (node.propertyMap.contains(name))
                    return node.propertyMap.value(name);
            }
            return QVariant();
        }
};

class QmlPreviewer : public QObject
{
        Q_OBJECT
        friend class QmlPreviewerPrivate;

    public:
        explicit QmlPreviewer(QObject *parent = 0);
        static QmlPreviewer* instance();
        static void requestPreview(Control* control);
        static bool working();

    signals:
        void previewReady(const PreviewResult& result);
        void workingChanged(bool value);

    private:
        static QmlPreviewerPrivate* _d;
        static bool _working;
};

#endif // QMLPREVIEWER_H
