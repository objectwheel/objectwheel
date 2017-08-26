#ifndef QMLPREVIEWER_H
#define QMLPREVIEWER_H

#include <QObject>
#include <QPointer>
#include <QWidget>
#include <QQmlError>
#include <QList>
#include <QMap>

#define NONGUI_CONTROL_SIZE (fit(40))

class QmlPreviewerPrivate;

struct PreviewResult {
        QPixmap preview;
        QPointF pos;
        QSizeF size;
        QString id;
        QString uid;
        QList<QString> events;
        QMap<QString, QVariant::Type> properties;
        bool clip;
        bool gui;
        int zValue;
};

class QmlPreviewer : public QObject
{
        Q_OBJECT

    public:
        explicit QmlPreviewer(QObject *parent = 0);
        ~QmlPreviewer();

    signals:
        void previewReady(const PreviewResult& result);
        void errorsOccurred(const QList<QQmlError>& errors, const PreviewResult& result);

    public slots:
        void requestReview(const QString& url, const QSizeF& size = QSizeF());

    private:
        QmlPreviewerPrivate* _d;
};

#endif // QMLPREVIEWER_H
