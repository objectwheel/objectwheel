#include <headlineitem.h>
#include <designerscene.h>
#include <controlpropertymanager.h>
#include <designerview.h>

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
    QSizeF ps(parentControl()->size());
    QFontMetrics fontMetrics(parentControl()->font());
    qreal height = fontMetrics.height();
    qreal width = fontMetrics.horizontalAdvance(m_text);
    if (parentControl()->form()) {
        QFont sizeFont(parentControl()->font());
        sizeFont.setPixelSize(sizeFont.pixelSize() - 3);
        fontMetrics = QFontMetrics(sizeFont);
        width += fontMetrics.horizontalAdvance(QStringLiteral(" (%1×%2)").arg(ps.width()).arg(ps.height()));
    }
    prepareGeometryChange();
    m_rect = QRectF(-0.5, -height, qMin(ps.width(), width + 10), height);
    update();
}

QVariant HeadlineItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged) {
        const QPointF& newPos = parentControl()->mapToParent(pos());
        if (!parentControl()->form()) {
            ControlPropertyManager::setPos(parentControl(), newPos, ControlPropertyManager::SaveChanges
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
    if (parentControl()->form() && !parentControl()->isSelected()) {
        painter->fillRect(m_rect, Qt::darkGray);
    } else {
        painter->fillRect(m_rect, scene()->outlineColor());
    }

    painter->setPen(Qt::white);
    QRectF rect = m_rect.adjusted(5, 0, -5, -1);
    if (parentControl()->form()) {
        QSizeF ps(parentControl()->size());
        QString sizeText = QStringLiteral(" (%1×%2)").arg(ps.width()).arg(ps.height());
        QFont sizeFont(parentControl()->font());
        sizeFont.setPixelSize(sizeFont.pixelSize() - 3);
        QFontMetrics fontMetrics(sizeFont);
        qreal sizeWidth = fontMetrics.horizontalAdvance(sizeText);
        rect.adjust(0, 0, -sizeWidth, 0);
        painter->drawText(rect, m_text, QTextOption(Qt::AlignLeft));
        rect.adjust(rect.width(), 0, sizeWidth, 0);
        painter->setFont(sizeFont);
        painter->drawText(rect, fontMetrics.elidedText(sizeText, Qt::ElideRight, rect.width() + 1),
                          QTextOption(Qt::AlignLeft | Qt::AlignBottom));
    } else {
        painter->drawText(rect, option->fontMetrics.elidedText(m_text, Qt::ElideRight, rect.width() + 1),
                          QTextOption(Qt::AlignCenter));
    }
}
