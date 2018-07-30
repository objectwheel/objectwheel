#include <transparentstyle.h>

#include <QStyleFactory>
#include <QStyleOptionSpinBox>
#include <QPainter>
#include <qdrawutil.h>

TransparentStyle::TransparentStyle::TransparentStyle(QObject* parent)
{
    setParent(parent);
}

int TransparentStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option,
                                  const QWidget* widget) const
{
    switch (metric) {
    case PM_SpinBoxFrameWidth:
        return 0;
    default:
        return QCommonStyle::pixelMetric(metric, option, widget);
    }
}

void TransparentStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option,
                                     QPainter* painter, const QWidget* widget) const
{
    switch (element) {
    case PE_PanelLineEdit:
        break;
    default:
        QCommonStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

void TransparentStyle::drawComplexControl(QStyle::ComplexControl control,
                                          const QStyleOptionComplex* option, QPainter* painter,
                                          const QWidget* widget) const
{
    switch (control) {
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            painter->save();
            QStyleOptionSpinBox copy = *sb;
            PrimitiveElement pe;
            if (sb->subControls & SC_SpinBoxUp) {
                copy.subControls = SC_SpinBoxUp;
                QPalette pal2 = sb->palette;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepUpEnabled)) {
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                    copy.state &= ~State_Enabled;
                }
                copy.palette = pal2;
                if (sb->activeSubControls == SC_SpinBoxUp && (sb->state & State_Sunken)) {
                    copy.state |= State_On;
                    copy.state |= State_Sunken;
                } else {
                    copy.state |= State_Raised;
                    copy.state &= ~State_Sunken;
                }
                pe = (sb->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_IndicatorSpinPlus
                                                                       : PE_IndicatorSpinUp);
                copy.rect = proxy()->subControlRect(CC_SpinBox, sb, SC_SpinBoxUp, widget);
                copy.rect.adjust(1, 1, -2, 1);
                proxy()->drawPrimitive(pe, &copy, painter, widget);
            }
            if (sb->subControls & SC_SpinBoxDown) {
                copy.subControls = SC_SpinBoxDown;
                copy.state = sb->state;
                QPalette pal2 = sb->palette;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepDownEnabled)) {
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                    copy.state &= ~State_Enabled;
                }
                copy.palette = pal2;
                if (sb->activeSubControls == SC_SpinBoxDown && (sb->state & State_Sunken)) {
                    copy.state |= State_On;
                    copy.state |= State_Sunken;
                } else {
                    copy.state |= State_Raised;
                    copy.state &= ~State_Sunken;
                }
                pe = (sb->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_IndicatorSpinMinus
                                                                       : PE_IndicatorSpinDown);
                copy.rect = proxy()->subControlRect(CC_SpinBox, sb, SC_SpinBoxDown, widget);
                copy.rect.adjust(1, -1, -2, -1);
                proxy()->drawPrimitive(pe, &copy, painter, widget);
            }
            painter->restore();
        }
        break;

    default:
        QCommonStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}