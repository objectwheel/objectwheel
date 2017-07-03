#include <qmlpreviewer.h>
#include <fit.h>

#include <QApplication>
#include <QQuickWindow>
#include <QSharedPointer>
#include <QQuickItemGrabResult>
#include <QQuickItem>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QImage>
#include <QtMath>
#include <QTimer>

using namespace Fit;

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
    if (!url.isValid()) {
        QQmlError error;
        error.setDescription("Invalid url");
        emit errorsOccurred(QList<QQmlError>() << error);
        return;
    }

    QObject* qmlObject;
    PreviewResult result;
    QSharedPointer<QQmlEngine> qmlEngine(new QQmlEngine);
    QSharedPointer<QQmlComponent> qmlComponent(new QQmlComponent(qmlEngine.data()));
    QSharedPointer<QQuickWindow> window;

    qmlComponent->loadUrl(url);
    qmlObject = qmlComponent->create();

    if (!qmlComponent->errors().isEmpty()) {
        emit errorsOccurred(qmlComponent->errors());
        return;
    }

    result.initial = !size.isValid();
    result.id = qmlContext(qmlObject)->nameForObject(qmlObject);
    window = QSharedPointer<QQuickWindow>(_d->handleWindowsIfAny(qmlObject));

    if (window == nullptr) {
        auto item = static_cast<QQuickItem*>(qmlObject);
        window = QSharedPointer<QQuickWindow>(new QQuickWindow);
        item->setParentItem(window->contentItem());

        if (size.isValid())
            item->setSize(size);
        else
            item->setSize(QSizeF(fit(item->width()), fit(item->height())));

        result.size = QSizeF(item->width(), item->height());

        window->resize(qCeil(item->width()), qCeil(item->height()));
        window->setClearBeforeRendering(true);
        window->setColor(QColor(Qt::transparent));
    } else {
        if (size.isValid())
            window->resize(QSize(qCeil(size.width()), qCeil(size.height())));
        else
            window->resize(QSize(qCeil(fit(window->width())), qCeil(fit(window->height()))));

        result.size = window->size();
    }

    window->setFlags(Qt::FramelessWindowHint);
    window->setOpacity(0);
    window->hide();

    QTimer::singleShot(100, [=] () mutable {
        Q_UNUSED(qmlEngine)
        Q_UNUSED(qmlComponent)

        QPixmap preview = QPixmap::fromImage(window->grabWindow());
        preview.setDevicePixelRatio(qApp->devicePixelRatio());
        _d->scratchPixmapIfEmpty(preview);

        result.preview = preview;

        emit previewReady(result);
    });
}

