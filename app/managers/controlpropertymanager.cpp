#include <controlpropertymanager.h>
#include <control.h>
#include <savemanager.h>
#include <saveutils.h>
#include <controlpreviewingmanager.h>

#include <QTimer>
#include <QPointer>

namespace {

qreal xWithMargin(const Control* control, bool add)
{
    qreal leftMargin = 0;
    if (control->parentControl())
        leftMargin = control->parentControl()->margins().left();
    return add ? control->x() + leftMargin : control->x() - leftMargin;
}

qreal yWithMargin(const Control* control, bool add)
{
    qreal topMargin = 0;
    if (control->parentControl())
        topMargin = control->parentControl()->margins().top();
    return add ? control->y() + topMargin : control->y() - topMargin;
}

QPointF posWithMargin(const Control* control, bool add)
{
    return QPointF(xWithMargin(control, add), yWithMargin(control, add));
}

QRectF geoWithMargin(const Control* control, bool add)
{
    return QRectF(posWithMargin(control, add), control->size());
}
}

ControlPropertyManager* ControlPropertyManager::s_instance = nullptr;
QTimer* ControlPropertyManager::s_dirtyPropertyProcessingTimer = nullptr;
QList<ControlPropertyManager::DirtyProperty> ControlPropertyManager::s_dirtyProperties;

ControlPropertyManager::ControlPropertyManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_dirtyPropertyProcessingTimer = new QTimer(this);
    s_dirtyPropertyProcessingTimer->setInterval(100);

    connect(s_dirtyPropertyProcessingTimer, &QTimer::timeout,
            this, &ControlPropertyManager::handleDirtyProperties);
}

ControlPropertyManager::~ControlPropertyManager()
{
    s_instance = nullptr;
}

ControlPropertyManager* ControlPropertyManager::instance()
{
    return s_instance;
}

void ControlPropertyManager::handleDirtyProperties()
{
    for (const DirtyProperty& dirtyProperty : s_dirtyProperties)
        dirtyProperty.function();

    s_dirtyProperties.clear();
    s_dirtyPropertyProcessingTimer->stop();
}

void ControlPropertyManager::setX(Control* control, qreal x, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if (control->hasErrors())
        return;

    bool isInt = control->propertyType("x") == QVariant::Int;

    if (!(options & DontApplyDesigner)) {
        qreal leftMargin = 0;
        if (control->parentControl())
            leftMargin = control->parentControl()->margins().left();
        control->setX(isInt ? int(leftMargin + x) : (x + leftMargin));
    }

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setX";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setX, QPointer<Control>(control),
                                           x, options ^ CompressedCall);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (options & SaveChanges) {
            if (isInt)
                SaveManager::setProperty(control, "x", QString::number(int(x)));
            else
                SaveManager::setProperty(control, "x", QString::number(x));
        }

        if (options & UpdatePreviewer) {
            if (isInt)
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", int(x));
            else
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", x);
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setY(Control* control, qreal y, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if (control->hasErrors())
        return;

    bool isInt = control->propertyType("y") == QVariant::Int;

    if (!(options & DontApplyDesigner)) {
        qreal topMargin = 0;
        if (control->parentControl())
            topMargin = control->parentControl()->margins().top();
        control->setY(isInt ? int(topMargin + y) : (y + topMargin));
    }

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setY";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setY, QPointer<Control>(control),
                                           y, options ^ CompressedCall);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (options & SaveChanges) {
            if (isInt)
                SaveManager::setProperty(control, "y", QString::number(int(y)));
            else
                SaveManager::setProperty(control, "y", QString::number(y));
        }

        if (options & UpdatePreviewer) {
            if (isInt)
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", int(y));
            else
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", y);
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setZ(Control* control, qreal z, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if (control->hasErrors())
        return;

    if (!(options & DontApplyDesigner))
        control->setZValue(z);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setZ";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setZ, QPointer<Control>(control),
                                           z, options ^ CompressedCall);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (options & SaveChanges)
            SaveManager::setProperty(control, "z", QString::number(z));

        if (options & UpdatePreviewer)
            ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "z", z);

        emit instance()->zChanged(control);
    }
}

void ControlPropertyManager::setWidth(Control* control, qreal width, Options options)
{
    if (!control)
        return;

    if (control->hasErrors())
        return;

    bool isInt = control->propertyType("width") == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->resize(isInt ? int(width) : width, control->size().height());

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setWidth";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setWidth, QPointer<Control>(control),
                                           width, options ^ CompressedCall);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (options & SaveChanges) {
            if (isInt)
                SaveManager::setProperty(control, "width", QString::number(int(width)));
            else
                SaveManager::setProperty(control, "width", QString::number(width));
        }

        if (options & UpdatePreviewer) {
            if (isInt)
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", int(width));
            else
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", width);
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setHeight(Control* control, qreal height, Options options)
{
    if (!control)
        return;

    if (control->hasErrors())
        return;

    bool isInt = control->propertyType("height") == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->resize(control->size().width(), isInt ? int(height) : height);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setHeight";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setHeight, QPointer<Control>(control),
                                           height, options ^ CompressedCall);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (options & SaveChanges) {
            if (isInt)
                SaveManager::setProperty(control, "height", QString::number(int(height)));
            else
                SaveManager::setProperty(control, "height", QString::number(height));
        }

        if (options & UpdatePreviewer) {
            if (isInt)
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", int(height));
            else
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", height);
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setPos(Control* control, const QPointF& pos, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if (control->hasErrors())
        return;

    bool isInt = control->propertyType("x") == QVariant::Int;

    if (!(options & DontApplyDesigner)) {
        qreal topMargin = 0;
        qreal leftMargin = 0;
        if (control->parentControl()) {
            topMargin = control->parentControl()->margins().top();
            leftMargin = control->parentControl()->margins().left();
        }
        const QPointF newPos(pos + QPointF{leftMargin, topMargin});
        control->setPos(isInt ? newPos.toPoint() : newPos);
    }

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setPos";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setPos, QPointer<Control>(control),
                                           pos, options ^ CompressedCall);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (options & SaveChanges) {
            if (isInt) {
                SaveManager::setProperty(control, "x", QString::number(int(pos.x())));
                SaveManager::setProperty(control, "y", QString::number(int(pos.y())));
            } else {
                SaveManager::setProperty(control, "x", QString::number(pos.x()));
                SaveManager::setProperty(control, "y", QString::number(pos.y()));
            }
        }

        if (options & UpdatePreviewer) {
            if (isInt) {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", int(pos.x()));
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", int(pos.y()));
            } else {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", pos.x());
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", pos.y());
            }
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setSize(Control* control, const QSizeF& size, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if (control->hasErrors())
        return;

    if (!size.isValid())
        return;

    bool isInt = control->propertyType("width") == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->resize(isInt ? size.toSize() : size);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setSize";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setSize, QPointer<Control>(control),
                                           size, options ^ CompressedCall);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (options & SaveChanges) {
            if (isInt) {
                SaveManager::setProperty(control, "width", QString::number(int(size.width())));
                SaveManager::setProperty(control, "height", QString::number(int(size.height())));
            } else {
                SaveManager::setProperty(control, "width", QString::number(size.width()));
                SaveManager::setProperty(control, "height", QString::number(size.height()));
            }
        }

        if (options & UpdatePreviewer) {
            if (isInt) {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", int(size.width()));
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", int(size.height()));
            } else {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", size.width());
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", size.height());
            }
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setGeometry(Control* control, const QRectF& geometry, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if (control->hasErrors())
        return;

    if (!geometry.isValid())
        return;

    bool isInt = control->propertyType("x") == QVariant::Int;

    if (!(options & DontApplyDesigner)) {
        qreal topMargin = 0;
        qreal leftMargin = 0;
        if (control->parentControl()) {
            topMargin = control->parentControl()->margins().top();
            leftMargin = control->parentControl()->margins().left();
        }
        const QRectF newGeo(geometry.adjusted(leftMargin, topMargin, leftMargin, topMargin));
        control->setGeometry(isInt ? newGeo.toRect() : newGeo);
    }

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setGeometry";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setGeometry, QPointer<Control>(control),
                                           geometry, options ^ CompressedCall);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (options & SaveChanges) {
            if (isInt) {
                SaveManager::setProperty(control, "x", QString::number(int(geometry.x())));
                SaveManager::setProperty(control, "y", QString::number(int(geometry.y())));
                SaveManager::setProperty(control, "width", QString::number(int(geometry.width())));
                SaveManager::setProperty(control, "height", QString::number(int(geometry.height())));
            } else {
                SaveManager::setProperty(control, "x", QString::number(geometry.x()));
                SaveManager::setProperty(control, "y", QString::number(geometry.y()));
                SaveManager::setProperty(control, "width", QString::number(geometry.width()));
                SaveManager::setProperty(control, "height", QString::number(geometry.height()));
            }
        }

        if (options & UpdatePreviewer) {
            if (isInt) {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", int(geometry.x()));
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", int(geometry.y()));
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", int(geometry.width()));
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", int(geometry.height()));
            } else {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", geometry.x());
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", geometry.y());
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", geometry.width());
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", geometry.height());
            }
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setParent(Control* control, Control* parentControl, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if (!parentControl)
        return;

    if (!(options & DontApplyDesigner))
        control->setParentItem(parentControl);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setParent";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setParent, QPointer<Control>(control),
                                           QPointer<Control>(parentControl), options ^ CompressedCall);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (options & SaveChanges)
            SaveManager::moveControl(control, parentControl);

        if (options & UpdatePreviewer) {
            ControlPreviewingManager::scheduleParentUpdate(control->dir(),
                                                           control->uid(), parentControl->uid());
        }

        emit instance()->parentChanged(control);
    }
}

void ControlPropertyManager::setId(Control* control, const QString& id, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if (id.isEmpty())
        return;

    const QString& previousId = control->id();

    if (options & SaveChanges)
        SaveManager::setProperty(control, "id", id);
    else
        control->setId(id);

    if (options & UpdatePreviewer)
        ControlPreviewingManager::scheduleIdUpdate(control->uid(), control->id());

    emit instance()->idChanged(control, previousId);
}

void ControlPropertyManager::setProperty(Control* control, const QString& propertyName,
                                         const QString& parserValue, const QVariant& propertyValue,
                                         Options options)
{
    Q_ASSERT(!control->hasErrors());

    if (!control)
        return;

    if (propertyName.isEmpty())
        return;

    if ((options & SaveChanges) && parserValue.isEmpty())
        return;

    Q_ASSERT(propertyName != "id"
            && propertyName != "x"
            && propertyName != "y"
            && propertyName != "z"
            && propertyName != "width"
            && propertyName != "height");

    if (options & SaveChanges)
        SaveManager::setProperty(control, propertyName, parserValue);

    if (options & UpdatePreviewer)
        ControlPreviewingManager::schedulePropertyUpdate(control->uid(), propertyName, propertyValue);

    emit instance()->propertyChanged(control, propertyName);
}