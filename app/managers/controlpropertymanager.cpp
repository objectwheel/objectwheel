#include <controlpropertymanager.h>
#include <control.h>
#include <savemanager.h>
#include <saveutils.h>
#include <controlrenderingmanager.h>
#include <designerscene.h>
#include <utilityfunctions.h>

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

QVariant ControlPropertyManager::geometrySyncValue(const QVariant& value, const QString& hash)
{
    return UtilityFunctions::push(value, hash);
}

void ControlPropertyManager::setX(Control* control, qreal x, ControlPropertyManager::Options options, const QString& geometrySyncKey)
{
    if (!control)
        return;

    bool isInt = control->property("x").type() == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->setX(isInt ? int(x) : x);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setX";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setX, QPointer<Control>(control),
                                           x, (options & ~CompressedCall) | DontApplyDesigner, geometrySyncKey);
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

        if (options & UpdateRenderer) {
            if (isInt)
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "x", geometrySyncValue(int(x), geometrySyncKey));
            else
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "x", geometrySyncValue(x, geometrySyncKey));
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setY(Control* control, qreal y, ControlPropertyManager::Options options, const QString& geometrySyncKey)
{
    if (!control)
        return;

    bool isInt = control->property("y").type() == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->setY(isInt ? int(y) : y);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setY";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setY, QPointer<Control>(control),
                                           y, (options & ~CompressedCall) | DontApplyDesigner, geometrySyncKey);
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

        if (options & UpdateRenderer) {
            if (isInt)
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "y", geometrySyncValue(int(y), geometrySyncKey));
            else
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "y", geometrySyncValue(y, geometrySyncKey));
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setZ(Control* control, qreal z, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if (!(options & DontApplyDesigner))
        control->setZValue(z);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setZ";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setZ, QPointer<Control>(control),
                                           z, (options & ~CompressedCall) | DontApplyDesigner);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (options & SaveChanges)
            SaveManager::setProperty(control, "z", QString::number(z));

        if (options & UpdateRenderer)
            ControlRenderingManager::schedulePropertyUpdate(control->uid(), "z", z);

        emit instance()->zChanged(control);
    }
}

void ControlPropertyManager::setWidth(Control* control, qreal width, Options options, const QString& geometrySyncKey)
{
    if (!control)
        return;

    bool isInt = control->property("width").type() == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->setSize(isInt ? int(width) : width, control->height());

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setWidth";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setWidth, QPointer<Control>(control),
                                           width, (options & ~CompressedCall) | DontApplyDesigner, geometrySyncKey);
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

        if (options & UpdateRenderer) {
            if (isInt)
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "width", geometrySyncValue(int(width), geometrySyncKey));
            else
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "width", geometrySyncValue(width, geometrySyncKey));
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setHeight(Control* control, qreal height, Options options, const QString& geometrySyncKey)
{
    if (!control)
        return;

    bool isInt = control->property("height").type() == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->setSize(control->width(), isInt ? int(height) : height);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setHeight";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setHeight, QPointer<Control>(control),
                                           height, (options & ~CompressedCall) | DontApplyDesigner, geometrySyncKey);
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

        if (options & UpdateRenderer) {
            if (isInt)
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "height", geometrySyncValue(int(height), geometrySyncKey));
            else
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "height", geometrySyncValue(height, geometrySyncKey));
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setPos(Control* control, const QPointF& pos,
                                    ControlPropertyManager::Options options,
                                    const QString& geometrySyncKey)
{
    if (!control)
        return;

    bool isInt = control->property("x").type() == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->setPos(isInt ? pos.toPoint() : pos);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setPos";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setPos, QPointer<Control>(control),
                                           pos, (options & ~CompressedCall) | DontApplyDesigner, geometrySyncKey);
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

        if (options & UpdateRenderer) {
            if (isInt) {
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "x", geometrySyncValue(int(pos.x())));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "y", geometrySyncValue(int(pos.y()), geometrySyncKey));
            } else {
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "x", geometrySyncValue(pos.x()));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "y", geometrySyncValue(pos.y(), geometrySyncKey));
            }
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setSize(Control* control, const QSizeF& size,
                                     ControlPropertyManager::Options options,
                                     const QString& geometrySyncKey)
{
    if (!control)
        return;

    if (!size.isValid())
        return;

    bool isInt = control->property("width").type() == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->setSize(isInt ? size.toSize() : size);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setSize";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setSize, QPointer<Control>(control),
                                           size, (options & ~CompressedCall) | DontApplyDesigner, geometrySyncKey);
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

        if (options & UpdateRenderer) {
            if (isInt) {
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "width", geometrySyncValue(int(size.width())));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "height", geometrySyncValue(int(size.height()), geometrySyncKey));
            } else {
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "width", geometrySyncValue(size.width()));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "height", geometrySyncValue(size.height(), geometrySyncKey));
            }
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setGeometry(Control* control, const QRectF& geometry, ControlPropertyManager::Options options, const QString& geometrySyncKey)
{
    if (!control)
        return;

    if (!geometry.isValid())
        return;

    bool isInt = control->property("x").type() == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->setGeometry(isInt ? geometry.toRect() : geometry);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setGeometry";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setGeometry, QPointer<Control>(control),
                                           geometry, (options & ~CompressedCall) | DontApplyDesigner, geometrySyncKey);
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

        if (options & UpdateRenderer) {
            if (isInt) {
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "x", geometrySyncValue(int(geometry.x())));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "y", geometrySyncValue(int(geometry.y())));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "width", geometrySyncValue(int(geometry.width())));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "height", geometrySyncValue(int(geometry.height()), geometrySyncKey));
            } else {
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "x", geometrySyncValue(geometry.x()));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "y", geometrySyncValue(geometry.y()));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "width", geometrySyncValue(geometry.width()));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "height", geometrySyncValue(geometry.height(), geometrySyncKey));
            }
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setParent(Control* control, Control* parentControl, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    Q_ASSERT(control->type() != Form::Type);

    if (!parentControl)
        return;

    // If parent change is gonna be saved, make sure to fix indexes of the
    // previous siblings of the related control before setting new parent
    if (options & SaveChanges)
        // FIXME: Should we also add UpdateRenderer here?
        setIndex(control, std::numeric_limits<quint32>::max(), SaveChanges);

    if (!(options & DontApplyDesigner)) {
        control->setParentItem(parentControl);
        control->setTransform(QTransform::fromTranslate(parentControl->margins().left(),
                                                        parentControl->margins().top()));
    }

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setParent";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setParent, QPointer<Control>(control),
                                           QPointer<Control>(parentControl),
                                           (options & ~CompressedCall) | DontApplyDesigner);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (options & SaveChanges) {
            SaveManager::moveControl(control, parentControl);
            // FIXME: Should we also add UpdateRenderer here?
            setIndex(control, control->siblings().size(), SaveChanges);
        }

        if (options & UpdateRenderer) {
            ControlRenderingManager::scheduleParentUpdate(control->dir(),
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

    if (options & UpdateRenderer)
        ControlRenderingManager::scheduleIdUpdate(control->uid(), control->id());

    emit instance()->idChanged(control, previousId);
}

// NOTE: Call for controls only these are present on the Designer Scene
void ControlPropertyManager::setIndex(Control* control, quint32 index, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if (options & SaveChanges) // Already applies the property change to the designer
        SaveManager::setIndex(control, index);
    else if (!(options & DontApplyDesigner))
        control->setIndex(index);

    if (options & UpdateRenderer)
        ControlRenderingManager::scheduleIndexUpdate(control->uid());

    emit instance()->indexChanged(control);
}

void ControlPropertyManager::setBinding(Control* control, const QString& bindingName,
                                         const QString& expression, Options options)
{
    Q_ASSERT(!control->hasErrors());

    if (!control)
        return;

    if (bindingName.isEmpty())
        return;

    Q_ASSERT(bindingName != "id"
            && bindingName != "x"
            && bindingName != "y"
            && bindingName != "z"
            && bindingName != "width"
            && bindingName != "height");

    if (options & SaveChanges)
        SaveManager::setProperty(control, bindingName, expression);

    if (options & UpdateRenderer)
        ControlRenderingManager::scheduleBindingUpdate(control->uid(), bindingName, expression);

    emit instance()->propertyChanged(control, bindingName);
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

    if (options & UpdateRenderer)
        ControlRenderingManager::schedulePropertyUpdate(control->uid(), propertyName, propertyValue);

    emit instance()->propertyChanged(control, propertyName);
}
