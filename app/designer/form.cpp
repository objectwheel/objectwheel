#include <form.h>
#include <designerscene.h>

#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>

Form::Form(Control* parent) : Control(parent)
{
    setCursor(Qt::ArrowCursor);
    setFlag(ItemIsMovable, false);
}

int Form::type() const
{
    return Type;
}

void Form::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Control::mousePressEvent(event);

    if (event->button() == Qt::LeftButton && (flags() & ItemIsSelectable)) {
        bool select = true;
        if (event->modifiers() & Qt::ControlModifier) // Multiple-selection
            select = !isSelected();
        else if (!isSelected() && scene())
            scene()->clearSelection();
        QMetaObject::invokeMethod(this, std::bind(&Form::setSelected, this, select), Qt::QueuedConnection);
    }
    // We block the event, thus it can go up to QGraphicsView
    // and draw rubber band, but the view deselects the form
    // So we also make QueuedConnection call to the setSelected
    // In this way, the setSelected function is called after view
    // handles the mousePressEvent and clears selection
    event->ignore();
}

void Form::paintBackground(QPainter* painter)
{
    painter->fillRect(rect(), DesignerScene::backgroundTexture());
}

void Form::paintForeground(QPainter* painter)
{
    // Draw grid view dots
    QVector<QPointF> points;
    for (qreal x = 0; x <= rect().right(); x += DesignerScene::gridSize()) {
        for (qreal y = 0; y <= rect().bottom(); y += DesignerScene::gridSize())
            points.append(QPointF(x, y));
    }
    painter->setBrush(Qt::NoBrush);
    painter->setPen(DesignerScene::pen(Qt::darkGray, 1, false));
    painter->drawPoints(points.data(), points.size());

    // Draw form frame
    painter->setPen(DesignerScene::pen(Qt::darkGray));
    painter->drawRect(DesignerScene::outerRect(rect()));
}

void Form::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    paintBackground(painter);
    Control::paint(painter, option, widget);
    paintForeground(painter);
}
