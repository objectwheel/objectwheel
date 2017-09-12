#include <qmlpreviewer.h>
#include <fit.h>
#include <formscene.h>
#include <filemanager.h>
#include <savemanager.h>
#include <control.h>
#include <delayer.h>

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

class QmlPreviewerPrivate : public QObject
{
        Q_OBJECT

    public:
        QmlPreviewerPrivate(QmlPreviewer* parent);
        void scratchPixmapIfEmpty(QPixmap& pixmap) const;
        QQuickWindow* handleWindowsIfAny(QObject* object) const;
        QMap<QString, QVariant> extractProperties(const QObject* object) const;
        QList<QString> extractEvents(const QObject* object) const;

    public slots:
        PreviewResult requestPreview(const QString& url, const QSizeF& size) const;

    signals:
        void previewReady(const PreviewResult& result) const;
        void errorsOccurred(const QList<QQmlError>& errors, const PreviewResult& result) const;

    public:
        QmlPreviewer* parent;
        bool working;
};

QmlPreviewerPrivate::QmlPreviewerPrivate(QmlPreviewer* parent)
    : QObject(parent)
    , parent(parent)
    , working(false)
{
}

void QmlPreviewerPrivate::scratchPixmapIfEmpty(QPixmap& pixmap) const
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

QQuickWindow* QmlPreviewerPrivate::handleWindowsIfAny(QObject* object) const
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

QMap<QString, QVariant> QmlPreviewerPrivate::extractProperties(const QObject* object) const
{
    QMap<QString, QVariant> properties;
    auto metaObject = object->metaObject();
    for (int i = 0; i < metaObject->propertyCount(); i++) {
        if (metaObject->property(i).isWritable() &&
            !QString(metaObject->property(i).name()).startsWith("__"))
            properties[(metaObject->property(i).name())] = metaObject->property(i).read(object);
    }
    return properties;
}

QList<QString> QmlPreviewerPrivate::extractEvents(const QObject* object) const
{
    QList<QString> events;
    auto metaObject = object->metaObject();
    for (int i = metaObject->methodCount(); --i;) {
        if (metaObject->method(i).methodType() == QMetaMethod::Signal
            && !metaObject->method(i).name().startsWith("__"))
            events << metaObject->method(i).name();
    }
    return events;
}

PreviewResult QmlPreviewerPrivate::requestPreview(const QString& url, const QSizeF& size) const
{
    PreviewResult result;
    result.id = "none";

    if (url.isEmpty() || !SaveManager::isOwctrl(dname(dname(url)))) {
        QQmlError error;
        error.setDescription("Invalid url or control.");
        emit errorsOccurred(QList<QQmlError>() << error, result);
        return PreviewResult();
    }

    QObject* qmlObject;
    QSharedPointer<QQmlEngine> qmlEngine(new QQmlEngine);
    QSharedPointer<QQmlComponent> qmlComponent(new QQmlComponent(qmlEngine.data()));
    QSharedPointer<QQuickWindow> window;

    qmlEngine->rootContext()->setContextProperty("dpi", Fit::ratio());
    qmlComponent->loadUrl(url);
    qmlObject = qmlComponent->create();
    result.id = SaveManager::id(dname(dname(url)));
    result.properties["id"] = result.id;

    if (!qmlComponent->errors().isEmpty()) {
        emit errorsOccurred(qmlComponent->errors(), result);
        return PreviewResult();
    }

    result.properties = extractProperties(qmlObject);
    result.events = extractEvents(qmlObject);
    window = QSharedPointer<QQuickWindow>(handleWindowsIfAny(qmlObject));
    result.gui = (qmlObject->inherits("QQuickItem") || window);

    if (result.gui == false) {
        result.preview = QPixmap(dname(url) + separator() + "icon.png")
                         .scaled(NONGUI_CONTROL_SIZE * qApp->devicePixelRatio(), NONGUI_CONTROL_SIZE * qApp->devicePixelRatio());
        emit previewReady(result);
        return PreviewResult();
    }

    if (window == nullptr) {
        auto item = static_cast<QQuickItem*>(qmlObject);
        window = QSharedPointer<QQuickWindow>(new QQuickWindow);
        item->setParentItem(window->contentItem());

        if (size.isValid())
            item->setSize(size);
        else
            item->setSize(QSizeF(fit(item->width()), fit(item->height())));

        result.pos = item->position();
        result.size = QSizeF(item->width(), item->height());
        result.clip = item->clip();
        result.zValue = item->z();

        item->setPosition({0,0});
        window->resize(qCeil(item->width()), qCeil(item->height()));
        window->setClearBeforeRendering(true);
        window->setColor(QColor(Qt::transparent));
    } else {
        if (size.isValid())
            window->resize(QSize(qCeil(size.width()), qCeil(size.height())));
        else
            window->resize(QSize(qCeil(fit(window->width())), qCeil(fit(window->height()))));

        result.size = window->size();
        result.zValue = 0;
        result.clip = true;
    }

    window->setFlags(Qt::FramelessWindowHint);
    window->setOpacity(0);
    window->hide();

    Delayer::delay(100);

    QPixmap preview = QPixmap::fromImage(window->grabWindow());
    preview.setDevicePixelRatio(qApp->devicePixelRatio());
    scratchPixmapIfEmpty(preview);
    result.preview = preview;
    return result;
}

QmlPreviewer::QmlPreviewer(Control* watched, QObject *parent)
    : QObject(parent)
    , _d(new QmlPreviewerPrivate(this))
    , _watched(watched)
{
}

#include <QPainter>

void QmlPreviewer::requestPreview(const QSizeF& size)
{
    if (_d->working)
        return;
    _d->working = true;

    PreviewResult finalResult;
    auto dir = _watched->dir();
    auto masterPaths = SaveManager::masterPaths(dir);
    QMap<QString, PreviewResult> masterResults;

    for (auto path : masterPaths) {
        QMap<QString, PreviewResult> results;
        auto childrenPaths = SaveManager::childrenPaths(path);
        for (auto childPath : childrenPaths) {
            int index = masterPaths.indexOf(childPath);
            if (index >= 0)
                results[masterPaths[index]] = masterResults[masterPaths[index]];
            else
                results[childPath] = _d->requestPreview(childPath + separator() + DIR_THIS + separator() + "main.qml", QSize());
        }

        if (masterPaths.last() == path) {
            masterResults[path] = _d->requestPreview(path + separator() + DIR_THIS + separator() + "main.qml", size);
            finalResult = masterResults[path];
        } else {
            masterResults[path] = _d->requestPreview(path + separator() + DIR_THIS + separator() + "main.qml", QSize());
        }

        QPixmap* pixmap;
        for (auto result : results.keys()) {
            if (dname(dname(result)) == path)
                pixmap = &masterResults[path].preview;

            QPainter p(pixmap);
            p.drawPixmap(results[result].pos, results[result].preview);
            p.end();

            pixmap = &results[result].preview;
        }
    }

    _d->working = false;
    emit previewReady(finalResult);
}

#include "qmlpreviewer.moc"
