#include <qmlpreviewer.h>

#include <QApplication>
#include <QQuickWidget>
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
        QQuickWindow* processWindows(QObject* object);

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
    brush.setColor(Qt::lightGray);
    brush.setStyle(Qt::DiagCrossPattern);
    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);
    painter.drawRect(pixmap.rect());
}

QQuickWindow* QmlPreviewerPrivate::processWindows(QObject* object)
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

QPointer<QWidget> QmlPreviewer::_puppetWidget;

QmlPreviewer::QmlPreviewer(QObject *parent)
    : QObject(parent)
    , _d(new QmlPreviewerPrivate(this))
{
}

QWidget* QmlPreviewer::puppetWidget()
{
    return _puppetWidget;
}

void QmlPreviewer::setPuppetWidget(QWidget* puppetWidget)
{
    _puppetWidget = puppetWidget;
}

void QmlPreviewer::requestReview(const QUrl& url, const QSizeF& size)
{
    if (!_puppetWidget || !url.isValid())
        return;

    QObject* qmlObject;
    QSharedPointer<QQmlEngine> qmlEngine(new QQmlEngine);
    QSharedPointer<QQmlComponent> qmlComponent(new QQmlComponent(qmlEngine.data()));
    QQuickWidget* quickWidget(new QQuickWidget(qmlEngine.data(), _puppetWidget));
    QSharedPointer<QQuickWindow> window;

    quickWidget->hide();
    qmlComponent->loadUrl(url);
    qmlObject = qmlComponent->create();

    if (!qmlComponent->errors().isEmpty())
        return;

    window = QSharedPointer<QQuickWindow>(_d->processWindows(qmlObject));

    if (window) {
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
            emit previewReady(preview, size);
        });
    } else {
        QQuickItem* item;
        QSharedPointer<QQuickItemGrabResult> grabResult;

        quickWidget->setSource(url);
        quickWidget->show();
        quickWidget->lower();

        if (!quickWidget->errors().isEmpty()) {
            quickWidget->deleteLater();
            return;
        }

        item = quickWidget->rootObject();
        item->setVisible(false);

        QSizeF gs;
        if (size.isValid()) {
            item->setSize(size);
            gs = size * qApp->devicePixelRatio();
        } else {
            gs = QSizeF(item->width(), item->height()) * qApp->devicePixelRatio();
        }
        gs = QSize(qCeil(gs.width()), qCeil(gs.height()));
        grabResult = item->grabToImage(gs.toSize());

        QObject::connect(grabResult.data(), &QQuickItemGrabResult::ready, [=] {
            QPixmap preview = QPixmap::fromImage(grabResult->image());
            preview.setDevicePixelRatio(qApp->devicePixelRatio());
            _d->scratchPixmapIfEmpty(preview);
            emit previewReady(preview, size);
        });
    }
}

