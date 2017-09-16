#ifndef QMLPREVIEWER_H
#define QMLPREVIEWER_H

#include <QObject>
#include <QPointer>
#include <QWidget>
#include <QQmlError>
#include <QList>
#include <QMap>

#define NONGUI_CONTROL_SIZE (fit(40))

class Control;
class QmlPreviewerPrivate;

struct PreviewResult {
        QPixmap preview;
        QPointF pos;
        QSizeF size;
        QList<QString> events;
        QMap<QString, QVariant> properties;
        bool clip;
        bool gui;
        int zValue;
        bool isNull() { return preview.isNull(); }
};

class QmlPreviewer : public QObject
{
        Q_OBJECT
        friend class QmlPreviewerPrivate;

    public:
        explicit QmlPreviewer(QObject *parent = 0);
        static QmlPreviewer* instance();
        static void requestPreview(Control* control, const QSizeF& size = QSizeF());
        static bool working();

    signals:
        void previewReady(Control* control, const PreviewResult& result);
        void errorsOccurred(Control* control, const QList<QQmlError>& errors, const PreviewResult& result);
        void workingChanged(bool value);

    private:
        static QmlPreviewerPrivate* _d;
        static bool _working;
};

#endif // QMLPREVIEWER_H
