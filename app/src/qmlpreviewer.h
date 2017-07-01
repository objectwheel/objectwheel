#ifndef QMLPREVIEWER_H
#define QMLPREVIEWER_H

#include <QObject>
#include <QPointer>
#include <QWidget>

class QmlPreviewerPrivate;

class QmlPreviewer : public QObject
{
        Q_OBJECT

    public:
        explicit QmlPreviewer(QObject *parent = 0);

    signals:
        void previewReady(const QPixmap& preview, const QSizeF& size, const bool valid) const;

    public slots:
        void requestReview(const QUrl& url, const QSizeF& size = QSizeF());

    private:
        QmlPreviewerPrivate* _d;
};

#endif // QMLPREVIEWER_H
