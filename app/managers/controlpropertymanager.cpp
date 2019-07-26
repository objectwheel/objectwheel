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

QVariant ControlPropertyManager::geometryHashedValue(const QVariant& value, const QString& hash)
{
    return UtilityFunctions::push(value, hash);
}

void ControlPropertyManager::setX(Control* control, qreal x, ControlPropertyManager::Options options, const QString& geometryHash)
{
    if (!control)
        return;

    // Not needeed since non-gui covers up error
    // state, and non-gui state is handled below
    //    if (control->hasErrors())
    //        return;

    bool isInt = control->propertyType("x") == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->setX(isInt ? int(x) : x);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setX";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setX, QPointer<Control>(control),
                                           x, (options & ~CompressedCall) | DontApplyDesigner, geometryHash);
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

        if (options & UpdateRenderer) {
            if (isInt)
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "x", geometryHashedValue(int(newX), geometryHash));
            else
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "x", geometryHashedValue(newX, geometryHash));
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setY(Control* control, qreal y, ControlPropertyManager::Options options, const QString& geometryHash)
{
    if (!control)
        return;

    // Not needeed since non-gui covers up error
    // state, and non-gui state is handled below
    //    if (control->hasErrors())
    //        return;

    bool isInt = control->propertyType("y") == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->setY(isInt ? int(y) : y);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setY";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setY, QPointer<Control>(control),
                                           y, (options & ~CompressedCall) | DontApplyDesigner, geometryHash);
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

        if (options & UpdateRenderer) {
            if (isInt)
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "y", geometryHashedValue(int(newY), geometryHash));
            else
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "y", geometryHashedValue(newY, geometryHash));
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

void ControlPropertyManager::setWidth(Control* control, qreal width, Options options, const QString& geometryHash)
{
    if (!control)
        return;

    if (control->hasErrors())
        return;

    bool isInt = control->propertyType("width") == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->setSize(isInt ? int(width) : width, control->height());

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setWidth";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setWidth, QPointer<Control>(control),
                                           width, (options & ~CompressedCall) | DontApplyDesigner, geometryHash);
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
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "width", geometryHashedValue(int(width), geometryHash));
            else
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "width", geometryHashedValue(width, geometryHash));
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setHeight(Control* control, qreal height, Options options, const QString& geometryHash)
{
    if (!control)
        return;

    if (control->hasErrors())
        return;

    bool isInt = control->propertyType("height") == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->setSize(control->width(), isInt ? int(height) : height);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setHeight";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setHeight, QPointer<Control>(control),
                                           height, (options & ~CompressedCall) | DontApplyDesigner, geometryHash);
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
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "height", geometryHashedValue(int(height), geometryHash));
            else
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "height", geometryHashedValue(height, geometryHash));
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setPos(Control* control, const QPointF& pos,
                                    ControlPropertyManager::Options options,
                                    const QString& geometryHash)
{
    if (!control)
        return;

    // Not needeed since non-gui covers up error
    // state, and non-gui state is handled below
    //    if (control->hasErrors())
    //        return;

    bool isInt = control->propertyType("x") == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->setPos(isInt ? pos.toPoint() : pos);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setPos";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setPos, QPointer<Control>(control),
                                           pos, (options & ~CompressedCall) | DontApplyDesigner, geometryHash);
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

        if (options & UpdateRenderer) {
            if (isInt) {
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "x", geometryHashedValue(int(newPos.x())));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "y", geometryHashedValue(int(newPos.y()), geometryHash));
            } else {
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "x", geometryHashedValue(newPos.x()));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "y", geometryHashedValue(newPos.y(), geometryHash));
            }
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setSize(Control* control, const QSizeF& size,
                                     ControlPropertyManager::Options options,
                                     const QString& geometryHash)
{
    if (!control)
        return;

    if (control->hasErrors())
        return;

    if (!size.isValid())
        return;

    bool isInt = control->propertyType("width") == QVariant::Int;

    if (!(options & DontApplyDesigner))
        control->setSize(isInt ? size.toSize() : size);

    if (options & CompressedCall) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setSize";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setSize, QPointer<Control>(control),
                                           size, (options & ~CompressedCall) | DontApplyDesigner, geometryHash);
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
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "width", geometryHashedValue(int(size.width())));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "height", geometryHashedValue(int(size.height()), geometryHash));
            } else {
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "width", geometryHashedValue(size.width()));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "height", geometryHashedValue(size.height(), geometryHash));
            }
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setGeometry(Control* control, const QRectF& geometry, ControlPropertyManager::Options options, const QString& geometryHash)
{
    if (!control)
        return;

    // NOTE: Becareful using this on errorneus controls
    // if you want to save position information
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
                                           geometry, (options & ~CompressedCall) | DontApplyDesigner, geometryHash);
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

        if (options & UpdateRenderer) {
            if (isInt) {
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "x", geometryHashedValue(int(newGoe.x())));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "y", geometryHashedValue(int(newGoe.y())));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "width", geometryHashedValue(int(newGoe.width())));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "height", geometryHashedValue(int(newGoe.height()), geometryHash));
            } else {
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "x", geometryHashedValue(newGoe.x()));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "y", geometryHashedValue(newGoe.y()));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "width", geometryHashedValue(newGoe.width()));
                ControlRenderingManager::schedulePropertyUpdate(control->uid(), "height", geometryHashedValue(newGoe.height(), geometryHash));
            }
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setParent(Control* control, Control* parentControl, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    Q_ASSERT(!control->form());

    if (!parentControl)
        return;

    // If parent change is gonna be saved, make sure to fix indexes of the
    // previous siblings of the related control before setting new parent
    if (options & SaveChanges)
        // FIXME: Should we also add UpdateRenderer here?
        setIndex(control, std::numeric_limits<quint32>::max(), SaveChanges);

    if (!(options & DontApplyDesigner))
        control->setParentItem(parentControl);

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