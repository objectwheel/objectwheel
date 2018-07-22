#include <controlpropertymanager.h>
#include <control.h>
#include <savemanager.h>
#include <saveutils.h>
#include <controlpreviewingmanager.h>

#include <QTimer>
#include <QPointer>

// NOTE: Compressed calls occur in arbitrary order
// FIXME: What if we set qreal value for an int property, especially for Window qml types
// TODO: connect timeout signal of s_dirtyPropertyProcessingTimer

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

void ControlPropertyManager::setX(Control* control, qreal x, bool save, bool updatePreviewer, bool compress)
{
    if (!control)
        return;

    if (control->x() != x)
        control->setX(x);

    if (compress) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setX";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setX, QPointer<Control>(control),
                                           x, save, updatePreviewer, false);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (save)
            SaveManager::setProperty(control, "x", QString::number(control->x()));

        if (updatePreviewer)
            ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", control->x());

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setY(Control* control, qreal y, bool save, bool updatePreviewer, bool compress)
{
    if (!control)
        return;

    if (control->y() != y)
        control->setY(y);

    if (compress) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setY";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setY, QPointer<Control>(control),
                                           y, save, updatePreviewer, false);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (save)
            SaveManager::setProperty(control, "y", QString::number(control->y()));

        if (updatePreviewer)
            ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", control->y());

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setPos(Control* control, const QPointF& pos, bool save,
                                    bool updatePreviewer, bool compress)
{
    if (!control)
        return;

    if (control->pos() != pos)
        control->setPos(pos);

    if (compress) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setPos";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setPos, QPointer<Control>(control),
                                           pos, save, updatePreviewer, false);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (save) {
            SaveManager::setProperty(control, "x", QString::number(control->x()));
            SaveManager::setProperty(control, "y", QString::number(control->y()));
        }

        if (updatePreviewer) {
            ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", control->x());
            ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", control->y());
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setSize(Control* control, const QSizeF& size, bool save,
                                     bool updatePreviewer, bool compress)
{
    if (!control)
        return;

    if (!size.isValid())
        return;

    if (control->size() != size)
        control->resize(size);

    if (compress) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setSize";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setSize, QPointer<Control>(control),
                                           size, save, updatePreviewer, false);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (save) {
            SaveManager::setProperty(control, "width", QString::number(control->size().width()));
            SaveManager::setProperty(control, "height", QString::number(control->size().height()));
        }

        if (updatePreviewer) {
            ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", control->size().width());
            ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", control->size().height());
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setGeometry(Control* control, const QRectF& geometry, bool save,
                                         bool updatePreviewer, bool compress)
{
    if (!control)
        return;

    if (!geometry.isValid())
        return;

    if (control->geometry() != geometry)
        control->setGeometry(geometry);

    if (compress) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setGeometry";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setGeometry, QPointer<Control>(control),
                                           geometry, save, updatePreviewer, false);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (save) {
            SaveManager::setProperty(control, "x", QString::number(control->x()));
            SaveManager::setProperty(control, "y", QString::number(control->y()));
            SaveManager::setProperty(control, "width", QString::number(control->size().width()));
            SaveManager::setProperty(control, "height", QString::number(control->size().height()));
        }

        if (updatePreviewer) {
            ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", control->x());
            ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "y", control->y());
            ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "width", control->size().width());
            ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "height", control->size().height());
        }

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setParent(Control* control, Control* parentControl, bool save,
                                       bool updatePreviewer, bool compress)
{
    if (!control)
        return;

    if (!parentControl)
        return;

    if (control->parentItem() != parentControl)
        control->setParentItem(parentControl);

    if (compress) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setParent";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setParent, QPointer<Control>(control),
                                           QPointer<Control>(parentControl), save, updatePreviewer, false);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (save)
            SaveManager::moveControl(control, parentControl);

        if (updatePreviewer) {
            ControlPreviewingManager::scheduleParentUpdate(control->dir(),
                                                           control->uid(), parentControl->uid());
        }

        emit instance()->parentChanged(control);
    }
}

void ControlPropertyManager::setZ(Control* control, qreal z, bool save, bool updatePreviewer, bool compress)
{
    if (!control)
        return;

    if (control->zValue() != z)
        control->setZValue(z);

    if (compress) {
        DirtyProperty dirtyProperty;
        dirtyProperty.key = control->uid() + "setZ";
        dirtyProperty.function = std::bind(&ControlPropertyManager::setZ, QPointer<Control>(control),
                                           z, save, updatePreviewer, false);
        s_dirtyProperties.removeAll(dirtyProperty);
        s_dirtyProperties.append(dirtyProperty);

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        if (save)
            SaveManager::setProperty(control, "z", QString::number(control->zValue()));

        if (updatePreviewer)
            ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "z", control->zValue());

        emit instance()->zChanged(control);
    }
}

void ControlPropertyManager::setId(Control* control, const QString& id, bool save, bool updatePreviewer)
{
    if (!control)
        return;

    if (id.isEmpty())
        return;

    const QString& previousId = control->id();

    if (save)
        SaveManager::setProperty(control, "id", id);
    else
        control->setId(id);

    if (updatePreviewer)
        ControlPreviewingManager::scheduleIdUpdate(control->uid(), control->id());

    emit instance()->idChanged(control, previousId);
}

void ControlPropertyManager::setProperty(Control* control, const QString& propertyName,
                                         const QString& parserValue, const QVariant& propertyValue,
                                         bool save, bool updatePreviewer)
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

    if (save)
        SaveManager::setProperty(control, propertyName, parserValue);

    if (updatePreviewer)
        ControlPreviewingManager::schedulePropertyUpdate(control->uid(), propertyName, propertyValue);

    emit instance()->propertyChanged(control);
}