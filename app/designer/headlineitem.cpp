#include <headlineitem.h>
#include <designerscene.h>
#include <controlpropertymanager.h>

#include <QPainter>
#include <QStyleOption>

enum { MARGIN = 10 };

HeadlineItem::HeadlineItem(Control* parent) : DesignerItem(parent)
  , m_showDimensions(false)
{
}

QRectF HeadlineItem::boundingRect() const
{
    return m_rect;
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
    const QSizeF& ts = calculateTextSize();
    const QSizeF& ps = parentControl()->size();
    qreal width = ts.width();
    if (m_showDimensions) {
        const QFontMetrics fm(dimensionTextFont());
        QString wstr = QString::number(ps.width());
        QString hstr = QString::number(ps.height());
        wstr.replace(QRegularExpression("\\d"), "9");
        hstr.replace(QRegularExpression("\\d"), "9");
        width += fm.horizontalAdvance(dimensionText(wstr.toDouble(), hstr.toDouble()));
    }
    prepareGeometryChange();
    m_rect = QRectF(-0.5, -ts.height(), qMin(ps.width(), width + MARGIN), ts.height());
    update();
}

QSizeF HeadlineItem::calculateTextSize() const
{
    const QFontMetrics fontMetrics(parentControl()->font());
    return QSizeF(fontMetrics.horizontalAdvance(m_text), fontMetrics.height());
}

QFont HeadlineItem::dimensionTextFont() const
{
    QFont font(parentControl()->font());
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
    if (dragStarted()) {
        scene()->setViewportCursor(Qt::ClosedHandCursor);
        ControlPropertyManager::Options option = parentControl()->form()
                ? ControlPropertyManager::NoOption
                : ControlPropertyManager::SaveChanges
                  | ControlPropertyManager::UpdateRenderer
                  | ControlPropertyManager::CompressedCall;
        ControlPropertyManager::setPos(parentControl(), snapPosition(), option);
    }
}

void HeadlineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (dragStarted())
        scene()->unsetViewportCursor();
    DesignerItem::mouseReleaseEvent(event);
}

void HeadlineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    // Background
    painter->fillRect(m_rect, brush());

    // Text
    painter->setPen(pen());
    const qreal textWidth = calculateTextSize().width();
    QRectF rect = m_rect.adjusted(MARGIN / 2, 0, -MARGIN / 2, -1);
    if (m_showDimensions && rect.width() >= textWidth) {
        const qreal availableWidth = rect.width() - textWidth;
        rect.adjust(0, 0, -availableWidth, 0);
        painter->drawText(rect, m_text, QTextOption(Qt::AlignLeft));
        rect.adjust(rect.width(), 0, availableWidth, 0);
        painter->setFont(dimensionTextFont());
        const QSizeF& ps = parentControl()->size();
        const QString& elidedText = painter->fontMetrics().elidedText(dimensionText(ps.width(), ps.height()),
                                                                      Qt::ElideRight, rect.width() + 1);
        painter->drawText(rect, elidedText, QTextOption(Qt::AlignHCenter | Qt::AlignBottom));
    } else {
        painter->drawText(rect, option->fontMetrics.elidedText(m_text, Qt::ElideRight, rect.width() + 1),
                          QTextOption(Qt::AlignCenter));
    }
}
