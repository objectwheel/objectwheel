#include <headlineitem.h>
#include <designerscene.h>

#include <QPainter>
#include <QStyleOption>

enum { MARGIN = 10 };

HeadlineItem::HeadlineItem(DesignerItem* parent) : ToolItem(parent)
  , m_sizeUpdateScheduled(false)
{
}

QSizeF HeadlineItem::dimensions() const
{
    return m_dimensions;
}

void HeadlineItem::setDimensions(const QSizeF& dimensions)
{
    if (m_dimensions != dimensions) {
        m_dimensions = dimensions;
        updateSize();
        update();
    }
}

QString HeadlineItem::text() const
{
    return m_text;
}

void HeadlineItem::setText(const QString& text)
{
    if (m_text != text) {
        m_text = text;
        setToolTip(text);
        updateSize();
        update();
    }
}

void HeadlineItem::scheduleSizeUpdate()
{
    if (m_sizeUpdateScheduled)
        return;
    m_sizeUpdateScheduled = true;
    QMetaObject::invokeMethod(this, &HeadlineItem::updateSize, Qt::QueuedConnection);
}

void HeadlineItem::updateSize()
{
    const QSizeF& ts = calculateTextSize();
    const QSizeF& ps = parentItem() ? parentItem()->size() : QSizeF();
    qreal width = ts.width();
    if (m_dimensions.isValid()) {
        const QFontMetrics fm(dimensionsFont());
        QString wstr = QString::number(m_dimensions.width());
        QString hstr = QString::number(m_dimensions.height());
        wstr.replace(QRegularExpression("\\d"), "9");
        hstr.replace(QRegularExpression("\\d"), "9");
        width += fm.horizontalAdvance(dimensionsText(wstr.toDouble(), hstr.toDouble()));
    }
    setRect(-0.5, -ts.height(), qMin(ps.width(), width + MARGIN), ts.height());
    m_sizeUpdateScheduled = false;
}

QFont HeadlineItem::dimensionsFont() const
{
    QFont f; // App default font
    f.setPixelSize(f.pixelSize() - 3);
    return f;
}

QString HeadlineItem::dimensionsText(qreal width, qreal height) const
{
    static const QString pattern(QStringLiteral(" (%1×%2)"));
    return pattern.arg(width).arg(height);
}

QSizeF HeadlineItem::calculateTextSize() const
{
    const QFontMetrics fm(QFont{}); // App default font
    return QSizeF(fm.horizontalAdvance(m_text), fm.height());
}

void HeadlineItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    ToolItem::mouseMoveEvent(event);

    if (parentItem() && dragAccepted()) {
        scene()->setViewportCursor(Qt::ClosedHandCursor);
        parentItem()->setBeingDragged(true);
        parentItem()->setPos(parentItem()->mapToParent(mapToParent(dragDistanceVector())));
    }
}

void HeadlineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (parentItem() && dragAccepted()) {
        scene()->unsetViewportCursor();
        parentItem()->setBeingDragged(false);
    }

    ToolItem::mouseReleaseEvent(event); // Clears m_dragAccepted
}

void HeadlineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    // Background
    painter->fillRect(rect(), brush());

    // Text
    painter->setPen(pen());
    const qreal textWidth = calculateTextSize().width();
    QRectF r = rect().adjusted(MARGIN / 2, 0, -MARGIN / 2, -1);
    if (parentItem() && m_dimensions.isValid() && r.width() >= textWidth) {
        const qreal availableWidth = r.width() - textWidth;
        r.adjust(0, 0, -availableWidth, 0);
        painter->drawText(r, m_text, QTextOption(Qt::AlignLeft));
        r.adjust(r.width(), 0, availableWidth, 0);
        painter->setFont(dimensionsFont());
        const QString& text = dimensionsText(m_dimensions.width(), m_dimensions.height());
        painter->drawText(r, painter->fontMetrics().elidedText(text, Qt::ElideRight, r.width() + 1),
                          QTextOption(Qt::AlignHCenter | Qt::AlignBottom));
    } else {
        painter->drawText(r, option->fontMetrics.elidedText(m_text, Qt::ElideRight, r.width() + 1),
                          QTextOption(Qt::AlignCenter));
    }
}
