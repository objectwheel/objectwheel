#include <controlpropertymanager.h>
#include <control.h>
#include <savemanager.h>
#include <saveutils.h>
#include <controlpreviewingmanager.h>

#include <QTimer>
#include <QPointer>

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

    if (!(options & DontApplyDesigner))
        control->setX(isInt ? int(x) : x);

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
        qreal newX = xWithMargin(control, x, false);
        if (options & SaveChanges) {
            if (isInt)
                SaveManager::setProperty(control, "x", QString::number(int(newX)));
            else
                SaveManager::setProperty(control, "x", QString::number(newX));
        }

        if (options & UpdatePreviewer) {
            if (isInt)
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", int(newX));
            else
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", newX);
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

    if (!(options & DontApplyDesigner))
        control->setY(isInt ? int(y) : y);

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
        qreal newY = yWithMargin(control, y, false);
        if (options & SaveChanges) {
            if (isInt)
                SaveManager::setProperty(control, "y", QString::number(int(newY)));
            else
                SaveManager::setProperty(control, "y", QString::number(newY));
        }

        if (options & UpdatePreviewer) {
            if (isInt)
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", int(newY));
            else
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", newY);
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

    if (!(options & DontApplyDesigner))
        control->setPos(isInt ? pos.toPoint() : pos);

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
        const QPointF& newPos = posWithMargin(control, pos, false);
        if (options & SaveChanges) {
            if (isInt) {
                SaveManager::setProperty(control, "x", QString::number(int(newPos.x())));
                SaveManager::setProperty(control, "y", QString::number(int(newPos.y())));
            } else {
                SaveManager::setProperty(control, "x", QString::number(newPos.x()));
                SaveManager::setProperty(control, "y", QString::number(newPos.y()));
            }
        }

        if (options & UpdatePreviewer) {
            if (isInt) {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", int(newPos.x()));
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", int(newPos.y()));
            } else {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", newPos.x());
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", newPos.y());
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

    if (!(options & DontApplyDesigner))
        control->setGeometry(isInt ? geometry.toRect() : geometry);

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
        const QRectF& newGoe = geoWithMargin(control, geometry, false);
        if (options & SaveChanges) {
            if (isInt) {
                SaveManager::setProperty(control, "x", QString::number(int(newGoe.x())));
                SaveManager::setProperty(control, "y", QString::number(int(newGoe.y())));
                SaveManager::setProperty(control, "width", QString::number(int(newGoe.width())));
                SaveManager::setProperty(control, "height", QString::number(int(newGoe.height())));
            } else {
                SaveManager::setProperty(control, "x", QString::number(newGoe.x()));
                SaveManager::setProperty(control, "y", QString::number(newGoe.y()));
                SaveManager::setProperty(control, "width", QString::number(newGoe.width()));
                SaveManager::setProperty(control, "height", QString::number(newGoe.height()));
            }
        }

        if (options & UpdatePreviewer) {
            if (isInt) {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", int(newGoe.x()));
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", int(newGoe.y()));
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", int(newGoe.width()));
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", int(newGoe.height()));
            } else {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", newGoe.x());
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", newGoe.y());
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", newGoe.width());
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", newGoe.height());
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

    if (options & SaveChanges) // Already applies the property change to the designer
        SaveManager::setProperty(control, "id", id);
    else if (!(options & DontApplyDesigner))
        control->setId(id);

    if (options & UpdatePreviewer)
        ControlPreviewingManager::scheduleIdUpdate(control->uid(), control->id());

    emit instance()->idChanged(control, previousId);
}

void ControlPropertyManager::setIndex(Control* control, quint32 index, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if (options & SaveChanges) // Already applies the property change to the designer
        SaveManager::setIndex(control, index);
    else if (!(options & DontApplyDesigner))
        control->setIndex(index);

    if (options & UpdatePreviewer)
        ControlPreviewingManager::scheduleIndexUpdate(control->uid(), control->index());

    emit instance()->indexChanged(control);
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

qreal ControlPropertyManager::xWithMargin(const Control* control, qreal x, bool add)
{
    qreal leftMargin = 0;
    if (control->parentControl())
        leftMargin = control->parentControl()->margins().left();
    return add ? x + leftMargin : x - leftMargin;
}

qreal ControlPropertyManager::yWithMargin(const Control* control, qreal y, bool add)
{
    qreal topMargin = 0;
    if (control->parentControl())
        topMargin = control->parentControl()->margins().top();
    return add ? y + topMargin : y - topMargin;
}

QPointF ControlPropertyManager::posWithMargin(const Control* control, const QPointF& pos, bool add)
{
    return QPointF(xWithMargin(control, pos.x(), add), yWithMargin(control, pos.y(), add));
}

QRectF ControlPropertyManager::geoWithMargin(const Control* control, const QRectF& geo, bool add)
{
    return QRectF(posWithMargin(control, geo.topLeft(), add), geo.size());
}