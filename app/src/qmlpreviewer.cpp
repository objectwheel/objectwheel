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
#include <QPainter>
#include <QTimer>

#define TASK_TIMEOUT 100

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
        void taskHandler();

    signals:
        void previewReady(const PreviewResult& result) const;
        void errorsOccurred(const QList<QQmlError>& errors, const PreviewResult& result) const;

    public:
        QmlPreviewer* parent;
        QList<QSizeF> taskList;
        QTimer taskTimer;
        static bool working;
};

bool QmlPreviewerPrivate::working = false;

QmlPreviewerPrivate::QmlPreviewerPrivate(QmlPreviewer* parent)
    : QObject(parent)
    , parent(parent)
{
    taskTimer.setInterval(TASK_TIMEOUT);
    connect(&taskTimer, SIGNAL(timeout()), SLOT(taskHandler()));
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

void QmlPreviewerPrivate::taskHandler()
{
    if (working)
        return;

    if (taskList.isEmpty()) {
        taskTimer.stop();
        return;
    }
    working = true;

    auto size = taskList.takeFirst();
    auto dir = parent->_watched->dir();

    if (SaveManager::suid(dir).isEmpty()) {
        auto res = requestPreview(dir + separator() + DIR_THIS +
                                      separator() + "main.qml", size);
        if (!res.isNull())
            emit parent->previewReady(res);
        working = false;
        return;
    }

    PreviewResult* finalResult = nullptr;
    auto masterPaths = SaveManager::masterPaths(dir);
    QMap<QString, PreviewResult> masterResults;

    for (auto path : masterPaths) {
        QMap<QString, PreviewResult> results;
        auto childrenPaths = SaveManager::childrenPaths(path);
        for (auto childPath : childrenPaths) {
            int index = masterPaths.indexOf(childPath);
            if (index >= 0) {
                results[childPath] = masterResults[childPath];
            } else {
                results[childPath] = requestPreview(childPath + separator() + DIR_THIS + separator() + "main.qml", QSize());
                if (results[childPath].isNull()) {
                    working = false;
                    return;
                }
            }
        }

        if (masterPaths.last() == path) {
            masterResults[path] = requestPreview(path + separator() + DIR_THIS + separator() + "main.qml", size);
            finalResult = &masterResults[path];
            if (masterResults[path].isNull()) {
                working = false;
                return;
            }
        } else {
            masterResults[path] = requestPreview(path + separator() + DIR_THIS + separator() + "main.qml", QSize());
            if (masterResults[path].isNull()) {
                working = false;
                return;
            }
        }

        QPixmap* parentPixmap;
        for (auto result : results.keys()) {
            if (dname(dname(result)) == path)
                parentPixmap = &masterResults[path].preview;

            QPainter p(parentPixmap);
            p.drawPixmap(QRectF(results[result].pos, results[result].size),
                         results[result].preview, QRectF(QPointF(0, 0),
                         results[result].size * qApp->devicePixelRatio()));
            p.end();

            parentPixmap = &results[result].preview;
        }
    }

    if (finalResult) //FIXME
        emit parent->previewReady(*finalResult);
    working = false;
}

QmlPreviewer::QmlPreviewer(Control* watched, QObject *parent)
    : QObject(parent)
    , _d(new QmlPreviewerPrivate(this))
    , _watched(watched)
{
}

void QmlPreviewer::requestPreview(const QSizeF& size)
{
    _d->taskList.append(size);
    _d->taskTimer.start();
}

#include "qmlpreviewer.moc"
