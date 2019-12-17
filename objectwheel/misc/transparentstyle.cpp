#include <transparentstyle.h>
#include <qdrawutil.h>
#include <utilityfunctions.h>

#include <private/qstylehelper_p.h>

#include <QPainter>
#include <QMainWindow>
#include <QComboBox>
#include <QAbstractScrollArea>

namespace {
const int PushButtonLeftOffset  = 6;
const int PushButtonRightOffset = 12;
const qreal pushButtonDefaultHeight[3] = { 32, 28, 16 };
}

TransparentStyle::TransparentStyle::TransparentStyle(QObject* parent) : ApplicationStyle()
{
    setParent(parent);
}

// In order to eliminate styleSheet override; this needs to be called before setStyleSheet call
void TransparentStyle::attach(QWidget* widget)
{
    static auto style = new TransparentStyle(QCoreApplication::instance());

    const QList<QWidget*>& childs = widget->findChildren<QWidget*>();
    QSet<QWidget*> widgetList;
    widgetList.insert(widget);
    widgetList.unite(QSet<QWidget*>(childs.begin(), childs.end()));

    for (QWidget* w : widgetList) {
        w->setStyleSheet(QString());
        w->setStyle(style);
    }
}

QStyle::SubControl TransparentStyle::hitTestComplexControl(ComplexControl control,
                                                           const QStyleOptionComplex *option,
                                                           const QPoint& point, const QWidget *widget) const
{
    SubControl sc = QStyle::SC_None;

    switch (control) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb
                = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            sc = ApplicationStyle::hitTestComplexControl(control, cmb, point, widget);
            if (!cmb->editable && sc != QStyle::SC_None)
                sc = SC_ComboBoxArrow;  // A bit of a lie, but what we want
        } break;
    default:
        sc = ApplicationStyle::hitTestComplexControl(control, option, point, widget);
        break;
    }

    return sc;
}

QSize TransparentStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption* option,
                                         const QSize& contentsSize, const QWidget* widget) const
{
    QSize sz(contentsSize);

    switch (type) {
    case CT_ComboBox:
        if (const auto *cb = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            if (!cb->editable) {
                // Same as CT_PushButton, because we have to fit the focus
                // ring and a non-editable combo box is a NSPopUpButton.
                sz.rwidth() += PushButtonLeftOffset + PushButtonRightOffset + 12;
                // All values as measured from HIThemeGetButtonBackgroundBounds()
                sz.rwidth() += 12; // We like 12 over here.
            } else {
                sz.rwidth() += 50; // FIXME Double check this
            }
            // This should be enough to fit the focus ring
            sz.setHeight(pushButtonDefaultHeight[1]);
            sz.setWidth(sz.width() + widget->contentsMargins().left() + widget->contentsMargins().right());
            return sz;
        } break;
    case CT_ScrollBar :
        // Make sure that the scroll bar is large enough to display the thumb indicator.
        if (const QStyleOptionSlider *slider
                = qstyleoption_cast<const QStyleOptionSlider*>(option)) {
            const int minimumSize = 24; // Smallest knob size, but Cocoa doesn't seem to care
            if (slider->orientation == Qt::Horizontal)
                sz = sz.expandedTo(QSize(minimumSize, sz.height()));
            else
                sz = sz.expandedTo(QSize(sz.width(), minimumSize));
        } break;
    default:
        sz = ApplicationStyle::sizeFromContents(type, option, contentsSize, widget);
        break;
    }

    return sz;
}

QRect TransparentStyle::subControlRect(QStyle::ComplexControl control,
                                       const QStyleOptionComplex* option,
                                       QStyle::SubControl subControl, const QWidget* widget) const
{
    QRect ret = ApplicationStyle::subControlRect(control, option, subControl, widget);

    switch (control) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox* combo
                = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            Q_UNUSED(combo)
            if (subControl == SC_ComboBoxEditField)
                ret.adjust(-3, 0, 8, 0);
            else if (subControl == SC_ComboBoxArrow)
                ret = QFusionStyle::subControlRect(control, option, subControl, widget);
        } break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox* spinbox
                = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            Q_UNUSED(spinbox)
            if (subControl == SC_SpinBoxEditField)
                ret.adjust(-3, 0, 0, 0);
        } break;
    default:
        break;
    }

    return ret;
}

int TransparentStyle::styleHint(QStyle::StyleHint hint, const QStyleOption* option,
                                const QWidget* widget, QStyleHintReturn* returnData) const
{
    switch (hint) {
    case SH_ToolBar_Movable:
        return false;
    case SH_ToolButtonStyle:
        return Qt::ToolButtonIconOnly;
    default:
        return ApplicationStyle::styleHint(hint, option, widget, returnData);
    }
}

int TransparentStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option,
                                  const QWidget* widget) const
{
    switch (metric) {
    case PM_ToolBarHandleExtent:
        return 0;
    case PM_ToolBarItemSpacing:
        return 0;
    case PM_ToolBarFrameWidth:
    case PM_ToolBarItemMargin:
        return 0;
    case PM_ToolBarIconSize:
        return 16;
    case PM_ComboBoxFrameWidth:
    case PM_SpinBoxFrameWidth:
        return 0;
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutVerticalSpacing:
        return -1;
    case PM_SmallIconSize:
        return 16;
    case PM_LargeIconSize:
        return 32;
    case PM_IconViewIconSize:
        return proxy()->pixelMetric(PM_LargeIconSize, option, widget);
    case PM_ScrollBarSliderMin:
        return 24;
    case PM_DefaultFrameWidth:
        if (widget && (widget->isWindow() || !widget->parentWidget()
                       || (qobject_cast<const QMainWindow*>(widget->parentWidget())
                           && static_cast<QMainWindow*>(widget->parentWidget())->centralWidget() == widget))
                && qobject_cast<const QAbstractScrollArea*>(widget)) {
            return 0;
        } else {
            // The combo box popup has no frame.
            if (qstyleoption_cast<const QStyleOptionComboBox*>(option) != 0)
                return 0;
            else
                return 1;
        }
    case PM_LayoutLeftMargin:
    case PM_LayoutTopMargin:
    case PM_LayoutRightMargin:
    case PM_LayoutBottomMargin:
    {
        bool isWindow = false;
        if (option) {
            isWindow = (option->state & State_Window);
        } else if (widget) {
            isWindow = widget->isWindow();
        }
        if (isWindow) {
            /*
                AHIG would have (20, 8, 10) here but that makes
                no sense. It would also have 14 for the top margin
                but this contradicts both Builder and most
                applications.
            */
            return 10;
        } else {
            // hack to detect QTabWidget
            if (widget && widget->parentWidget()
                    && widget->parentWidget()->sizePolicy().controlType() == QSizePolicy::TabWidget) {
                if (metric == PM_LayoutTopMargin) {
                    /*
                        Builder would have 14 (= 20 - 6) instead of 12,
                        but that makes the tab look disproportionate.
                    */
                    return 6;
                } else {
                    return 8;
                }
            } else {
                /*
                    Child margins are highly inconsistent in AHIG and Builder.
                */
                return 8;
            }
        }
    }
    default:
        return ApplicationStyle::pixelMetric(metric, option, widget);
    }
}

void TransparentStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option,
                                     QPainter* painter, const QWidget* widget) const
{
    switch (element) {
    case PE_PanelLineEdit:
        break; // Skip for transparent spinbox line edit
    case PE_PanelButtonTool:
        if ((option->state & State_Enabled || option->state & State_On) || !(option->state & State_AutoRaise)) {
            painter->save();
            if (widget && widget->inherits("QDockWidgetTitleButton")) {
                if (option->state & State_MouseOver)
                    proxy()->drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
            } else {
                proxy()->drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
            }
            painter->restore();
        } break;
    case PE_PanelButtonCommand: {
        painter->save();
        bool isDown = (option->state & State_Sunken) || (option->state & State_On);
        bool isEnabled = option->state & State_Enabled;
        if (isEnabled)
            painter->fillRect(option->rect, isDown ? "#30000000" : "#15000000");
        painter->restore();
    } break;
    default:
        ApplicationStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

void TransparentStyle::drawComplexControl(QStyle::ComplexControl control,
                                          const QStyleOptionComplex* option, QPainter* painter,
                                          const QWidget* widget) const
{
    switch (control) {
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *so
                = qstyleoption_cast<const QStyleOptionSpinBox*>(option)) {
            painter->save();
            QStyleOptionSpinBox copy = *so;
            PrimitiveElement pe;
            if (so->subControls & SC_SpinBoxUp) {
                copy.subControls = SC_SpinBoxUp;
                QPalette pal2 = so->palette;
                QColor arrowColor = pal2.windowText().color();
                if (so->stepEnabled & QAbstractSpinBox::StepUpEnabled)
                    arrowColor.setAlpha(160);
                else
                    arrowColor.setAlpha(60);
                pal2.setColor(QPalette::ButtonText, arrowColor);
                if (!(so->stepEnabled & QAbstractSpinBox::StepUpEnabled)) {
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                    copy.state &= ~State_Enabled;
                }
                copy.palette = pal2;
                if (so->activeSubControls == SC_SpinBoxUp && (so->state & State_Sunken)) {
                    copy.state |= State_On;
                    copy.state |= State_Sunken;
                } else {
                    copy.state |= State_Raised;
                    copy.state &= ~State_Sunken;
                }
                pe = (so->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_IndicatorSpinPlus
                                                                       : PE_IndicatorSpinUp);
                copy.rect = proxy()->subControlRect(CC_SpinBox, so, SC_SpinBoxUp, widget);
                ApplicationStyle::drawPrimitive(pe, &copy, painter, widget);
            }
            if (so->subControls & SC_SpinBoxDown) {
                copy.subControls = SC_SpinBoxDown;
                copy.state = so->state;
                QPalette pal2 = so->palette;
                QColor arrowColor = pal2.windowText().color();
                if (so->stepEnabled & QAbstractSpinBox::StepDownEnabled)
                    arrowColor.setAlpha(160);
                else
                    arrowColor.setAlpha(60);
                pal2.setColor(QPalette::ButtonText, arrowColor);
                if (!(so->stepEnabled & QAbstractSpinBox::StepDownEnabled)) {
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                    copy.state &= ~State_Enabled;
                }
                copy.palette = pal2;
                if (so->activeSubControls == SC_SpinBoxDown && (so->state & State_Sunken)) {
                    copy.state |= State_On;
                    copy.state |= State_Sunken;
                } else {
                    copy.state |= State_Raised;
                    copy.state &= ~State_Sunken;
                }
                pe = (so->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_IndicatorSpinMinus
                                                                       : PE_IndicatorSpinDown);
                copy.rect = proxy()->subControlRect(CC_SpinBox, so, SC_SpinBoxDown, widget);
                ApplicationStyle::drawPrimitive(pe, &copy, painter, widget);
            }
            painter->restore();
        } break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb
                = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            painter->save();
            if ((cmb->subControls & SC_ComboBoxFrame) && UtilityFunctions::hasHover(widget))
                painter->fillRect(option->rect, "#15000000");
            if (cmb->subControls & SC_ComboBoxArrow) {
                State flags = State_None;
                QRect ar = proxy()->subControlRect(CC_ComboBox, cmb, SC_ComboBoxArrow, widget);
                bool sunkenArrow = cmb->activeSubControls == SC_ComboBoxArrow
                        && cmb->state & State_Sunken;
                ar.adjust(2, 2, -2, -2);
                if (option->state & State_Enabled)
                    flags |= State_Enabled;
                if (option->state & State_HasFocus)
                    flags |= State_HasFocus;
                if (sunkenArrow)
                    flags |= State_Sunken;
                QStyleOption arrowOpt = *cmb;
                arrowOpt.rect = ar.adjusted(3, 1, 4, -1);
                arrowOpt.state = flags;
                arrowOpt.palette.setColor(QPalette::ButtonText, arrowOpt.palette.buttonText().color().lighter(250));
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, painter, widget);
            }
            painter->restore();
        } break;
    case CC_ToolButton:
        if (const QStyleOptionToolButton *toolbutton
                = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            QRect button, menuarea;
            button = proxy()->subControlRect(control, toolbutton, SC_ToolButton, widget);
            menuarea = proxy()->subControlRect(control, toolbutton, SC_ToolButtonMenu, widget);
            State bflags = toolbutton->state & ~State_Sunken;
            if (bflags & State_AutoRaise) {
                if (!UtilityFunctions::hasHover(widget) || !(bflags & State_Enabled)) {
                    bflags &= ~State_Raised;
                }
            }
            State mflags = bflags;
            if (toolbutton->state & State_Sunken) {
                if (toolbutton->activeSubControls & SC_ToolButton)
                    bflags |= State_Sunken;
                mflags |= State_Sunken;
            }
            QStyleOption tool = *toolbutton;
            if (toolbutton->subControls & SC_ToolButton) {
                if (bflags & (State_Sunken | State_On | State_Raised)) {
                    tool.rect = button;
                    tool.state = bflags;
                    proxy()->drawPrimitive(PE_PanelButtonTool, &tool, painter, widget);
                }
            }
            QStyleOptionToolButton label = *toolbutton;
            label.state = bflags;
            int fw = proxy()->pixelMetric(PM_DefaultFrameWidth, option, widget);
            label.rect = button.adjusted(fw, fw, -fw, -fw);
            proxy()->drawControl(CE_ToolButtonLabel, &label, painter, widget);
            if (toolbutton->subControls & SC_ToolButtonMenu) {
                tool.rect = menuarea;
                tool.state = mflags;
                if (mflags & (State_Sunken | State_On | State_Raised))
                    proxy()->drawPrimitive(PE_IndicatorButtonDropDown, &tool, painter, widget);
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &tool, painter, widget);
            } else if (toolbutton->features & QStyleOptionToolButton::HasMenu) {
                int mbi = proxy()->pixelMetric(PM_MenuButtonIndicator, toolbutton, widget);
                QRect ir = toolbutton->rect;
                QStyleOptionToolButton newBtn = *toolbutton;
                newBtn.rect = QRect(ir.right() + 5 - mbi, ir.y() + ir.height() - mbi + 4, mbi - 6, mbi - 6);
                newBtn.rect = visualRect(toolbutton->direction, button, newBtn.rect);
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &newBtn, painter, widget);
            }
        } break;
    default:
        ApplicationStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}