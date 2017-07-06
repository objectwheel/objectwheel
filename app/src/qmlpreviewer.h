#ifndef QMLPREVIEWER_H
#define QMLPREVIEWER_H

#include <QObject>
#include <QPointer>
#include <QWidget>
#include <QQmlError>
#include <QList>

class QmlPreviewerPrivate;

struct PreviewResult {
        QPixmap preview;
        QSizeF size;
        QString id;
        bool clip;
        bool initial;
};

class QmlPreviewer : public QObject
{
        Q_OBJECT

    public:
        explicit QmlPreviewer(QObject *parent = 0);

    signals:
        void previewReady(const PreviewResult& result);
        void errorsOccurred(const QList<QQmlError>& errors);

    public slots:
        void requestReview(const QUrl& url, const QSizeF& size = QSizeF());

    private:
        QmlPreviewerPrivate* _d;
};

#endif // QMLPREVIEWER_H
