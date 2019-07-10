#include <headlineitem.h>
#include <designerscene.h>
#include <controlpropertymanager.h>
#include <utilityfunctions.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>

enum { MARGIN = 10 };

HeadlineItem::HeadlineItem(Control* parent) : QGraphicsObject(parent)
{
    setVisible(false);
    setFlag(ItemClipsToShape);
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
    setToolTip(text);
    updateSize();
}

void HeadlineItem::updateSize()
{
    const QSizeF ps(parentControl()->size());
    const QSizeF size(textSize());
    qreal width = size.width();
    qreal height = size.height();
    if (parentControl()->form())
        width += dimensionTextWidth();
    prepareGeometryChange();
    m_rect = QRectF(-0.5, -height, qMin(ps.width(), width + MARGIN), height);
    update();
}

QSizeF HeadlineItem::textSize() const
{
    const QFontMetrics fontMetrics(parentControl()->font());
    return QSizeF(fontMetrics.horizontalAdvance(m_text), fontMetrics.height());
}

QString HeadlineItem::dimensionText() const
{
    static const QString textTemplate(QStringLiteral(" (%1×%2)"));
    return textTemplate.arg(parentControl()->size().width()).arg(parentControl()->size().height());
}

QFont HeadlineItem::dimensionTextFont() const
{
    QFont font(parentControl()->font());
    font.setPixelSize(font.pixelSize() - 3);
    return font;
}

qreal HeadlineItem::dimensionTextWidth() const
{
    const QFontMetricsF fontMetrics(dimensionTextFont());
    int digits = 0;
    digits += UtilityFunctions::numberOfDigits(parentControl()->size().width());
    digits += UtilityFunctions::numberOfDigits(parentControl()->size().height());
    return fontMetrics.horizontalAdvance(QStringLiteral(" (×)")) +
            fontMetrics.horizontalAdvance('9') * digits;
}

QFontMetrics HeadlineItem::dimensionTextFontMetrics() const
{
    return QFontMetrics(dimensionTextFont());
}

void HeadlineItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    event->ignore();
    emit doubleClicked();
}

void HeadlineItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    m_dragStarted = false;
    m_dragStartPoint = event->pos();
}

void HeadlineItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    const QPointF& diff = event->pos() - m_dragStartPoint;
    if (m_dragStarted || diff.manhattanLength() >= QApplication::startDragDistance()) {
        m_dragStarted = true;
        scene()->setViewportCursor(Qt::ClosedHandCursor);
        const QPointF& snapPos = scene()->snapPosition(parentControl()->mapToParent(diff));
        ControlPropertyManager::setPos(parentControl(), snapPos, ControlPropertyManager::SaveChanges
                                       | ControlPropertyManager::UpdateRenderer
                                       | ControlPropertyManager::CompressedCall);
    }
}

void HeadlineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
{
    if (m_dragStarted)
        scene()->unsetViewportCursor();
}

void HeadlineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    // Background
    bool isDark = parentControl()->form() && !parentControl()->isSelected();
    painter->fillRect(m_rect, isDark ? Qt::darkGray : scene()->outlineColor());

    // Text
    painter->setPen(Qt::white);
    qreal textWidth = textSize().width();
    QFontMetrics fm = option->fontMetrics;
    QRectF rect = m_rect.adjusted(MARGIN / 2, 0, -MARGIN / 2, -1);
    if (parentControl()->form() && rect.width() >= textWidth) {
        qreal availableWidth = rect.width() - textWidth;
        rect.adjust(0, 0, -availableWidth, 0);
        painter->drawText(rect, m_text, {Qt::AlignLeft});
        rect.adjust(rect.width(), 0, availableWidth, 0);
        painter->setFont(dimensionTextFont());
        fm = dimensionTextFontMetrics();
        painter->drawText(rect, fm.elidedText(dimensionText(), Qt::ElideRight, rect.width() + 1),
                          QTextOption(Qt::AlignHCenter | Qt::AlignBottom));
    } else {
        painter->drawText(rect, fm.elidedText(m_text, Qt::ElideRight, rect.width() + 1),
                          QTextOption(Qt::AlignCenter));
    }
}
