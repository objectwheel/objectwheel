#include <headlineitem.h>
#include <designerscene.h>

#include <QPainter>
#include <QStyleOption>
#include <QGraphicsSceneMouseEvent>

enum { MARGIN = 10 }; // Left + Right

HeadlineItem::HeadlineItem(DesignerItem* parent) : GadgetItem(parent)
  , m_geometryUpdateScheduled(false)
{
}

int HeadlineItem::type() const
{
    return Type;
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
        updateGeometry();
        update();
    }
}

QSizeF HeadlineItem::dimensions() const
{
    return m_dimensions;
}

void HeadlineItem::setDimensions(const QSizeF& dimensions)
{
    if (m_dimensions != dimensions) {
        m_dimensions = dimensions;
        updateGeometry();
        update();
    }
}

DesignerItem* HeadlineItem::targetItem() const
{
    return m_targetItem;
}

void HeadlineItem::setTargetItem(DesignerItem* targetItem)
{
    if (m_targetItem != targetItem) {
        m_targetItem = targetItem;
        updateGeometry();
        update();
    }
}

void HeadlineItem::updateGeometry(bool callLater)
{
    if (callLater) {
        if (m_geometryUpdateScheduled)
            return;
        m_geometryUpdateScheduled = true;
        QMetaObject::invokeMethod(this, &HeadlineItem::updateGeometryNow, Qt::QueuedConnection);
    } else {
        updateGeometryNow();
    }
}

void HeadlineItem::updateGeometryNow()
{
    const QSizeF& ts = calculateTextSize();
    const QSizeF& ps = targetItem() ? targetItem()->size() : QSizeF();
    qreal width = ts.width();
    if (m_dimensions.isValid()) {
        const QFontMetrics fm(dimensionsFont());
        QString wstr = QString::number(m_dimensions.width());
        QString hstr = QString::number(m_dimensions.height());
        wstr.replace(QRegularExpression("\\d"), "9");
        hstr.replace(QRegularExpression("\\d"), "9");
        width += fm.horizontalAdvance(dimensionsText(wstr.toDouble(), hstr.toDouble()));
    }
    setPos(targetItem() ? targetItem()->sceneBoundingRect().topLeft() : QPointF());
    setRect(-0.5, -ts.height(), qMin(ps.width(), width + MARGIN), ts.height());
    m_geometryUpdateScheduled = false;
}

QFont HeadlineItem::dimensionsFont() const
{
    QFont font; // App default font
    font.setPixelSize(font.pixelSize() - 3);
    return font;
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

void HeadlineItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    GadgetItem::mousePressEvent(event);

    if (event->button() == Qt::LeftButton && targetItem() && (targetItem()->flags() & ItemIsSelectable)) {
        if (event->modifiers() & Qt::ControlModifier) { // Multiple-selection
            targetItem()->setSelected(!targetItem()->isSelected());
        } else if (!targetItem()->isSelected()) {
            if (scene())
                scene()->clearSelection();
            targetItem()->setSelected(true);
        }
    }
}

void HeadlineItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    GadgetItem::mouseMoveEvent(event);

    if (dragAccepted() && targetItem()) {
        if (!targetItem()->raised()) {
            if (scene()) {
                scene()->setCursor(Qt::ClosedHandCursor);
                scene()->prepareDragLayer(targetItem());
            }
            targetItem()->setBeingDragged(true);
            targetItem()->setRaised(true);
        }
        targetItem()->setPos(targetItem()->pos() + dragDistanceVector());
    }
}

void HeadlineItem::mouseUngrabEvent(QEvent* event)
{
    if (dragAccepted() && targetItem() && targetItem()->raised()) {
        if (scene())
            scene()->unsetCursor();
        targetItem()->setRaised(false);
        targetItem()->setBeingDragged(false);
    }

    GadgetItem::mouseUngrabEvent(event); // Clears dragAccepted state
}

void HeadlineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    // Background
    painter->fillRect(rect(), brush());

    // Text
    painter->setPen(pen());
    const qreal textWidth = calculateTextSize().width();
    QRectF r = rect().adjusted(MARGIN / 2, 0, -MARGIN / 2, -1);
    if (m_dimensions.isValid() && r.width() >= textWidth) {
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
