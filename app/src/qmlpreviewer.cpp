#include <qmlpreviewer.h>

#include <QApplication>
#include <QQuickWindow>
#include <QSharedPointer>
#include <QQuickItemGrabResult>
#include <QQuickItem>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QImage>
#include <QtMath>
#include <QTimer>

class QmlPreviewerPrivate
{
    public:
        QmlPreviewerPrivate(QmlPreviewer* parent);
        void scratchPixmapIfEmpty(QPixmap& pixmap);
        QQuickWindow* handleWindowsIfAny(QObject* object);

    public:
        QmlPreviewer* parent;
};

QmlPreviewerPrivate::QmlPreviewerPrivate(QmlPreviewer* parent)
    : parent(parent)
{
}

void QmlPreviewerPrivate::scratchPixmapIfEmpty(QPixmap& pixmap)
{
    // Check 10 pixels atleast that has alpha > 250
    int totalAlpha = 0;
    QImage img = pixmap.toImage();
    for (int i = 0; i < img.width(); i++) {
        for (int j = 0; j < img.height(); j++) {
            totalAlpha += qAlpha(img.pixel(i, j));
            if (totalAlpha > (250 * 10)) {
                return;
            }
        }
    }

    QBrush brush;
    QPainter painter(&pixmap);
    brush.setColor(Qt::gray);
    brush.setStyle(Qt::DiagCrossPattern);
    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);
    painter.drawRect(pixmap.rect());
}

QQuickWindow* QmlPreviewerPrivate::handleWindowsIfAny(QObject* object)
{
    QQuickWindow* ret = nullptr;

    if (object->isWindowType())
        ret = qobject_cast<QQuickWindow*>(object);

    for (auto object : object->findChildren<QObject*>()) {
        if (object->isWindowType())
            object->deleteLater();
    }
    return ret;
}

QmlPreviewer::QmlPreviewer(QObject *parent)
    : QObject(parent)
    , _d(new QmlPreviewerPrivate(this))
{
}

void QmlPreviewer::requestReview(const QUrl& url, const QSizeF& size)
{
    if (!url.isValid())
        return;

    QObject* qmlObject;
    QSharedPointer<QQmlEngine> qmlEngine(new QQmlEngine);
    QSharedPointer<QQmlComponent> qmlComponent(new QQmlComponent(qmlEngine.data()));
    QSharedPointer<QQuickWindow> window;

    qmlComponent->loadUrl(url);
    qmlObject = qmlComponent->create();

    if (!qmlComponent->errors().isEmpty())
        return;

    window = QSharedPointer<QQuickWindow>(_d->handleWindowsIfAny(qmlObject));

    if (!window) {
        auto item = static_cast<QQuickItem*>(qmlObject);
        window = QSharedPointer<QQuickWindow>(new QQuickWindow);
        item->setParentItem(window->contentItem());
        if (!size.isValid())
            window->resize(item->width(), item->height());
        else
            item->setSize(size);

        window->setClearBeforeRendering(true);
        window->setColor(QColor(Qt::transparent));
    }

    window->setFlags(Qt::FramelessWindowHint);
    window->setOpacity(0);
    window->hide();

    QSizeF gs;
    if (size.isValid()) {
        window->show();
        window->resize(size.toSize());
        window->hide();
        gs = size * qApp->devicePixelRatio();
    } else {
        gs = QSizeF(window->width(), window->height()) * qApp->devicePixelRatio();
    }
    gs = QSize(qCeil(gs.width()), qCeil(gs.height()));

    QTimer::singleShot(100, [=] {
        QPixmap preview = QPixmap::fromImage(window->grabWindow());
        preview.setDevicePixelRatio(qApp->devicePixelRatio());
        _d->scratchPixmapIfEmpty(preview);
        window->deleteLater();
        qmlComponent->deleteLater();
        qmlEngine->deleteLater();
        emit previewReady(preview, size);
    });
}

