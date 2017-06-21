#include <qmlpreviewer.h>

#include <QApplication>
#include <QQuickWidget>
#include <QQuickWindow>
#include <QSharedPointer>
#include <QQuickItemGrabResult>
#include <QQuickItem>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QImage>
#include <QtMath>

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
    if (!_puppetWidget) return;

    QQuickItem* item;
    QSharedPointer<QQuickItemGrabResult> grabResult;
    QQuickWidget* quickWidget(new QQuickWidget(_puppetWidget));

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
        quickWidget->deleteLater();
        emit previewReady(preview);
    });


//    QSizeF grabSize;
//    QObject* qmlObject;
//    QQuickItem* quickItem;
//    QQuickWindow* rootWindow;
//    QSharedPointer<QQuickItemGrabResult> grabResult;
//    QQuickWidget* quickWidget = new QQuickWidget(parent->puppetWidget());
//    QQmlComponent qmlComponent(quickWidget->engine());

//    quickWidget->show();
//    quickWidget->lower();
//    qmlComponent.loadUrl(parent->url());

//    if (!qmlComponent.errors().isEmpty()) {
//        quickWidget->deleteLater();
//        return;
//    }

//    qmlObject = qmlComponent.create();
//    rootWindow = processWindows(qmlObject);

//    if (rootWindow) {
//        //        QWidget* container = QWidget::createWindowContainer(rootWindow, quickWidget);

//        //        if (parent->size().width() == 0 && parent->size().height() == 0)
//        parent->resize(rootWindow->size());
//        //        else
//        //            rootWindow->resize(parent->size().toSize());

//        QPixmap pixmap = QPixmap::fromImage(rootWindow->grabWindow());
//        pixmap.save("/users/omergoktas/desktop/ee.png");
//        qDebug() << pixmap;
//        pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
//        itemPixmap = pixmap;
//        //        scratchPixmapIfEmpty(itemPixmap);
//        //        quickWidget->deleteLater();
//        //        rootWindow->deleteLater();
//        //        container->deleteLater();
//        parent->update();

//    } else {
//        quickItem = qobject_cast<QQuickItem*>(qmlObject);

//        if (!quickItem)
//            return;

//        quickItem->setParentItem(quickWidget->quickWindow()->contentItem());
//        quickItem->setVisible(false);

//        if (parent->size().width() == 0 && parent->size().height() == 0)
//            parent->resize(quickItem->width(), quickItem->height());
//        else
//            quickItem->setSize(parent->size());

//        grabSize = parent->size() * qApp->devicePixelRatio();
//        grabSize = QSize(qCeil(grabSize.width()), qCeil(grabSize.height()));
//        grabResult = quickItem->grabToImage(grabSize.toSize());

//        QObject::connect(grabResult.data(), &QQuickItemGrabResult::ready, [=] {
//            QPixmap pixmap = QPixmap::fromImage(grabResult->image());
//            pixmap.save("/users/omergoktas/desktop/ee.png");
//            pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
//            itemPixmap = pixmap;
//            scratchPixmapIfEmpty(itemPixmap);
//            quickItem->deleteLater();
//            parent->update();
//        });
//    }
}

