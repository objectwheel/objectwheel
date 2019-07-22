#include <form.h>
#include <designerscene.h>
#include <designersettings.h>
#include <scenesettings.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>

Form::Form(const QString& dir, Form* parent) : Control(dir, parent)
{
    setCursor(Qt::ArrowCursor);
    setFlag(ItemIsMovable, false);
}

int Form::type() const
{
    return Type;
}

void Form::paintFormFrame(QPainter* painter)
{
    painter->setPen(scene()->pen(Qt::darkGray));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(scene()->outerRect(rect()));
}

void Form::paintGridViewDots(QPainter* painter, int gridSize)
{
    painter->save();
    QVector<QPointF> points;
    for (qreal x = 0; x <= rect().right(); x += gridSize) {
        for (qreal y = 0; y <= rect().bottom(); y += gridSize)
            points.append(QPointF(x, y));
    }
    painter->setClipRect(rect());
    painter->setPen(scene()->pen("#505050", 1, false));
    painter->drawPoints(points.data(), points.size());
    painter->restore();
}

void Form::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    const SceneSettings* settings = DesignerSettings::sceneSettings();

    // Background
    painter->fillRect(rect(), settings->toBackgroundBrush());

    if (parentControl() && parentControl()->clip() && !beingDragged())
        restrainPaintRegion(painter);

    if (!image().isNull())
        paintImage(painter);

    if (settings->showGridViewDots)
        paintGridViewDots(painter, settings->gridSize);

    if (settings->controlOutline != 0)
        scene()->paintOutline(painter, scene()->outerRect(settings->controlOutline == 1 ? rect() : frame()));

    paintFormFrame(painter);

    if (settings->showMouseoverOutline && option->state & QStyle::State_MouseOver)
        paintHoverOutline(painter);

    if (dragIn())
        paintHighlight(painter);
}
