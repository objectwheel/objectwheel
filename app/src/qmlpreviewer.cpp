#include <qmlpreviewer.h>
#include <fit.h>
#include <formscene.h>
#include <filemanager.h>
#include <savemanager.h>
#include <control.h>
#include <delayer.h>
#include <parserworker.h>

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
#include <QPair>

#define TASK_TIMEOUT 100

using namespace Fit;

class QmlPreviewerPrivate : public QObject
{
        Q_OBJECT

    public:
        QmlPreviewerPrivate(QmlPreviewer* parent);
        void scratchPixmapIfEmpty(QPixmap& pixmap) const;
        QQuickWindow* handleWindowsIfAny(QObject* object) const;
        PropertyNodes extractProperties(const QObject* object) const;
        QList<QString> extractEvents(const QObject* object) const;

    public slots:
        PreviewResult requestPreview(const QString& url, const QSizeF& size) const;
        void taskHandler();

    signals:
        void previewReady(const PreviewResult& result) const;
        void errorsOccurred(const QList<QQmlError>& errors, const PreviewResult& result) const;

    public:
        QmlPreviewer* parent;
        QList<QPair<QSizeF, Control*>> taskList;
        QTimer taskTimer;
};

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

PropertyNodes QmlPreviewerPrivate::extractProperties(const QObject* object) const
{
    PropertyNodes propertyNodes;
    auto metaObject = object->metaObject();

    while (metaObject) {
        QString className = metaObject->className();
        if (className == "QObject" ||
            (metaObject->propertyOffset() -
             metaObject->propertyCount()) == 0) {
            metaObject = metaObject->superClass();
            continue;
        }

        PropertyNode propertyNode;
        PropertyMap propertyMap;

        for (int i = metaObject->propertyOffset();
             i < metaObject->propertyCount(); i++) {
            if (metaObject->property(i).isWritable()) {
                auto name = metaObject->property(i).name();
                auto val = metaObject->property(i).read(object);
                propertyMap[name] = val;
            }
        }

        className = className.split("_QMLTYPE").at(0);
        className.remove("QQuick");

        propertyNode.cleanClassName = className;
        propertyNode.propertyMap = propertyMap;
        propertyNode.metaObject = metaObject;

        propertyNodes << propertyNode;

        metaObject = metaObject->superClass();
    }

    // ** Attached Properties **
    //    #include <QtQml>
    //    #include <QtQml/private/qqmldata_p.h>
    //    QQmlData *data = QQmlData::get(object);
    //    if (data && data->hasExtendedData()) {
    //        auto attachedProperties = data->attachedProperties();
    //        for (auto key : attachedProperties->keys()) {
    //            auto metaObject = attachedProperties->value(key)->metaObject();
    //            qDebug() << metaObject->className();
    //            for (int i = metaObject->propertyCount(); --i;) {
    //                    qDebug() << metaObject->property(i).name();
    //            }
    //        }
    //    }

    return propertyNodes;
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

    if (url.isEmpty() || !SaveManager::isOwctrl(dname(dname(url)))) {
        QQmlError error;
        error.setDescription("Invalid url or control.");
        emit errorsOccurred(QList<QQmlError>() << error, result);
        return PreviewResult();
    }

    QObject* qmlObject;
    QByteArray qmlData;
    QSharedPointer<QQmlEngine> qmlEngine(new QQmlEngine);
    QSharedPointer<QQmlComponent> qmlComponent(new QQmlComponent(qmlEngine.data()));
    QSharedPointer<QQuickWindow> window;

    qmlEngine->rootContext()->setContextProperty("dpi", Fit::ratio());

    qmlData = rdfile(url);
    ParserWorker parserWorker;
    if (parserWorker.typeName(qmlData).contains("Window"))
        parserWorker.setVariantProperty(qmlData, url, "visible", false);

    qmlComponent->setData(qmlData, QUrl::fromLocalFile(url));
    qmlObject = qmlComponent->create();

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
                         .scaled(NONGUI_CONTROL_SIZE * qApp->devicePixelRatio(),
                                 NONGUI_CONTROL_SIZE * qApp->devicePixelRatio());
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

    window->setFlags(Qt::Window | Qt::FramelessWindowHint);
    window->setOpacity(0);
    window->hide();
    window->create();

    Delayer::delay(100);

    QPixmap preview = QPixmap::fromImage(window->grabWindow());
    preview.setDevicePixelRatio(qApp->devicePixelRatio());
    scratchPixmapIfEmpty(preview);
    result.preview = preview;

    return result;
}

void QmlPreviewerPrivate::taskHandler()
{
    if (parent->_working)
        return;

    if (taskList.isEmpty()) {
        taskTimer.stop();
        return;
    }
    parent->_working = true;
    emit parent->workingChanged(parent->_working);

    auto task = taskList.takeFirst();
    auto dir = task.second->dir();
    auto masterPaths = SaveManager::masterPaths(dir);

    if (SaveManager::suid(dir).isEmpty() || masterPaths.isEmpty()) {
        auto res = requestPreview(dir + separator() + DIR_THIS +
                                  separator() + "main.qml", task.first);
        if (!res.isNull())
            emit parent->previewReady(task.second, res);
        parent->_working = false;
        emit parent->workingChanged(parent->_working);
        return;
    }

    PreviewResult* finalResult = nullptr;
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
                    parent->_working = false;
                    emit parent->workingChanged(parent->_working);
                    return;
                }
            }
        }

        if (masterPaths.last() == path) {
            masterResults[path] = requestPreview(path + separator() + DIR_THIS + separator() + "main.qml", task.first);
            finalResult = &masterResults[path];
            if (masterResults[path].isNull()) {
                parent->_working = false;
                emit parent->workingChanged(parent->_working);
                return;
            }
        } else {
            masterResults[path] = requestPreview(path + separator() + DIR_THIS + separator() + "main.qml", QSize());
            if (masterResults[path].isNull()) {
                parent->_working = false;
                emit parent->workingChanged(parent->_working);
                return;
            }
        }

        QPixmap* parentPixmap;
        for (auto result : results.keys()) { //FIXME
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

    emit parent->previewReady(task.second, *finalResult);

    parent->_working = false;
    emit parent->workingChanged(parent->_working);
}

bool QmlPreviewer::_working = false;
QmlPreviewerPrivate* QmlPreviewer::_d = nullptr;

QmlPreviewer::QmlPreviewer(QObject *parent) : QObject(parent)
{
    if (_d)
        return;

    _d = new QmlPreviewerPrivate(this);
}

QmlPreviewer* QmlPreviewer::instance()
{
    return _d ? _d->parent : nullptr;
}

void QmlPreviewer::requestPreview(Control* control, const QSizeF& size)
{
    _d->taskList.append(QPair<QSizeF, Control*>(size, control));
    _d->taskTimer.start();
}

bool QmlPreviewer::working()
{
    return _working;
}

#include "qmlpreviewer.moc"
