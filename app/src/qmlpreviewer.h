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

        static QWidget* puppetWidget();
        static void setPuppetWidget(QWidget* puppetWidget);

    signals:
        void previewReady(const QPixmap& preview, const QSizeF& size) const;

    public slots:
        void requestReview(const QUrl& url, const QSizeF& size = QSizeF());

    private:
        QmlPreviewerPrivate* _d;
        static QPointer<QWidget> _puppetWidget;
};

#endif // QMLPREVIEWER_H
