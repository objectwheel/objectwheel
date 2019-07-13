#include <headlineitem.h>
#include <designerscene.h>

#include <QPainter>
#include <QStyleOption>

enum { MARGIN = 10 };

HeadlineItem::HeadlineItem(DesignerItem* parent) : DesignerItem(parent)
  , m_showDimensions(false)
{
    setFlag(ItemIgnoresTransformations);
    setZValue(std::numeric_limits<int>::max());
}

void HeadlineItem::setText(const QString& text)
{
    if (m_text != text) {
        m_text = text;
        setToolTip(text);
        updateSize();
    }
}

bool HeadlineItem::showDimensions() const
{
    return m_showDimensions;
}

void HeadlineItem::setShowDimensions(bool showDimensions)
{
    m_showDimensions = showDimensions;
    updateSize();
}

void HeadlineItem::updateSize()
{
    QSizeF ps;
    const QSizeF& ts = calculateTextSize();
    qreal width = ts.width();
    if (parentItem() && showDimensions()) {
        const QFontMetrics fm(dimensionTextFont());
        ps = parentItem()->size();
        QString wstr = QString::number(ps.width());
        QString hstr = QString::number(ps.height());
        wstr.replace(QRegularExpression("\\d"), "9");
        hstr.replace(QRegularExpression("\\d"), "9");
        width += fm.horizontalAdvance(dimensionText(wstr.toDouble(), hstr.toDouble()));
    }
    setRect(-0.5, -ts.height(), qMin(ps.width(), width + MARGIN), ts.height());
}

QSizeF HeadlineItem::calculateTextSize() const
{
    const QFontMetrics fontMetrics(parentItem() ? parentItem()->font() : QFont());
    return QSizeF(fontMetrics.horizontalAdvance(m_text), fontMetrics.height());
}

QFont HeadlineItem::dimensionTextFont() const
{
    QFont font(parentItem() ? parentItem()->font() : QFont());
    font.setPixelSize(font.pixelSize() - 3);
    return font;
}

QString HeadlineItem::dimensionText(qreal width, qreal height) const
{
    static const QString pattern(QStringLiteral(" (%1×%2)"));
    return pattern.arg(width).arg(height);
}

void HeadlineItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mouseMoveEvent(event);

    if (parentItem() && dragDistanceExceeded()) {
        scene()->setViewportCursor(Qt::ClosedHandCursor);
        parentItem()->setBeingDragged(true);
        parentItem()->setPos(parentItem()->mapToParent(mapToParent(dragDistance())));
    }
}

void HeadlineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (parentItem() && dragDistanceExceeded()) {
        scene()->unsetViewportCursor();
        parentItem()->setBeingDragged(false);
    }

    DesignerItem::mouseReleaseEvent(event); // Clears m_dragDistanceExceeded
}

void HeadlineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    // Background
    painter->fillRect(rect(), brush());

    // Text
    painter->setPen(pen());
    const qreal textWidth = calculateTextSize().width();
    QRectF r = rect().adjusted(MARGIN / 2, 0, -MARGIN / 2, -1);
    if (parentItem() && m_showDimensions && r.width() >= textWidth) {
        const qreal availableWidth = r.width() - textWidth;
        r.adjust(0, 0, -availableWidth, 0);
        painter->drawText(r, m_text, QTextOption(Qt::AlignLeft));
        r.adjust(r.width(), 0, availableWidth, 0);
        painter->setFont(dimensionTextFont());
        const QString& text = dimensionText(parentItem()->width(), parentItem()->height());
        painter->drawText(r, painter->fontMetrics().elidedText(text, Qt::ElideRight, r.width() + 1),
                          QTextOption(Qt::AlignHCenter | Qt::AlignBottom));
    } else {
        painter->drawText(r, option->fontMetrics.elidedText(m_text, Qt::ElideRight, r.width() + 1),
                          QTextOption(Qt::AlignCenter));
    }
}
