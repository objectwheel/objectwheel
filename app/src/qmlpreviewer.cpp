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
        QmlPreviewerPrivate(QmlPreviewer*);
        PreviewResult preview(Control*, const QString&) const;

    public:
        QmlPreviewer* parent;
        QTimer taskTimer;
        QList<QPointer<Control>> taskList;

    private slots:
        void processTasks();

    private:
        void dash(QPixmap&) const;
        bool containsWindow(QObject*) const;
        QPixmap prepreview(const Control*) const;
        QList<QString> events(const QObject*) const;
        PropertyNodes properties(const QObject*) const;

    private:
        QPixmap errorPixmap;
};

QmlPreviewerPrivate::QmlPreviewerPrivate(QmlPreviewer* parent)
    : QObject(parent)
    , parent(parent)
    , errorPixmap(QPixmap(":/resources/images/error.png").
      scaled(QSize(fit(14), fit(14)) * pS->devicePixelRatio(),
      Qt::IgnoreAspectRatio, Qt::SmoothTransformation))
{
    taskTimer.setInterval(TASK_TIMEOUT);
    connect(&taskTimer, SIGNAL(timeout()), SLOT(processTasks()));
}

void QmlPreviewerPrivate::dash(QPixmap& pixmap) const
{
    // Check 15 pixels atleast that has alpha > 250
    int totalAlpha = 0;
    const auto& img = pixmap.toImage();
    for (int i = 0; i < img.width(); i++) {
        for (int j = 0; j < img.height(); j++) {
            totalAlpha += qAlpha(img.pixel(i, j));
            if (totalAlpha > (250 * 15)) {
                return;
            }
        }
    }

    QBrush brush;
    brush.setColor(Qt::gray);
    brush.setStyle(Qt::Dense5Pattern);
    QPainter painter(&pixmap);
    painter.setBrush(brush);
    painter.setPen(Qt::NoPen);
    painter.drawRect(pixmap.rect());
}

bool QmlPreviewerPrivate::containsWindow(QObject* o) const
{
    for (auto _o : o->findChildren<QObject*>()) {
        if (_o->isWindowType())
            return true;
    }
    return false;
}

QPixmap QmlPreviewerPrivate::prepreview(const Control* control) const
{
    QPixmap p(qCeil(control->size().width()),
      qCeil(control->size().height()));
    QRectF r(0, 0, fit(14), fit(14));
    r.moveCenter(p.rect().center());
    QBrush brush;
    brush.setColor(Qt::gray);
    brush.setStyle(Qt::Dense6Pattern);
    QPainter painter(&p);
    painter.setBrush(brush);
    painter.setPen(Qt::NoPen);
    painter.drawRect(p.rect());
    painter.drawPixmap(r, errorPixmap, errorPixmap.rect());
    return p;
}

PropertyNodes QmlPreviewerPrivate::properties(const QObject* object) const
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

    /** Attached Properties **/
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

QList<QString> QmlPreviewerPrivate::events(const QObject* object) const
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

PreviewResult QmlPreviewerPrivate::preview(Control* control, const QString& url) const
{
    if (!SaveManager::isOwctrl(dname(dname(url)))) {
        qFatal("Fatal error: Control doesn't meet Owctrl™ requirements.");
        return PreviewResult();
    }

    QQmlEngine engine;
    PreviewResult result;
    QQmlComponent component(&engine);

    engine.setOutputWarningsToStandardError(false);
    engine.rootContext()->setContextProperty("dpi", Fit::ratio());

    // FIXME: What if qml file contains errors?
    // BUG: Possible bug if property 'visible' is a binding
    bool isWindow = false;
    auto qml = rdfile(url);
    ParserWorker pw;
    if (pw.typeName(qml).contains("Window")) {
        pw.setVariantProperty(qml, url, "visible", false);
        isWindow = true;
    }

    result.control = control;
    result.preview = prepreview(control);

    if (isWindow && !control->form()) {
        QQmlError error;
        error.setDescription("Only forms can be 'Window' qml type.");
        result.errors = (QList<QQmlError>() << error);
        return result;
    }

    component.setData(qml, QUrl::fromLocalFile(url));
    auto object = component.create();
    engine.setObjectOwnership(object, QQmlEngine::JavaScriptOwnership);

    if (component.isError()) {
        result.errors = component.errors();
        return result;
    }

    if (containsWindow(object)) {
        QQmlError error;
        error.setDescription("You can not define child objects as 'Window' qml type.");
        result.errors = (QList<QQmlError>() << error);
        return result;
    }

    QSharedPointer<QQuickWindow> window(qobject_cast<QQuickWindow*>(object));
    result.properties = properties(object);
    result.events = events(object);
    result.gui = (object->inherits("QQuickItem") || window);

    if (result.gui == false) {
        result.preview = QPixmap(dname(url) + separator() + "icon.png")
                         .scaled(NONGUI_CONTROL_SIZE * pS->devicePixelRatio(),
                                 NONGUI_CONTROL_SIZE * pS->devicePixelRatio());
    } else {
        const qreal w = control->size().width();
        const qreal h = control->size().height();

        if (window.isNull()) {
            auto item = qobject_cast<QQuickItem*>(object); Q_ASSERT(item);
            window = QSharedPointer<QQuickWindow>(new QQuickWindow);
            item->setParentItem(window->contentItem());
            item->setSize(QSizeF(fit(w), fit(h)));
            item->setPosition({0,0});
            window->resize(qCeil(item->width()), qCeil(item->height()));
            window->setClearBeforeRendering(true);
            window->setColor(Qt::transparent);
        } else {
            window->resize(QSize(qCeil(fit(w)), qCeil(fit(h))));
        }

        window->setFlags(Qt::Window | Qt::FramelessWindowHint);
        window->setOpacity(0);
        window->hide();
        window->create();

        Delayer::delay(1); //WARNING

        QPixmap p = QPixmap::fromImage(window->grabWindow());
        p.setDevicePixelRatio(pS->devicePixelRatio());
        dash(p);
        result.preview = p;
    }
    return result;
}

void QmlPreviewerPrivate::processTasks()
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

    if (control.isNull()) {
        parent->_working = false;
        emit parent->workingChanged(parent->_working);
        return;
    }

    auto dir = control->dir();
    auto masterPaths = SaveManager::masterPaths(dir);

    // If it's a form, tool or non-master child control, then we just need a background
    // preview pixmap. So, we don't need a nested-preview pixmap.
    if (SaveManager::suid(dir).isEmpty() || masterPaths.isEmpty()) {
        auto res = preview(control, dir + separator() + DIR_THIS +
                           separator() + "main.qml");
        emit parent->previewReady(res);
        parent->_working = false;
        emit parent->workingChanged(parent->_working);
        return;
    }

    // Only master-child items are handled here, others are handled above
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
                results[childPath] = preview(control, childPath +
                  separator() + DIR_THIS + separator() + "main.qml");
                if (results[childPath].hasError()) {
                    emit parent->previewReady(results[childPath]);
                    parent->_working = false;
                    emit parent->workingChanged(parent->_working);
                    return;
                }
            }
        }

        if (masterPaths.last() == path) {
            masterResults[path] = preview(control, path +
              separator() + DIR_THIS + separator() + "main.qml");
            finalResult = &masterResults[path];
            if (masterResults[path].hasError()) {
                emit parent->previewReady(masterResults[path]);
                parent->_working = false;
                emit parent->workingChanged(parent->_working);
                return;
            }
        } else {
            masterResults[path] = preview(control, path +
              separator() + DIR_THIS + separator() + "main.qml");
            if (masterResults[path].hasError()) {
                emit parent->previewReady(masterResults[path]);
                parent->_working = false;
                emit parent->workingChanged(parent->_working);
                return;
            }
        }

        QPixmap* parentPixmap;
        for (auto result : results.keys()) { //FIXME: Some child previews lost, wrong 'parentPixmap' logic, use maps here
            if (dname(dname(result)) == path)
                parentPixmap = &masterResults[path].preview;

            auto& r = results[result];
            const QPointF p = r.control->pos();
            const QSizeF s = r.control->size();

            QPainter px(parentPixmap);
            px.drawPixmap(QRectF(p, s), r.preview, QRectF(QPointF(0, 0),
              s * pS->devicePixelRatio()));
            px.end();

            parentPixmap = &r.preview;
        }
    }

    emit parent->previewReady(*finalResult);

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
    if (!_d->taskList.contains(control))
        _d->taskList.append(control);
    _d->taskTimer.start();
}

bool QmlPreviewer::working()
{
    return _working;
}

#include "qmlpreviewer.moc"
