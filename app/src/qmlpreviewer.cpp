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
#include <QScreen>

#define TASK_TIMEOUT 100
#define pS (QApplication::primaryScreen())

using namespace Fit;

class QmlPreviewerPrivate : public QObject
{
        Q_OBJECT

    public:
        QmlPreviewerPrivate(QmlPreviewer* parent);
        void scratchPixmapIfEmpty(QPixmap& pixmap) const;
        QQuickWindow* handleWindowsIfAny(QObject* object) const;
        PropertyNodes extractProperties(const QObject* object, const Control* control) const;
        QList<QString> extractEvents(const QObject* object) const;

    public slots:
        PreviewResult requestPreview(Control* control, const QString& url) const;
        void taskHandler();

    signals:
        void previewReady(const PreviewResult& result) const;
        void errorsOccurred(Control*, const QList<QQmlError>& errors, const PreviewResult& result) const;

    public:
        QmlPreviewer* parent;
        QList<QPointer<Control>> taskList;
        QTimer taskTimer;
};

QmlPreviewerPrivate::QmlPreviewerPrivate(QmlPreviewer* parent)
    : QObject(parent)
    , parent(parent)
{
    taskTimer.setInterval(TASK_TIMEOUT);
    connect(&taskTimer, SIGNAL(timeout()), SLOT(taskHandler()));
    connect(this, SIGNAL(errorsOccurred(Control*,QList<QQmlError>,PreviewResult)),
        parent, SIGNAL(errorsOccurred(Control*,QList<QQmlError>,PreviewResult)));
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

PropertyNodes QmlPreviewerPrivate::extractProperties(const QObject* object, const Control* control) const
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
                QString name = metaObject->property(i).name();
                auto val = metaObject->property(i).read(object);

                if (name == TAG_X)
                    propertyMap[name] = SaveManager::x(control->dir());
                else
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

PreviewResult QmlPreviewerPrivate::requestPreview(Control* control, const QString& url) const
{
    bool isWindow = false;
    PreviewResult result;

    if (url.isEmpty() || !SaveManager::isOwctrl(dname(dname(url)))) {
        QQmlError error;
        error.setDescription("Invalid url or control.");
        emit errorsOccurred(control, QList<QQmlError>() << error, result);
        return PreviewResult();
    }

    QObject* qmlObject;
    QSharedPointer<QQmlEngine> qmlEngine(new QQmlEngine);
    QSharedPointer<QQmlComponent> qmlComponent(new QQmlComponent(qmlEngine.data()));
    QSharedPointer<QQuickWindow> window;

    qmlEngine->setOutputWarningsToStandardError(false);
    qmlEngine->rootContext()->setContextProperty("dpi", Fit::ratio());

    auto qmlData = rdfile(url);
    ParserWorker parserWorker; //FIXME: What if qml file contains errors?
    //BUG: Possible bug if property 'visible' is a binding
    if (parserWorker.typeName(qmlData).contains("Window")) {
        parserWorker.setVariantProperty(qmlData, url, "visible", false);
        isWindow = true;
    }

    if (isWindow && !control->form()) {
        QQmlError error;
        error.setDescription("Only forms can be 'Window' qml type.");
        emit errorsOccurred(control, QList<QQmlError>() << error, result);
        return PreviewResult();
    }

    qmlComponent->setData(qmlData, QUrl::fromLocalFile(url));
    qmlObject = qmlComponent->create();

    if (!qmlComponent->errors().isEmpty()) {
        emit errorsOccurred(control, qmlComponent->errors(), result);
        return PreviewResult();
    }

    result.skin = SaveManager::skin(dname(dname(url)));
    result.properties = extractProperties(qmlObject, control);
    result.events = extractEvents(qmlObject);
    window = QSharedPointer<QQuickWindow>(handleWindowsIfAny(qmlObject));
    result.gui = (qmlObject->inherits("QQuickItem") || window);

    if (result.gui == false) {
        result.preview = QPixmap(dname(url) + separator() + "icon.png")
                         .scaled(NONGUI_CONTROL_SIZE * pS->devicePixelRatio(),
                                 NONGUI_CONTROL_SIZE * pS->devicePixelRatio());
        return result;
    }

    if (window == nullptr) {
        auto item = static_cast<QQuickItem*>(qmlObject);
        window = QSharedPointer<QQuickWindow>(new QQuickWindow);
        item->setParentItem(window->contentItem());
        item->setSize(QSizeF(fit(item->width()), fit(item->height())));
        item->setPosition({0,0});
        window->resize(qCeil(item->width()), qCeil(item->height()));
        window->setClearBeforeRendering(true);
        window->setColor(QColor(Qt::transparent));
    } else {
        window->resize(QSize(qCeil(fit(window->width())), qCeil(fit(window->height()))));
    }

    window->setFlags(Qt::Window | Qt::FramelessWindowHint);
    window->setOpacity(0);
    window->hide();
    window->create();

    Delayer::delay(100);

    QPixmap preview = QPixmap::fromImage(window->grabWindow());
    preview.setDevicePixelRatio(pS->devicePixelRatio());
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

    auto control = taskList.takeFirst();

    if (!control) {
        parent->_working = false;
        emit parent->workingChanged(parent->_working);
        return;
    }

    auto dir = control->dir();
    auto masterPaths = SaveManager::masterPaths(dir);

    if (SaveManager::suid(dir).isEmpty() || masterPaths.isEmpty()) {
        auto res = requestPreview(control, dir + separator() + DIR_THIS +
                                  separator() + "main.qml");
        if (!res.isNull())
            emit parent->previewReady(control, res);
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
                results[childPath] = requestPreview(control, childPath + separator() + DIR_THIS + separator() + "main.qml");
                if (results[childPath].isNull()) {
                    parent->_working = false;
                    emit parent->workingChanged(parent->_working);
                    return;
                }
            }
        }

        if (masterPaths.last() == path) {
            masterResults[path] = requestPreview(control, path + separator() + DIR_THIS + separator() + "main.qml");
            finalResult = &masterResults[path];
            if (masterResults[path].isNull()) {
                parent->_working = false;
                emit parent->workingChanged(parent->_working);
                return;
            }
        } else {
            masterResults[path] = requestPreview(control, path + separator() + DIR_THIS + separator() + "main.qml");
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

            auto& r = results[result];
            const QPointF p = { r.property("x").toReal(), r.property("y").toReal() };
            const QSizeF s = { r.property("width").toReal(), r.property("height").toReal() };

            QPainter px(parentPixmap);
            px.drawPixmap(QRectF(p, s), r.preview, QRectF(QPointF(0, 0),
             s * pS->devicePixelRatio()));
            px.end();

            parentPixmap = &r.preview;
        }
    }

    emit parent->previewReady(control, *finalResult);

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

void QmlPreviewer::requestPreview(Control* control)
{
    _d->taskList.append(control);
    _d->taskTimer.start();
}

bool QmlPreviewer::working()
{
    return _working;
}

#include "qmlpreviewer.moc"
