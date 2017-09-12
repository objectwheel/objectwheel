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
        QString id;
        QString uid;
        QList<QString> events;
        QMap<QString, QVariant> properties;
        bool clip;
        bool gui;
        int zValue;
};

class QmlPreviewer : public QObject
{
        Q_OBJECT

    public:
        explicit QmlPreviewer(Control* watched, QObject *parent = 0);

    signals:
        void previewReady(const PreviewResult& result);
        void errorsOccurred(const QList<QQmlError>& errors, const PreviewResult& result);

    public slots:
        void requestPreview(const QSizeF& size = QSizeF());

    private:
        QmlPreviewerPrivate* _d;
        Control* _watched;
};

#endif // QMLPREVIEWER_H
