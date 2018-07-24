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

    if ((options & IntegerValue) ? int(control->x()) != int(x) : control->x() != x)
        control->setX((options & IntegerValue) ? int(x) : x);

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
            if (options & IntegerValue)
                SaveManager::setProperty(control, "x", QString::number(int(control->x())));
            else
                SaveManager::setProperty(control, "x", QString::number(control->x()));
        }

        if (options & UpdatePreviewer) {
            if (options & IntegerValue)
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", int(control->x()));
            else
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", control->x());
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setY(Control* control, qreal y, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if ((options & IntegerValue) ? int(control->y()) != int(y) : control->y() != y)
        control->setY((options & IntegerValue) ? int(y) : y);

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
            if (options & IntegerValue)
                SaveManager::setProperty(control, "y", QString::number(int(control->y())));
            else
                SaveManager::setProperty(control, "y", QString::number(control->y()));
        }

        if (options & UpdatePreviewer) {
            if (options & IntegerValue)
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", int(control->y()));
            else
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", control->y());
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setZ(Control* control, qreal z, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if ((options & IntegerValue) ? int(control->zValue()) != int(z) : control->zValue() != z)
        control->setZValue((options & IntegerValue) ? int(z) : z);

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
        if (options & SaveChanges) {
            if (options & IntegerValue)
                SaveManager::setProperty(control, "z", QString::number(int(control->zValue())));
            else
                SaveManager::setProperty(control, "z", QString::number(control->zValue()));
        }

        if (options & UpdatePreviewer) {
            if (options & IntegerValue)
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "z", int(control->zValue()));
            else
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "z", control->zValue());
        }

        emit instance()->zChanged(control);
    }
}

void ControlPropertyManager::setWidth(Control* control, qreal width, Options options)
{
    if (!control)
        return;

    if ((options & IntegerValue) ?
            int(control->size().width()) != int(width) : control->size().width() != width) {
        control->resize((options & IntegerValue) ? int(width) : width, control->size().height());
    }

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
            if (options & IntegerValue)
                SaveManager::setProperty(control, "width", QString::number(int(control->size().width())));
            else
                SaveManager::setProperty(control, "width", QString::number(control->size().width()));
        }

        if (options & UpdatePreviewer) {
            if (options & IntegerValue)
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", int(control->size().width()));
            else
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", control->size().width());
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setHeight(Control* control, qreal height, Options options)
{
    if (!control)
        return;

    if ((options & IntegerValue) ?
            int(control->size().height()) != int(height) : control->size().height() != height) {
        control->resize(control->size().width(), (options & IntegerValue) ? int(height) : height);
    }

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
            if (options & IntegerValue)
                SaveManager::setProperty(control, "height", QString::number(int(control->size().height())));
            else
                SaveManager::setProperty(control, "height", QString::number(control->size().height()));
        }

        if (options & UpdatePreviewer) {
            if (options & IntegerValue)
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", int(control->size().height()));
            else
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", control->size().height());
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setPos(Control* control, const QPointF& pos, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if ((options & IntegerValue) ?
            control->pos().toPoint() != pos.toPoint() : control->pos() != pos) {
        control->setPos((options & IntegerValue) ? pos.toPoint() : pos);
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
            if (options & IntegerValue) {
                SaveManager::setProperty(control, "x", QString::number(int(control->x())));
                SaveManager::setProperty(control, "y", QString::number(int(control->y())));
            } else {
                SaveManager::setProperty(control, "x", QString::number(control->x()));
                SaveManager::setProperty(control, "y", QString::number(control->y()));
            }
        }

        if (options & UpdatePreviewer) {
            if (options & IntegerValue) {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", int(control->x()));
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", int(control->y()));
            } else {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", control->x());
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", control->y());
            }
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setSize(Control* control, const QSizeF& size, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if (!size.isValid())
        return;

    if ((options & IntegerValue) ?
            control->size().toSize() != size.toSize() : control->size() != size) {
        control->resize((options & IntegerValue) ? size.toSize() : size);
    }

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
            if (options & IntegerValue) {
                SaveManager::setProperty(control, "width", QString::number(int(control->size().width())));
                SaveManager::setProperty(control, "height", QString::number(int(control->size().height())));
            } else {
                SaveManager::setProperty(control, "width", QString::number(control->size().width()));
                SaveManager::setProperty(control, "height", QString::number(control->size().height()));
            }
        }

        if (options & UpdatePreviewer) {
            if (options & IntegerValue) {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", int(control->size().width()));
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", int(control->size().height()));
            } else {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", control->size().width());
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", control->size().height());
            }
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setGeometry(Control* control, const QRectF& geometry, ControlPropertyManager::Options options)
{
    if (!control)
        return;

    if (!geometry.isValid())
        return;

    if ((options & IntegerValue) ?
            control->geometry().toRect() != geometry.toRect() : control->geometry() != geometry) {
        control->setGeometry((options & IntegerValue) ? geometry.toRect() : geometry);
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
            if (options & IntegerValue) {
                SaveManager::setProperty(control, "x", QString::number(int(control->x())));
                SaveManager::setProperty(control, "y", QString::number(int(control->y())));
                SaveManager::setProperty(control, "width", QString::number(int(control->size().width())));
                SaveManager::setProperty(control, "height", QString::number(int(control->size().height())));
            } else {
                SaveManager::setProperty(control, "x", QString::number(control->x()));
                SaveManager::setProperty(control, "y", QString::number(control->y()));
                SaveManager::setProperty(control, "width", QString::number(control->size().width()));
                SaveManager::setProperty(control, "height", QString::number(control->size().height()));
            }
        }

        if (options & UpdatePreviewer) {
            if (options & IntegerValue) {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", int(control->x()));
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", int(control->y()));
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", int(control->size().width()));
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", int(control->size().height()));
            } else {
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", control->x());
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", control->y());
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", control->size().width());
                ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", control->size().height());
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

    if (control->parentItem() != parentControl)
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
    if (!control)
        return;

    if (propertyName.isEmpty())
        return;

    if (parserValue.isEmpty())
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

    emit instance()->propertyChanged(control);
}