#ifndef PREVIEWBACKEND_H
#define PREVIEWBACKEND_H

#include <QObject>
#include <QPointer>
#include <QWidget>
#include <QQmlError>
#include <QList>
#include <QMap>
#include <global.h>

class Control;
class PreviewBackendPrivate;

typedef QMap<QString, QVariant> PropertyMap;

struct PropertyNode {
        QString cleanClassName;
        QMetaObject metaObject;
        PropertyMap propertyMap;
};

typedef QList<PropertyNode> PropertyNodes;

struct PreviewResult {
        bool gui = true;
        QImage preview;
        Control* control = nullptr;
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

class PreviewBackend : public QObject
{
        Q_OBJECT
        friend class PreviewBackendPrivate;

    public:
        explicit PreviewBackend(QObject *parent = 0);
        static PreviewBackend* instance();
        static void requestPreview(Control* control);
        static bool working();
        static QImage initialPreview(const QSizeF& size);

    signals:
        void previewReady(const QSharedPointer<PreviewResult> result);
        void workingChanged(bool value);

    private:
        static PreviewBackendPrivate* _d;
        static bool _working;
};

#endif // PREVIEWBACKEND_H
