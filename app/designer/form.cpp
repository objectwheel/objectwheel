#include <form.h>
#include <designerscene.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

Form::Form(const QString& dir, Form* parent) : Control(dir, parent)
{
    setFlag(ItemIsMovable, false);
}

int Form::type() const
{
    return Type;
}

QRectF Form::frameGeometry() const
{
    return QRectF(QPointF(-size().width() / 2.0, -size().height() / 2.0), size());
}

void Form::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    Control::resizeEvent(event);
    scene()->centralize();
}

void Form::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // We don't propagate the mouse press event in order to block selection on form.
    // Hence rubber band is activated on form.
    event->ignore();
}

void Form::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Control::paint(painter, option, widget);


    QVector<QPointF> points;
    for (qreal x = 0; x < rect().right(); x += scene()->gridSize()) {
        for (qreal y = 0; y < rect().bottom(); y += scene()->gridSize())
            points.append(QPointF(x, y));
    }
    painter->setBrush(Qt::NoBrush);
    painter->setPen("#222222");
    painter->drawPoints(points.data(), points.size());

    painter->setPen(isSelected() ? "#4BA2FF" : "#A4A4A4");
    painter->drawRect(rect());

    QString text(QStringLiteral("%1 (%2×%3)").arg(id()).arg(size().width()).arg(size().width()));
    QFontMetricsF fm(painter->font());
    QRectF titleRect(0, 0, fm.horizontalAdvance(text) + 2 * fm.height(), fm.height());
    titleRect.moveCenter(rect().center());
    titleRect.moveTop(- fm.height() - 5);
    painter->setBrush(QBrush("#4BA2FF"));
    painter->setPen("#4391e5");
    painter->drawRoundedRect(titleRect, fm.height() / 2.0, fm.height() / 2.0);
    titleRect.adjust(0, -1, 0, -1);
    painter->setPen("#FFFFFF");
    painter->drawText(titleRect, text, QTextOption(Qt::AlignCenter));
}
