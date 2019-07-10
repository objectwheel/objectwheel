#include <headlineitem.h>
#include <designerscene.h>
#include <designerview.h>
#include <controlpropertymanager.h>

#include <QPainter>
#include <QStyleOption>

HeadlineItem::HeadlineItem(Control* parent) : QGraphicsItem(parent)
{
    setVisible(false);
    setFlag(ItemIsMovable);
    setFlag(ItemClipsToShape);
    setFlag(ItemSendsGeometryChanges);
    setFlag(ItemIgnoresTransformations);
    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(std::numeric_limits<int>::max());
}

DesignerScene* HeadlineItem::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsItem::scene());
}

Control* HeadlineItem::parentControl() const
{
    return static_cast<Control*>(parentItem());
}

QRectF HeadlineItem::boundingRect() const
{
    return m_rect;
}

void HeadlineItem::setText(const QString& text)
{
    if (m_text == text)
        return;
    m_text = text;
    updateSize();
}

void HeadlineItem::updateSize()
{
    const QSizeF ps(parentControl()->size());
    const QSizeF size(textSize());
    qreal width = size.width();
    qreal height = size.height();
    if (parentControl()->form())
        width += sizeWidth();
    prepareGeometryChange();
    m_rect = QRectF(-0.5, -height, qMin(ps.width(), width + 10), height);
    update();
}

QSizeF HeadlineItem::textSize() const
{
    const QFontMetrics fontMetrics(parentControl()->font());
    return QSizeF(fontMetrics.horizontalAdvance(m_text), fontMetrics.height());
}

QFont HeadlineItem::sizeFont() const
{
    QFont font(parentControl()->font());
    font.setPixelSize(font.pixelSize() - 4);
    return font;
}

qreal HeadlineItem::sizeWidth() const
{
    const QFontMetricsF fontMetrics(sizeFont());
    return fontMetrics.horizontalAdvance(sizeText());
}

QString HeadlineItem::sizeText() const
{
    static const QString textTemplate(QStringLiteral(" [%1×%2]"));
    return textTemplate.arg(parentControl()->size().width()).arg(parentControl()->size().height());
}

QFontMetrics HeadlineItem::sizeFontMetrics() const
{
    return QFontMetrics(sizeFont());
}

QVariant HeadlineItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged) {
        const QPointF& newPos = parentControl()->mapToParent(pos());
        if (!parentControl()->form()) {
            ControlPropertyManager::setPos(parentControl(), scene()->snapPosition(newPos),
                                           ControlPropertyManager::SaveChanges
                                           | ControlPropertyManager::UpdateRenderer
                                           | ControlPropertyManager::CompressedCall);
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void HeadlineItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mousePressEvent(event);
    if (!parentControl()->form())
        scene()->view()->viewport()->setCursor(Qt::ClosedHandCursor);
}

void HeadlineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    if (!parentControl()->form())
        scene()->view()->viewport()->unsetCursor();
}

void HeadlineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    painter->setClipRect(m_rect.adjusted(0, 0, 0, -0.5));
    if (parentControl()->form() && !parentControl()->isSelected())
        painter->fillRect(m_rect, Qt::darkGray);
    else
        painter->fillRect(m_rect, scene()->outlineColor());

    painter->setPen(Qt::white);
    QRectF rect = m_rect.adjusted(5, 0, -5, -1);
    if (parentControl()->form()) {
        rect.adjust(0, 0, -sizeWidth(), 0);
        painter->drawText(rect, m_text, QTextOption(Qt::AlignLeft));
        rect.adjust(rect.width(), 0, sizeWidth() + 1, 0);
        painter->setFont(sizeFont());
        painter->drawText(rect, sizeFontMetrics().elidedText(sizeText(), Qt::ElideRight, rect.width()),
                          QTextOption(Qt::AlignLeft | Qt::AlignBottom));
    } else {
        painter->drawText(rect, option->fontMetrics.elidedText(m_text, Qt::ElideRight, rect.width()),
                          QTextOption(Qt::AlignCenter));
    }
}
