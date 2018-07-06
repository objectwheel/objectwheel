#include <controlpropertymanager.h>
#include <control.h>
#include <savemanager.h>
#include <saveutils.h>

#include <QTimer>
#include <QPointer>

// NOTE: Compressed calls occur in arbitrary order

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

void ControlPropertyManager::setX(Control* control, qreal x, bool save, bool compress, std::function<bool()> extraConditions)
{
    setGeometry(control, {{x, control->y()}, {control->size()}}, save, compress, extraConditions);
}

void ControlPropertyManager::setY(Control* control, qreal y, bool save, bool compress, std::function<bool()> extraConditions)
{
    setGeometry(control, {{control->x(), y}, control->size()}, save, compress, extraConditions);
}

void ControlPropertyManager::setPos(Control* control, const QPointF& pos, bool save, bool compress, std::function<bool()> extraConditions)
{
    setGeometry(control, {pos, control->size()}, save, compress, extraConditions);
}

void ControlPropertyManager::setSize(Control* control, const QSizeF& size, bool save, bool compress, std::function<bool()> extraConditions)
{
    setGeometry(control, {control->pos(), size}, save, compress, extraConditions);
}

void ControlPropertyManager::setGeometry(Control* control, const QRectF& geometry, bool save,
                                         bool compress, std::function<bool()> extraConditions)
{
    if (!control)
        return;

    if (!geometry.isValid())
        return;

    if (extraConditions && !extraConditions())
        return;

    if (compress) {
        s_dirtyPropertyHandlingFunctions.insert(control->uid() + "setGeometry",
                                                std::bind(&ControlPropertyManager::setGeometry,
                                                          QPointer<Control>(control), geometry,
                                                          save, false, extraConditions));

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

        emit instance()->geometryChanged(control);
    }
}

void ControlPropertyManager::setId(Control* control, const QString& id, bool save, bool compress,
                                   std::function<bool()> extraConditions)
{
    if (!control)
        return;

    if (id.isEmpty())
        return;

    if (extraConditions && !extraConditions())
        return;

    if (compress) {
        s_dirtyPropertyHandlingFunctions.insert(control->uid() + "setId",
                                                std::bind(&ControlPropertyManager::setId,
                                                          QPointer<Control>(control), id,
                                                          save, false, extraConditions));

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        const QString& previousId = control->id();

        control->setId(id);

        if (save)
            SaveManager::setProperty(control, "id", id);

        emit instance()->idChanged(control, previousId);
    }
}

void ControlPropertyManager::setParent(Control* control, Control* parentControl, bool save, bool compress,
                                       std::function<bool()> extraConditions)
{
    if (!control)
        return;

    if (!parentControl)
        return;

    if (extraConditions && !extraConditions())
        return;

    if (compress) {
        s_dirtyPropertyHandlingFunctions.insert(control->uid() + "setParent",
                                                std::bind(&ControlPropertyManager::setParent,
                                                          QPointer<Control>(control),
                                                          QPointer<Control>(parentControl),
                                                          save, false, extraConditions));

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        control->setParentItem(parentControl);

        if (save)
            SaveManager::moveControl(control, parentControl);

        emit instance()->parentChanged(control);
    }
}

void ControlPropertyManager::setZ(Control* control, qreal z, bool save, bool compress,
                                  std::function<bool()> extraConditions)
{
    if (!control)
        return;

    if (extraConditions && !extraConditions())
        return;

    if (compress) {
        s_dirtyPropertyHandlingFunctions.insert(control->uid() + "setZ",
                                                std::bind(&ControlPropertyManager::setZ,
                                                          QPointer<Control>(control), z,
                                                          save, false, extraConditions));

        if (!s_dirtyPropertyProcessingTimer->isActive())
            s_dirtyPropertyProcessingTimer->start();
    } else {
        control->setZValue(z);

        if (save)
            SaveManager::setProperty(control, "z", QString::number(control->zValue()));

        emit instance()->zChanged(control);
    }
}
