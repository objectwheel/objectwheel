#include <form.h>
#include <designerscene.h>
#include <designersettings.h>
#include <scenesettings.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>

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

void Form::paintFrame(QPainter* painter)
{
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(scene()->pen(Qt::darkGray));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(outerRect(rect()));
    painter->setRenderHint(QPainter::Antialiasing, true);
}

void Form::paintHeadline(QPainter* painter)
{
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

void Form::paintGridViewDots(QPainter* painter, int gridSize)
{
    QVector<QPointF> points;
    for (qreal x = 0; x < rect().right(); x += gridSize) {
        for (qreal y = 0; y < rect().bottom(); y += gridSize)
            points.append(QPointF(x, y));
    }
    painter->setPen(scene()->pen("#505050", 1, false));
    painter->drawPoints(points.data(), points.size());
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

void Form::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    const SceneSettings* settings = DesignerSettings::sceneSettings();

    // Background
    painter->fillRect(rect(), settings->toBackgroundBrush());

    if (parentControl() && parentControl()->clip() && !dragging())
        restrainPaintRegion(painter);

    if (!image().isNull())
        paintImage(painter);

    if (settings->showGridViewDots)
        paintGridViewDots(painter, settings->gridSize);

    if (settings->controlOutline != 0)
        paintOutline(painter, settings->controlOutline);

    paintFrame(painter);

    if (settings->showMouseoverOutline && option->state & QStyle::State_MouseOver)
        paintHoverOutline(painter);

    if (isSelected())
        paintSelectionOutline(painter);

    if (dragIn())
        paintHighlight(painter);

    paintHeadline(painter);
}
