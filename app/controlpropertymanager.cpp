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
QHash<QString, std::function<void()>> ControlPropertyManager::s_dirtyPropertyHandlingFunctions;

ControlPropertyManager::ControlPropertyManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_dirtyPropertyProcessingTimer = new QTimer(this);
    s_dirtyPropertyProcessingTimer->setInterval(100);
}

ControlPropertyManager::~ControlPropertyManager()
{
    s_instance = nullptr;
}

ControlPropertyManager* ControlPropertyManager::instance()
{
    return s_instance;
}

void ControlPropertyManager::setX(Control* control, qreal x, bool save, bool updatePreviewer, bool compress)
{
    if (!control)
        return;

    if (compress) {
        s_dirtyPropertyHandlingFunctions.insert(control->uid() + "setX",
                                                std::bind(&ControlPropertyManager::setX,
                                                          QPointer<Control>(control), x, save,
                                                          updatePreviewer, false));

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        control->setX(x);

        if (save) {
            SaveUtils::setX(control->dir(), control->x());
            SaveManager::setProperty(control, "x", QString::number(control->x()));
        }

        if (updatePreviewer)
            ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "x", control->x());

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setY(Control* control, qreal y, bool save, bool updatePreviewer, bool compress)
{
    if (!control)
        return;

    if (compress) {
        s_dirtyPropertyHandlingFunctions.insert(control->uid() + "setY",
                                                std::bind(&ControlPropertyManager::setY,
                                                          QPointer<Control>(control), y, save,
                                                          updatePreviewer, false));

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        control->setY(y);

        if (save) {
            SaveUtils::setY(control->dir(), control->y());
            SaveManager::setProperty(control, "y", QString::number(control->y()));
        }

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

    if (compress) {
        s_dirtyPropertyHandlingFunctions.insert(control->uid() + "setPos",
                                                std::bind(&ControlPropertyManager::setPos,
                                                          QPointer<Control>(control), pos, save,
                                                          updatePreviewer, false));

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        control->setPos(pos);

        if (save) {
            SaveUtils::setX(control->dir(), control->x());
            SaveUtils::setY(control->dir(), control->y());
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

    if (compress) {
        s_dirtyPropertyHandlingFunctions.insert(control->uid() + "setSize",
                                                std::bind(&ControlPropertyManager::setSize,
                                                          QPointer<Control>(control), size, save,
                                                          updatePreviewer, false));

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        control->resize(size);

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

    if (compress) {
        s_dirtyPropertyHandlingFunctions.insert(control->uid() + "setGeometry",
                                                std::bind(&ControlPropertyManager::setGeometry,
                                                          QPointer<Control>(control), geometry, save,
                                                          updatePreviewer, false));

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        control->setGeometry(geometry);

        if (save) {
            SaveUtils::setX(control->dir(), control->x());
            SaveUtils::setY(control->dir(), control->y());
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

void ControlPropertyManager::setId(Control* control, const QString& id, bool save,
                                   bool updatePreviewer, bool compress)
{
    if (!control)
        return;

    if (id.isEmpty())
        return;

    if (compress) {
        s_dirtyPropertyHandlingFunctions.insert(control->uid() + "setId",
                                                std::bind(&ControlPropertyManager::setId,
                                                          QPointer<Control>(control), id, save,
                                                          updatePreviewer, false));

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        const QString& previousId = control->id();

        control->setId(id);

        if (save)
            SaveManager::setProperty(control, "id", id);

        if (updatePreviewer)
            ControlPreviewingManager::scheduleIdChange(control->uid(), control->id());

        emit instance()->idChanged(control, previousId);
    }
}

void ControlPropertyManager::setParent(Control* control, Control* parentControl, bool save,
                                       bool updatePreviewer, bool compress)
{
    if (!control)
        return;

    if (!parentControl)
        return;

    if (compress) {
        s_dirtyPropertyHandlingFunctions.insert(control->uid() + "setParent",
                                                std::bind(&ControlPropertyManager::setParent,
                                                          QPointer<Control>(control),
                                                          QPointer<Control>(parentControl), save,
                                                          updatePreviewer, false));

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        control->setParentItem(parentControl);

        if (save)
            SaveManager::moveControl(control, parentControl);

        if (updatePreviewer) // FIXME: newUrl parameter?
            ControlPreviewingManager::scheduleParentUpdate(control->uid(), parentControl->uid(), "");

        emit instance()->parentChanged(control);
    }
}

void ControlPropertyManager::setZ(Control* control, qreal z, bool save, bool updatePreviewer, bool compress)
{
    if (!control)
        return;

    if (compress) {
        s_dirtyPropertyHandlingFunctions.insert(control->uid() + "setZ",
                                                std::bind(&ControlPropertyManager::setZ,
                                                          QPointer<Control>(control), z, save,
                                                          updatePreviewer, false));

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        control->setZValue(z);

        if (save) {
            SaveManager::setProperty(control, "z", QString::number(control->zValue()));
        }

        if (updatePreviewer)
            ControlPreviewingManager::schedulePropertyUpdate(control->uid(), "z", control->zValue());

        emit instance()->zChanged(control);
    }
}
