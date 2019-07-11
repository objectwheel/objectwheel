#include <headlineitem.h>
#include <designerscene.h>
#include <controlpropertymanager.h>
#include <utilityfunctions.h>

#include <QPainter>
#include <QStyleOption>

enum { MARGIN = 10 };

HeadlineItem::HeadlineItem(Control* parent) : DesignerItem(parent)
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

void HeadlineItem::updateSize()
{
    const QSizeF& ts = textSize();
    const QSizeF& ps = parentControl()->size();
    qreal width = ts.width();
    if (parentControl()->form()) {
        using namespace UtilityFunctions;
        const QFontMetrics fm(dimensionTextFont());
        width += fm.horizontalAdvance(dimensionText(nine(digits(ps.width())), nine(digits(ps.height()))));
    }
    prepareGeometryChange();
    m_rect = QRectF(-0.5, -ts.height(), qMin(ps.width(), width + MARGIN), ts.height());
    update();
}

QSizeF HeadlineItem::textSize() const
{
    const QFontMetrics fontMetrics(parentControl()->font());
    return QSizeF(fontMetrics.horizontalAdvance(m_text), fontMetrics.height());
}

QString HeadlineItem::dimensionText(int width, int height) const
{
    static const QString pattern(QStringLiteral(" (%1×%2)"));
    return pattern.arg(width).arg(height);
}

QFont HeadlineItem::dimensionTextFont() const
{
    QFont font(parentControl()->font());
    font.setPixelSize(font.pixelSize() - 3);
    return font;
}

void HeadlineItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    DesignerItem::mouseMoveEvent(event);

    if (dragStarted()) {
        scene()->setViewportCursor(Qt::ClosedHandCursor);
        ControlPropertyManager::setPos(parentControl(), snapPosition(),
                                       ControlPropertyManager::SaveChanges
                                       | ControlPropertyManager::UpdateRenderer
                                       | ControlPropertyManager::CompressedCall);
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
    qreal textWidth = textSize().width();
    QFontMetrics fm = option->fontMetrics;
    QRectF rect = m_rect.adjusted(MARGIN / 2, 0, -MARGIN / 2, -1);
    if (parentControl()->form() && rect.width() >= textWidth) {
        const QSizeF& ps = parentControl()->size();
        qreal availableWidth = rect.width() - textWidth;
        rect.adjust(0, 0, -availableWidth, 0);
        painter->drawText(rect, m_text, {Qt::AlignLeft});
        rect.adjust(rect.width(), 0, availableWidth, 0);
        painter->setFont(dimensionTextFont());
        fm = QFontMetrics(dimensionTextFont());
        painter->drawText(rect, fm.elidedText(dimensionText(ps.width(), ps.height()), Qt::ElideRight, rect.width() + 1),
                          QTextOption(Qt::AlignHCenter | Qt::AlignBottom));
    } else {
        painter->drawText(rect, fm.elidedText(m_text, Qt::ElideRight, rect.width() + 1),
                          QTextOption(Qt::AlignCenter));
    }
}
