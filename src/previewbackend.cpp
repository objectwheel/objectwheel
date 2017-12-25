#include <previewbackend.h>
#include <fit.h>
#include <formscene.h>
#include <filemanager.h>
#include <savebackend.h>
#include <control.h>
#include <delayer.h>
#include <parserworker.h>

#include <QApplication>
#include <QQuickWindow>
#include <QSharedPointer>
#include <QQuickItem>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QImage>
#include <QBrush>
#include <QImage>
#include <QtMath>
#include <QTimer>
#include <QPainter>
#include <QTimer>
#include <QPair>
#include <QScreen>

#define TASK_TIMEOUT 10
#define SIZE_ERROR_PIXMAP (QSizeF(fit::fx(16), fit::fx(16)))
#define SIZE_NONGUI_PIXMAP (QSize(fit::fx(16), fit::fx(16)))
#define SIZE_INITIAL_PIXMAP (QSizeF(fit::fx(50), fit::fx(50)))
#define SIZE_NOPREVIEW_PIXMAP (QSizeF(fit::fx(40), fit::fx(40)))
#define pS (QApplication::primaryScreen())

class PreviewBackendPrivate : public QObject
{
        Q_OBJECT
    public:
        PreviewBackendPrivate(PreviewBackend*);
        QSharedPointer<PreviewResult> preview(Control*, const QString&);
        void draw(QImage&, const QImage&, const QSizeF&) const;

    public:
        PreviewBackend* parent;
        QTimer taskTimer;
        QList<QPointer<Control>> taskList;
        QImage initialImage;

    private slots:
        void processTasks();

    private:
        void dash(Control* control, QImage&) const;
        bool containsWindow(QObject*) const;
        QImage prepreview(const Control*) const;
        QList<QString> events(const QObject*) const;
        PropertyNodes properties(const QObject*) const;

    private:
        QImage errorImage, nopreviewImage;
        QQuickWindow renderWindow;
};

PreviewBackendPrivate::PreviewBackendPrivate(PreviewBackend* parent)
    : QObject(parent)
    , parent(parent)
    , initialImage(QImage(":/resources/images/wait.png").
      scaled((SIZE_INITIAL_PIXMAP * pS->devicePixelRatio()).toSize(),
      Qt::IgnoreAspectRatio, Qt::SmoothTransformation))
    , errorImage(QImage(":/resources/images/error.png").
      scaled((SIZE_ERROR_PIXMAP * pS->devicePixelRatio()).toSize(),
      Qt::IgnoreAspectRatio, Qt::SmoothTransformation))
    , nopreviewImage(QImage(":/resources/images/nopreview.png").
      scaled((SIZE_NOPREVIEW_PIXMAP * pS->devicePixelRatio()).toSize(),
      Qt::IgnoreAspectRatio, Qt::SmoothTransformation))
{
    taskTimer.setInterval(TASK_TIMEOUT);
    connect(&taskTimer, SIGNAL(timeout()), SLOT(processTasks()));

    renderWindow.setColor(Qt::transparent);
    renderWindow.setClearBeforeRendering(true);
    renderWindow.setFlags(Qt::Window | Qt::FramelessWindowHint);
    renderWindow.setOpacity(0);
    renderWindow.hide();
    renderWindow.create();
}

void PreviewBackendPrivate::dash(Control* control, QImage& image) const
{
    // Check 15 pixels atleast that has alpha > 250
    int totalAlpha = 0;
    for (int i = 0; i < image.width(); i++) {
        for (int j = 0; j < image.height(); j++) {
            totalAlpha += qAlpha(image.pixel(i, j));
            if (totalAlpha > (250 * 15)) {
                return;
            }
        }
    }

    draw(image, nopreviewImage, control->size());
}

QImage PreviewBackendPrivate::prepreview(const Control* control) const
{
    QImage image(qCeil(control->size().width() * pS->devicePixelRatio()),
      qCeil(control->size().height() * pS->devicePixelRatio()),
       QImage::Format_ARGB32);
    image.setDevicePixelRatio(pS->devicePixelRatio());
    image.fill(Qt::transparent);
    draw(image, errorImage, control->size());
    return image;
}

bool PreviewBackendPrivate::containsWindow(QObject* o) const
{
    for (auto _o : o->findChildren<QObject*>()) {
        if (_o->isWindowType())
            return true;
    }
    return false;
}

PropertyNodes PreviewBackendPrivate::properties(const QObject* object) const
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
        propertyNode.metaObject = object->staticMetaObject;

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

void PreviewBackendPrivate::draw(QImage& dest, const QImage& source, const QSizeF& size) const
{
    auto r = QRectF({QPointF(), size}).adjusted(1, 1, -1, -1);
    QRectF wr(QPoint(), QSizeF(source.size()) / pS->devicePixelRatio());
    wr.moveCenter(r.center());
    QPainter painter(&dest);
    painter.setRenderHint(QPainter::Antialiasing);
    QBrush brush;
    brush.setColor("#808487");
    brush.setStyle(Qt::Dense6Pattern);
    QPen pen;
    pen.setWidthF(fit::fx(1));
    pen.setStyle(Qt::DotLine);
    pen.setColor(brush.color());
    painter.setPen(pen);
    painter.drawRect(r);
    brush.setColor("#b0b4b7");
    painter.setBrush(brush);
    painter.drawRect(r);
    painter.drawImage(wr, source, source.rect());
    // Draw corner lines
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    painter.drawLine(r.topLeft(), r.topLeft() + QPointF(2, 0));
    painter.drawLine(r.topLeft(), r.topLeft() + QPointF(0, 2));
    painter.drawLine(r.bottomLeft(), r.bottomLeft() + QPointF(2, 0));
    painter.drawLine(r.bottomLeft(), r.bottomLeft() + QPointF(0, -2));
    painter.drawLine(r.topRight(), r.topRight() + QPointF(-2, 0));
    painter.drawLine(r.topRight(), r.topRight() + QPointF(0, 2));
    painter.drawLine(r.bottomRight(), r.bottomRight() + QPointF(-2, 0));
    painter.drawLine(r.bottomRight(), r.bottomRight() + QPointF(0, -2));
}

QList<QString> PreviewBackendPrivate::events(const QObject* object) const
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

QSharedPointer<PreviewResult> PreviewBackendPrivate::preview(Control* control, const QString& url)
{
    QSharedPointer<PreviewResult> result(new PreviewResult);

    if (!SaveBackend::isOwctrl(dname(dname(url)))) {
        qFatal("Fatal error: Control doesn't meet Owctrl™ requirements.");
        return result;
    }

    QQmlEngine engine;
    QQmlComponent component(&engine);

    engine.setOutputWarningsToStandardError(false);
    engine.rootContext()->setContextProperty("dpi", fit::ratio()); //FIXME

    // FIXME: What if qml file contains errors?
    // BUG: Possible bug if property 'visible' is a binding
    bool isWindow = false;
    auto qml = rdfile(url);
    ParserWorker pw;
    if (pw.typeName(qml).contains("Window")) {
        pw.setVariantProperty(qml, url, "visible", false);
        isWindow = true;
    }

    result->control = control;
    result->preview = prepreview(control);

    if (isWindow && !control->form()) {
        QQmlError error;
        error.setDescription("Only forms can be 'Window' qml type.");
        result->errors = (QList<QQmlError>() << error);
        return result;
    }

    component.setData(qml, QUrl::fromLocalFile(url));
    auto object = component.create();
    engine.setObjectOwnership(object, QQmlEngine::JavaScriptOwnership);

    if (component.isError()) {
        result->errors = component.errors();
        return result;
    }

    if (containsWindow(object)) {
        QQmlError error;
        error.setDescription("You can not define child objects as 'Window' qml type.");
        result->errors = (QList<QQmlError>() << error);
        return result;
    }

    auto w = qobject_cast<QQuickWindow*>(object);
    result->properties = properties(object);
    result->events = events(object);
    result->gui = (object->inherits("QQuickItem") || isWindow);

    if (result->gui == false) {
        QImage p(qCeil(control->size().width() * pS->devicePixelRatio()),
          qCeil(control->size().height() * pS->devicePixelRatio()),
            QImage::Format_ARGB32);
        p.setDevicePixelRatio(pS->devicePixelRatio());
        p.fill(Qt::transparent);
        draw(p, QImage(dname(url) + separator() + "icon.png")
          .scaled(SIZE_NONGUI_PIXMAP * pS->devicePixelRatio()), control->size());
        result->preview = p;
    } else {
        QQuickItem* item = nullptr;
        if (w) {
            item = w->contentItem();
            renderWindow.setColor(w->color());
        } else {
            item = qobject_cast<QQuickItem*>(object);
            renderWindow.setColor(Qt::transparent);
        }
        Q_ASSERT(item);

        item->setParentItem(renderWindow.contentItem());
        item->setPosition({0, 0});
        item->setSize(control->size());
        renderWindow.resize(qCeil(item->width()), qCeil(item->height()));

        Delayer::delay(60);
        result->preview = renderWindow.grabWindow();
        dash(control, result->preview);
    }
    return result;
}

void PreviewBackendPrivate::processTasks()
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
    auto masterPaths = SaveBackend::masterPaths(dir);

    // If it's a form, tool or non-master child control, then we just need a background
    // preview image. So, we don't need a nested-preview image.
    if (SaveBackend::suid(dir).isEmpty() || masterPaths.isEmpty()) {
        auto res = preview(control, dir + separator() + DIR_THIS +
          separator() + "main.qml");
        emit parent->previewReady(res);
        parent->_working = false;
        emit parent->workingChanged(parent->_working);
    } else {
        // Only master-child items are handled here, others are handled above
        QSharedPointer<PreviewResult> finalResult = nullptr;
        QMap<QString, QSharedPointer<PreviewResult>> masterResults;

        for (auto path : masterPaths) {
            QMap<QString, QSharedPointer<PreviewResult>> results;
            auto childrenPaths = SaveBackend::childrenPaths(path);
            for (auto childPath : childrenPaths) {
                int index = masterPaths.indexOf(childPath);
                if (index >= 0) {
                    results[childPath] = masterResults[childPath];
                } else {
                    results[childPath] = preview(control, childPath +
                      separator() + DIR_THIS + separator() + "main.qml");
                    if (results[childPath]->hasError()) {
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
                finalResult = masterResults[path];
                if (masterResults[path]->hasError()) {
                    emit parent->previewReady(masterResults[path]);
                    parent->_working = false;
                    emit parent->workingChanged(parent->_working);
                    return;
                }
            } else {
                masterResults[path] = preview(control, path +
                  separator() + DIR_THIS + separator() + "main.qml");
                if (masterResults[path]->hasError()) {
                    emit parent->previewReady(masterResults[path]);
                    parent->_working = false;
                    emit parent->workingChanged(parent->_working);
                    return;
                }
            }

            QImage* parentImage;
            for (auto result : results.keys()) { //FIXME: Some child previews lost, wrong 'parentImage' logic, use QMap here to fix
                if (dname(dname(result)) == path)
                    parentImage = &masterResults[path]->preview;

                auto& r = results[result];
                const QPointF p = r->control->pos();
                const QSizeF s = r->control->size();

                QPainter px(parentImage);
                px.setRenderHint(QPainter::Antialiasing);
                px.drawImage(QRectF(p, s), r->preview, QRectF(QPointF(0, 0),
                  s * pS->devicePixelRatio()));
                px.end();

                parentImage = &r->preview;
            }
        }

        emit parent->previewReady(finalResult);
        parent->_working = false;
        emit parent->workingChanged(parent->_working);
    }
}

bool PreviewBackend::_working = false;
PreviewBackendPrivate* PreviewBackend::_d = nullptr;

PreviewBackend::PreviewBackend(QObject *parent) : QObject(parent)
{
    if (_d)
        return;

    _d = new PreviewBackendPrivate(this);
}

PreviewBackend* PreviewBackend::instance()
{
    return _d ? _d->parent : nullptr;
}

void PreviewBackend::requestPreview(Control* control)
{
    if (_d->taskList.isEmpty())
        _d->taskTimer.start();
    if (!_d->taskList.contains(control))
        _d->taskList.append(control);
}

bool PreviewBackend::working()
{
    return _working;
}

QImage PreviewBackend::initialPreview(const QSizeF& size)
{
    QImage image(qCeil(size.width() * pS->devicePixelRatio()),
      qCeil(size.height() * pS->devicePixelRatio()),
        QImage::Format_ARGB32);
    image.setDevicePixelRatio(pS->devicePixelRatio());
    image.fill(Qt::transparent);
    _d->draw(image, _d->initialImage, size);
    return image;
}

#include "previewbackend.moc"
