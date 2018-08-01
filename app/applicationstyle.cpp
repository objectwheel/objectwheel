#include <applicationstyle.h>

#include <QStyleFactory>
#include <QApplication>
#include <QPainter>
#include <QStyleOption>

ApplicationStyle::ApplicationStyle() : QProxyStyle("fusion")
{
    Q_ASSERT(QStyleFactory::keys().contains("fusion", Qt::CaseInsensitive));
}

int ApplicationStyle::styleHint(QStyle::StyleHint hint, const QStyleOption* option,
                                const QWidget* widget, QStyleHintReturn* returnData) const
{
    switch (hint) {
    case SH_ToolTipLabel_Opacity:
        return 242; // About 95%
    default:
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
}

int ApplicationStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    switch (metric) {
    case PM_ToolTipLabelFrameWidth:
        return 1;
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

void ApplicationStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    switch (element) {
    case PE_Frame: {
        painter->save();
        painter->setPen(option->palette.base().color().darker(140));
        painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
        painter->setPen(option->palette.base().color().darker(180));
        painter->drawLine(option->rect.topLeft(), option->rect.topRight());
        painter->restore();
    } break;
    case PE_PanelTipLabel:
        painter->fillRect(option->rect, option->palette.brush(QPalette::ToolTipBase));
        break;
    default:
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}
