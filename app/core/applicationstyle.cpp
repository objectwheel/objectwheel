#include <applicationstyle.h>
#include <utilityfunctions.h>
#include <utilsicons.h>
#include <paintutils.h>

#include <QStyleFactory>
#include <QApplication>
#include <QPainter>
#include <QStyleOption>
#include <QComboBox>
#include <QMenu>
#include <QScrollBar>
#include <QMdiSubWindow>
#include <QFocusFrame>
#include <QToolBar>
#include <QToolButton>
#include <QPushButton>

#include <private/qapplication_p.h>
#include <private/qcombobox_p.h>
#include <private/qfusionstyle_p_p.h>

namespace {

const int macItemFrame          = 2;    // menu item frame width
const int macItemHMargin        = 3;    // menu item hor text margin
const int macRightBorder        = 12;   // right border on mac
const qreal comboBoxDefaultHeight[3] = { 26, 22, 19 };
const char buttonStyleProperty[] = "_q_ApplicationStyle_buttonStyle";
const char highlightingDisabledForCheckedStateProperty[] = "_q_ApplicationStyle_highlightingDisabledForCheckedState";

void setLayoutItemMargins(int left, int top, int right, int bottom, QRect *rect,
                          Qt::LayoutDirection dir)
{
    if (dir == Qt::RightToLeft)
        rect->adjust(-right, top, -left, bottom);
    else
        rect->adjust(left, top, right, bottom);
}

QRect comboboxInnerBounds(const QRect& outerBounds)
{
    return outerBounds.adjusted(3, 3, -1, -5);
}

void drawArrow(Qt::ArrowType arrowType, const QStyle *style, const QStyleOption *option,
               const QRect &rect, QPainter *painter, const QWidget *widget = 0)
{
    QStyle::PrimitiveElement pe;
    switch (arrowType) {
    case Qt::LeftArrow:
        pe = QStyle::PE_IndicatorArrowLeft;
        break;
    case Qt::RightArrow:
        pe = QStyle::PE_IndicatorArrowRight;
        break;
    case Qt::UpArrow:
        pe = QStyle::PE_IndicatorArrowUp;
        break;
    case Qt::DownArrow:
        pe = QStyle::PE_IndicatorArrowDown;
        break;
    default:
        return;
    }
    QStyleOption arrowOpt = *option;
    arrowOpt.rect = rect;
    style->drawPrimitive(pe, &arrowOpt, painter, widget);
}

void drawArrow(const QStyle *style, const QStyleOptionToolButton *toolbutton,
               const QRect &rect, QPainter *painter, const QWidget *widget = 0)
{
    drawArrow(toolbutton->arrowType, style, toolbutton, rect, painter, widget);
}

bool hasVerticalParentToolBar(const QWidget* widget)
{
    if (widget == 0)
        return false;

    if (auto toolBar = qobject_cast<QToolBar*>(widget->parentWidget()))
        return toolBar->orientation() == Qt::Vertical;

    return false;
}
}

QRectF comboboxEditBounds(const QRectF& outerBounds) // Used by transparentstyle.cpp
{
    QRectF ret = outerBounds;
    ret = ret.adjusted(-5, 0, -24, 0).translated(3, 2);
    ret.setHeight(14);
    return ret;
}

QRectF adjustedControlFrame(const QRectF& rect) // Used by transparentstyle.cpp
{
    QRectF frameRect;
    const auto frameSize = QSizeF(-1, comboBoxDefaultHeight[1]);
    // Center in the style option's rect.
    frameRect = QRectF(QPointF(0, (rect.height() - frameSize.height()) / 2.0),
                       QSizeF(rect.width(), frameSize.height()));
    frameRect = frameRect.translated(rect.topLeft());
    frameRect = frameRect.adjusted(0, 0, -6, 0).translated(4, 0);
    return frameRect;
}

ApplicationStyle::ButtonStyle ApplicationStyle::buttonStyle(const QWidget* widget)
{
    if (widget == 0) {
        qWarning() << "ApplicationStyle: Null widget pointer passed through";
        return Disclosure;
    }
    const QVariant& val = widget->property(buttonStyleProperty);
    if (val.isValid())
        return val.value<ButtonStyle>();
    if (qobject_cast<const QPushButton*>(widget))
        return Push;
    else
        return Disclosure;
}

void ApplicationStyle::setButtonStyle(QWidget* widget, ApplicationStyle::ButtonStyle buttonStyle)
{
    widget->setProperty(buttonStyleProperty, buttonStyle);
}

bool ApplicationStyle::highlightingDisabledForCheckedState(const QWidget* widget)
{
    if (widget == 0) {
        qWarning() << "ApplicationStyle: Null widget pointer passed through";
        return false;
    }
    return widget->property(highlightingDisabledForCheckedStateProperty).value<bool>();
}

void ApplicationStyle::setHighlightingDisabledForCheckedState(QWidget* widget, bool highlightingDisabledForCheckedState)
{
    widget->setProperty(highlightingDisabledForCheckedStateProperty, highlightingDisabledForCheckedState);
}

QPointF ApplicationStyle::visualPos(Qt::LayoutDirection direction, const QRectF& boundingRect,
                                    const QPointF& logicalPos)
{
    if (direction == Qt::LeftToRight)
        return logicalPos;
    return QPointF(boundingRect.right() - logicalPos.x(), logicalPos.y());
}

ApplicationStyle::ApplicationStyle() : QFusionStyle()
{
}

QSize ApplicationStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption* option, const QSize& contentsSize, const QWidget* widget) const
{
    QSize sz = QFusionStyle::sizeFromContents(type, option, contentsSize, widget);

    switch (type) {
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *mi
                = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
            sz = contentsSize;
            int maxpmw = mi->maxIconWidth;
            const QComboBox *comboBox = qobject_cast<const QComboBox*>(widget);
            int w = sz.width(), h = sz.height();
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                w = 10;
                h = 2; // Menu separator height
            } else {
                h = mi->fontMetrics.height() + 2;
                if (!mi->icon.isNull()) {
                    if (comboBox) {
                        const QSize& iconSize = comboBox->iconSize();
                        h = qMax(h, iconSize.height() + 4);
                        maxpmw = qMax(maxpmw, iconSize.width());
                    } else
                    {
                        int iconExtent = proxy()->pixelMetric(PM_SmallIconSize);
                        h = qMax(h, mi->icon.actualSize(QSize(iconExtent, iconExtent)).height() + 4);
                    }
                }
            }
            if (mi->text.contains(QLatin1Char('\t')))
                w += 12;
            else if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                w += 35; // Not quite exactly as it seems to depend on other factors
            if (maxpmw)
                w += maxpmw + 6;
            // add space for a check. All items have place for a check too.
            w += 20;
            if (comboBox && comboBox->isVisible()) {
                QStyleOptionComboBox cmb;
                cmb.initFrom(comboBox);
                cmb.editable = false;
                cmb.subControls = QStyle::SC_ComboBoxEditField;
                cmb.activeSubControls = QStyle::SC_None;
                w = qMax(w, subControlRect(QStyle::CC_ComboBox, &cmb,
                                           QStyle::SC_ComboBoxEditField,
                                           comboBox).width());
            } else {
                w += 12;
            }
            sz = QSize(w, h);
        } break;
    case CT_MenuBarItem:
        sz = contentsSize;
        if (!sz.isEmpty())
            sz += QSize(12, 4); // Constants from QWindowsStyle
        break;
    case CT_Menu: {
        sz = contentsSize;
        QStyleHintReturnMask menuMask;
        QStyleOption myOption = *option;
        myOption.rect.setSize(sz);
        if (proxy()->styleHint(SH_Menu_Mask, &myOption, widget, &menuMask))
            sz = menuMask.region.boundingRect().size();
    } break;
    case CT_ItemViewItem:
        sz.setHeight(sz.height() + 2);
        break;
    case CT_ToolButton:
        if (hasVerticalParentToolBar(widget))
            sz = sz.transposed();
        break;
    case CT_PushButton:
        sz.setHeight(22);
        break;
    case CT_ComboBox:
        sz.setHeight(22);
        break;
    default:
        break;
    }
    return sz;
}

QRect ApplicationStyle::subElementRect(QStyle::SubElement element, const QStyleOption* option,
                                       const QWidget* widget) const
{
    QRect rect;

    switch (element) {
    case SE_ComboBoxLayoutItem:
        if (widget && qobject_cast<QToolBar*>(widget->parentWidget())) {
            // Do nothing, because QToolbar needs the entire widget rect.
            // Otherwise it will be clipped. Equivalent to
            // widget->setAttribute(Qt::WA_LayoutUsesWidgetRect), but without
            // all the hassle.
        } else {
            rect = option->rect;
            setLayoutItemMargins(+2, +1, -3, -4, &rect, option->direction);
        } break;
    default:
        rect = QFusionStyle::subElementRect(element, option, widget);
    }

    return rect;
}

QRect ApplicationStyle::subControlRect(QStyle::ComplexControl control,
                                       const QStyleOptionComplex* option,
                                       QStyle::SubControl subControl, const QWidget* widget) const
{
    QRect ret;

    switch (control) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *combo
                = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            auto editRect = comboboxEditBounds(adjustedControlFrame(combo->rect));
            switch (subControl) {
            case SC_ComboBoxListBoxPopup:
                if (combo->editable) {
                    const QRect inner = comboboxInnerBounds(combo->rect);
                    const int comboTop = combo->rect.top();
                    ret = QRect(inner.x(), comboTop,
                                inner.x() - combo->rect.left() + inner.width(),
                                editRect.bottom() - comboTop + 2);
                } else {
                    ret = QRect(combo->rect.x() + 4 - 11,
                                combo->rect.y() + 1,
                                editRect.width() + 10 + 11,
                                1);
                } break;
            default:
                ret = QFusionStyle::subControlRect(control, option, subControl, widget);
                break;
            }
        } break;
    case CC_ToolButton:
        ret = QFusionStyle::subControlRect(control, option, subControl, widget);
        if (hasVerticalParentToolBar(widget))
            ret = ret.transposed();
        break;
    default:
        ret = QFusionStyle::subControlRect(control, option, subControl, widget);
        break;
    }

    return ret;
}

QPixmap ApplicationStyle::standardPixmap(QStyle::StandardPixmap standardPixmap,
                                         const QStyleOption* option, const QWidget* widget) const
{
    QPixmap pixmap;
    switch (standardPixmap) {
    case SP_DockWidgetCloseButton:
        pixmap = Utils::Icons::CLOSE_TOOLBAR.icon().pixmap(UtilityFunctions::window(widget), {64, 64});
        break;
    case SP_LineEditClearButton:
        pixmap = Utils::Icons::EDIT_CLEAR.icon().pixmap(UtilityFunctions::window(widget), {64, 64});
        break;
    case SP_ToolBarHorizontalExtensionButton: {
        QSize size(16, 16); // Default toolbar icon size
        if (auto btn = qobject_cast<const QToolButton*>(widget))
            size = btn->iconSize();
        pixmap = PaintUtils::renderOverlaidPixmap(":/images/extension.svg", "#505050", size, widget);
    } break;
    default:
        pixmap = QFusionStyle::standardPixmap(standardPixmap, option, widget);
        break;
    }

    if (widget)
        pixmap.setDevicePixelRatio(widget->devicePixelRatioF());
    else
        pixmap.setDevicePixelRatio(qApp->devicePixelRatio());

    return pixmap;
}

QIcon ApplicationStyle::standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption* option,
                                     const QWidget* widget) const
{
    QIcon icon;
    switch (standardIcon) {
    case SP_DockWidgetCloseButton:
    case SP_LineEditClearButton:
        icon.addPixmap(standardPixmap(standardIcon, option, widget), QIcon::Normal);
        icon.addPixmap(PaintUtils::renderOverlaidPixmap(standardPixmap(standardIcon, option, widget),
                                                        QColor(0, 0, 0, 100)), QIcon::Active);
        break;
    case SP_ToolBarHorizontalExtensionButton:
        icon.addPixmap(standardPixmap(standardIcon, option, widget), QIcon::Normal);
        icon.addPixmap(PaintUtils::renderOverlaidPixmap(standardPixmap(standardIcon, option, widget),
                                                        QColor(0, 0, 0, 150)), QIcon::Normal, QIcon::On);
        break;
    default:
        icon = QFusionStyle::standardIcon(standardIcon, option, widget);
        break;
    }
    return icon;
}

int ApplicationStyle::styleHint(QStyle::StyleHint hint, const QStyleOption* option,
                                const QWidget* widget, QStyleHintReturn* returnData) const
{
    switch (hint) {
    case SH_Widget_Animation_Duration:
        return 0; // No animation
    case SH_ToolTipLabel_Opacity:
        return 242; // About 95%
    case SH_ComboBox_AllowWheelScrolling:
        return false;
    case SH_SpinBox_AnimateButton:
        return true;
    case SH_ComboBox_ListMouseTracking:
        return false;
    case SH_ItemView_ScrollMode:
        return QAbstractItemView::ScrollPerPixel;
    case SH_ItemView_ShowDecorationSelected:
        return true;
    case SH_ItemView_MovementWithoutUpdatingSelection:
        return false;
    case SH_ComboBox_Popup:
        return true;
    case SH_ComboBox_PopupFrameStyle:
        return QFrame::NoFrame;
    case SH_Menu_FillScreenWithScroll:
        return false;
    default:
        return QFusionStyle::styleHint(hint, option, widget, returnData);
    }
}

int ApplicationStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    switch (metric) {
    case PM_SmallIconSize:
    case PM_ButtonIconSize:
    case PM_TreeViewIndentation:
    case PM_ListViewIconSize:
    case PM_TabBarIconSize:
    case PM_ToolBarIconSize:
        return 16;
    case PM_ToolBarExtensionExtent:
        return 22; // FIXME: Make this 18 and check if ModeSelectorPane extension icon
        // Minimum geometry for the button, not icon size,
        // icon size is determined by the toolbar's setIconSize
    case PM_ToolBarSeparatorExtent:
    case PM_DockWidgetSeparatorExtent:
        return 1;
    case PM_ToolBarFrameWidth: // QToolBar's contentMargins
    case PM_SplitterWidth:
        return 0;
    case PM_ToolBarItemSpacing:
        return 2;
    case PM_ToolBarItemMargin:
        return 0;
    case PM_ToolBarHandleExtent:
        return 9;
    case PM_ToolTipLabelFrameWidth:
        return 1;
    case PM_MenuVMargin:
        return 4;
    case PM_MenuScrollerHeight:
        return 15;
    case PM_MenuButtonIndicator:
        return 12;
    default:
        return QFusionStyle::pixelMetric(metric, option, widget);
    }
}

void ApplicationStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option,
                                     QPainter* painter, const QWidget* widget) const
{
    switch (element) {
    case PE_Frame: {
        if (widget && widget->inherits("QComboBoxPrivateContainer")){
            QStyleOption copy = *option;
            copy.state |= State_Raised;
            proxy()->drawPrimitive(PE_PanelMenu, &copy, painter, widget);
            break;
        }
        painter->save();
        painter->setPen(QPen(QColor(QStringLiteral("#b6b6b6")), 0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        painter->drawRect(QRectF(option->rect).adjusted(0, 0, -0.5, -0.5));
        painter->restore();
    } break;
    case PE_PanelTipLabel: {
        painter->fillRect(option->rect, option->palette.brush(QPalette::ToolTipBase));
    } break;
    case PE_PanelMenu: {
        painter->save();
        painter->setPen("#b0b0b0");
        painter->setBrush(option->palette.window());
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawRoundedRect(QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5), 5, 5);
        painter->restore();
    } break;
    case PE_IndicatorMenuCheckMark: {
        painter->save();
        QColor pc;
        if (option->state & State_On)
            pc = option->palette.highlightedText().color();
        else
            pc = option->palette.text().color();

        painter->setPen(pc);
        painter->setFont(QFont());
        painter->drawText(option->rect.adjusted(-2, 1, -2, 1), "\u2713", QTextOption(Qt::AlignCenter));
        painter->restore();
    } break;
    case PE_IndicatorArrowDown: {
        if (option->rect.width() <= 1 || option->rect.height() <= 1)
            break;
        if (!(option->state & State_Enabled))
            break;

        painter->save();

        enum { ARROW_LENGTH = 3 };
        QRectF r(0, 0, ARROW_LENGTH, ARROW_LENGTH / 2.0);
        r.moveCenter(QRectF(option->rect).center());

        const QColor& textColor = (option->state & State_Sunken)
                ? option->palette.buttonText().color().darker()
                : option->palette.buttonText().color();
        QPen arrowPen(textColor);
        arrowPen.setWidthF(1.3);
        arrowPen.setCapStyle(Qt::RoundCap);
        arrowPen.setJoinStyle(Qt::MiterJoin);

        QPointF points[] = {{0, 0}, {ARROW_LENGTH / 2.0, ARROW_LENGTH / 2.0}, {ARROW_LENGTH, 0}};
        points[0] += r.topLeft(); points[1] += r.topLeft(); points[2] += r.topLeft();
        painter->setPen(arrowPen);
        painter->setBrush(textColor);
        painter->drawPolygon(points, 3);
        painter->restore();
    } break;
    case PE_IndicatorToolBarSeparator: {
        QPainterPath path;
        QRectF r(option->rect);
        if (option->state & State_Horizontal) {
            qreal xpoint = r.center().x();
            path.moveTo(xpoint + 0.5, r.top() + 3);
            path.lineTo(xpoint + 0.5, r.bottom() - 3);
        } else {
            qreal ypoint = r.center().y();
            path.moveTo(r.left() + 2 , ypoint + 0.5);
            path.lineTo(r.right() + 1, ypoint + 0.5);
        }
        QPainterPathStroker theStroker;
        theStroker.setCapStyle(Qt::FlatCap);
        // theStroker.setDashPattern(QVector<qreal>() << 1 << 2);
        path = theStroker.createStroke(path);
        painter->fillPath(path, QColor("#50000000"));
    } break;
    case PE_PanelButtonTool:
        if (widget && widget->objectName() == "qt_toolbar_ext_button")
            break;
        if (widget && widget->parent() && widget->parent()->objectName() == "_q_ModeSelectorPane") {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::NoBrush);
            if (option->state & State_Sunken)
                painter->setBrush(QColor("#80000000"));
            else if (option->state & State_On)
                painter->setBrush(QColor("#60000000"));
            else if (option->state & State_MouseOver)
                painter->setBrush(QColor("#20000000"));
            painter->drawRoundedRect(QRectF(option->rect), 7, 7);
            painter->restore();
            break;
        }
        if (hasVerticalParentToolBar(widget)) {
            painter->save();
            painter->translate(0, widget->height());
            painter->rotate(-90);
            QFusionStyle::drawPrimitive(element, option, painter, widget);
            painter->restore();
            break;
        }
    case PE_PanelButtonCommand: {
        QRectF rect(option->rect);
        ButtonStyle style = buttonStyle(widget);
        bool hasMenu = false;
        bool isFlat = false;
        bool isDefault = false;
        bool hasHover = option->state & QStyle::State_MouseOver;
        bool isEnabled = option->state & QStyle::State_Enabled;
        bool isDown = (option->state & QStyle::State_Sunken) || (!highlightingDisabledForCheckedState(widget) && option->state & QStyle::State_On);
        bool hasFocus = (option->state & QStyle::State_HasFocus) && (option->state & QStyle::State_KeyboardFocusChange);

        if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            hasMenu = button->features & QStyleOptionButton::HasMenu;
            isFlat = button->features & QStyleOptionButton::Flat;
            isDefault = button->features & QStyleOptionButton::DefaultButton;
        }

        if (widget && !UtilityFunctions::hasHover(widget)) // FIXME: This is a workaround for QTBUG-44400
            hasHover = false;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        if (!isFlat || isDown) {
            switch (style) {
            case Push: {
                rect.adjust(0.5, 0.5, -0.5, 0);
                // Draw shadows
                QLinearGradient shadowGrad(0, 0, 0, 1);
                shadowGrad.setCoordinateMode(QGradient::ObjectMode);
                shadowGrad.setColorAt(0.0, isEnabled ? "#07000000" : "#05000000");
                shadowGrad.setColorAt(0.1, isEnabled ? "#07000000" : "#05000000");
                shadowGrad.setColorAt(0.9, isEnabled ? "#07000000" : "#05000000");
                shadowGrad.setColorAt(1.0, isEnabled ? "#15000000" : "#08000000");
                painter->setPen(Qt::NoPen);
                painter->setBrush(shadowGrad);
                painter->drawRoundedRect(rect.adjusted(0, 0, 0, -0.5), 4, 4);
                painter->setBrush(QColor(isEnabled ? "#09000000" : "#05000000"));
                painter->drawRoundedRect(rect.adjusted(1, 1, -1, 0), 4, 4);

                // Draw border
                QLinearGradient borderGrad(0, 0, 0, 1);
                borderGrad.setCoordinateMode(QGradient::ObjectMode);
                borderGrad.setColorAt(0.0, isEnabled ? isDown ? "#3280f7" : ((isDefault || hasFocus) ? "#5094f7" : "#d8d8d8") : "#ebebeb");
                borderGrad.setColorAt(0.1, isEnabled ? isDown ? "#2e7bf3" : ((isDefault || hasFocus) ? "#468ef8" : "#d0d0d0") : "#e7e7e7");
                borderGrad.setColorAt(0.9, isEnabled ? isDown ? "#1a5fda" : ((isDefault || hasFocus) ? "#196dfb" : "#d0d0d0") : "#e7e7e7");
                borderGrad.setColorAt(1.0, isEnabled ? isDown ? "#1659d5" : ((isDefault || hasFocus) ? "#1367fb" : "#bcbcbc") : "#dddddd");
                painter->setBrush(borderGrad);
                painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -1), 3.5, 3.5);

                // Draw body
                QLinearGradient bodyGrad(0, 0, 0, 1);
                bodyGrad.setCoordinateMode(QGradient::ObjectMode);
                bodyGrad.setColorAt(0.0, isEnabled ? isDown ? "#5496f9" : ((isDefault || hasFocus) ? "#6fa7f8" : "white") : "white");
                bodyGrad.setColorAt(0.9, isEnabled ? isDown ? "#1c65dd" : ((isDefault || hasFocus) ? "#176ffb" : "white") : "white");
                bodyGrad.setColorAt(1.0, isEnabled ? isDown ? "#1c65dd" : ((isDefault || hasFocus) ? "#176ffb" : "white") : "white");
                painter->setBrush(bodyGrad);
                painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1.5), 3, 3);
            } break;
            case Disclosure: {
                rect.adjust(0.5, 0.5, -0.5, 0);
                // Draw shadows
                QLinearGradient shadowGrad(0, 0, 0, 1);
                shadowGrad.setCoordinateMode(QGradient::ObjectMode);
                shadowGrad.setColorAt(0.0, isEnabled ? "#07000000" : "#05000000");
                shadowGrad.setColorAt(0.1, isEnabled ? "#07000000" : "#05000000");
                shadowGrad.setColorAt(0.9, isEnabled ? "#07000000" : "#05000000");
                shadowGrad.setColorAt(1.0, isEnabled ? "#15000000" : "#08000000");
                painter->setPen(Qt::NoPen);
                painter->setBrush(shadowGrad);
                painter->drawRoundedRect(rect.adjusted(0, 0, 0, -0.5), 4, 4);
                painter->setBrush(QColor(isEnabled ? "#09000000" : "#05000000"));
                painter->drawRoundedRect(rect.adjusted(1, 1, -1, 0), 4, 4);

                // Draw border
                QLinearGradient borderGrad(0, 0, 0, 1);
                borderGrad.setCoordinateMode(QGradient::ObjectMode);
                borderGrad.setColorAt(0.0, isEnabled ? "#d8d8d8" : "#ebebeb");
                borderGrad.setColorAt(0.1, isEnabled ? "#d0d0d0" : "#e7e7e7");
                borderGrad.setColorAt(0.9, isEnabled ? "#d0d0d0" : "#e7e7e7");
                borderGrad.setColorAt(1.0, isEnabled ? "#bcbcbc" : "#dddddd");
                painter->setBrush(borderGrad);
                painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -1), 3.5, 3.5);

                // Draw body
                painter->setBrush(QColor(isEnabled ? isDown ? "#f0f0f0" : "white" : "white"));
                painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1.5), 3, 3);
            } break;
            case Help: {
                rect.adjust(0.5, 0.5, -0.5, 0);
                // Draw shadows
                qreal radius = qMin(rect.width() - 1, rect.height() - 1.5) / 2.0;
                QLinearGradient shadowGrad(0, 0, 0, 1);
                shadowGrad.setCoordinateMode(QGradient::ObjectMode);
                shadowGrad.setColorAt(0.0, isEnabled ? "#07000000" : "#05000000");
                shadowGrad.setColorAt(0.1, isEnabled ? "#07000000" : "#05000000");
                shadowGrad.setColorAt(0.9, isEnabled ? "#07000000" : "#05000000");
                shadowGrad.setColorAt(1.0, isEnabled ? "#15000000" : "#08000000");
                painter->setPen(Qt::NoPen);
                painter->setBrush(shadowGrad);
                painter->drawRoundedRect(rect.adjusted(0, 0, 0, -0.5), radius + 0.5, radius + 0.5);
                painter->setBrush(QColor(isEnabled ? "#09000000" : "#05000000"));
                painter->drawRoundedRect(rect.adjusted(1, 1, -1, 0), radius + 0.5, radius + 0.5);

                // Draw border
                QLinearGradient borderGrad(0, 0, 0, 1);
                borderGrad.setCoordinateMode(QGradient::ObjectMode);
                borderGrad.setColorAt(0.0, isEnabled ? "#d8d8d8" : "#ebebeb");
                borderGrad.setColorAt(0.1, isEnabled ? "#d0d0d0" : "#e7e7e7");
                borderGrad.setColorAt(0.9, isEnabled ? "#d0d0d0" : "#e7e7e7");
                borderGrad.setColorAt(1.0, isEnabled ? "#bcbcbc" : "#dddddd");
                painter->setBrush(borderGrad);
                painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -1), radius, radius);

                // Draw body
                painter->setBrush(QColor(isEnabled ? isDown ? "#f0f0f0" : "white" : "white"));
                painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1.5),  radius - 0.5,  radius - 0.5);
            } break;
            case TexturedRounded: {
                // Draw drop shadow
                rect.adjust(0.5, 0, -0.5, 0);
                QLinearGradient shadowGrad(0, 0, 1, 0);
                shadowGrad.setCoordinateMode(QGradient::ObjectMode);
                shadowGrad.setColorAt(0, "#12202020");
                shadowGrad.setColorAt(0.05, "#10202020");
                shadowGrad.setColorAt(0.5, "#10202020");
                shadowGrad.setColorAt(0.95, "#10202020");
                shadowGrad.setColorAt(1, "#12202020");
                QPainterPath dropShadowPath;
                dropShadowPath.addRect(3.5, option->rect.bottom() + 0.5, option->rect.width() - 7, 1);
                painter->setPen(Qt::NoPen);
                painter->setBrush(shadowGrad);
                painter->drawPath(dropShadowPath);

                // Draw shadow
                QLinearGradient darkGrad(0, 0, 0, 1);
                darkGrad.setCoordinateMode(QGradient::ObjectMode);
                darkGrad.setColorAt(0.85, "#20303030");
                darkGrad.setColorAt(1, "#3f000000");
                QPainterPath shadowPath;
                shadowPath.addRoundedRect(rect.adjusted(0, 0.5, 0, -0.5), 4, 4);
                painter->setPen(Qt::NoPen);
                painter->setBrush(darkGrad);
                painter->drawPath(shadowPath);

                // Draw body
                QLinearGradient midGrad(0, 0, 0, 1);
                midGrad.setCoordinateMode(QGradient::ObjectMode);
                midGrad.setColorAt(0, "#e4e4e4");
                midGrad.setColorAt(1, "#dedede");
                QLinearGradient buttonGrad(0, 0, 0, 1);
                buttonGrad.setCoordinateMode(QGradient::ObjectMode);
                buttonGrad.setColorAt(0, "#fdfdfd");
                buttonGrad.setColorAt(1, "#f3f3f3");
                QPainterPath bodyPath;
                bodyPath.addRoundedRect(rect.adjusted(0.5, 1, -0.5, -1), 3.5, 3.5);
                painter->setPen(Qt::NoPen);
                painter->setBrush(isDown ? midGrad : buttonGrad);
                painter->drawPath(bodyPath);
                // NOTE: QRect's bottom() and bottomLeft()... are different from QRectF, for historical reasons
                // as stated in the docs, those functions return top() + height() - 1 (QRect)

                // Draw glowing for pressed state
                if (isDown) {
                    QPainterPath glowPathUp;
                    glowPathUp.addRoundedRect(rect.adjusted(0, 1.5, 0, 1), 4.5, 4.5);
                    QPainterPath glowPathDown;
                    glowPathDown.addRoundedRect(rect.adjusted(0, 2, 0, 1), 4.5, 4.5);
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(QColor("#ebebeb"));
                    painter->drawPath(bodyPath.subtracted(glowPathDown));
                    painter->setBrush(QColor("#f3f3f3"));
                    painter->drawPath(bodyPath.subtracted(glowPathUp));
                }
            } break;
            default:
                break;
            }
        }

        if (isEnabled && hasMenu && (hasHover || isDown)) {
            if (auto button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
                int mbi = proxy()->pixelMetric(QStyle::PM_MenuButtonIndicator, option);
                QStyleOptionButton newBtn = *button;
                newBtn.rect = QRect(rect.right() - mbi, rect.bottom() - mbi, mbi, mbi);
                proxy()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &newBtn, painter);
            }
        }

        painter->restore();
    } break;
    default:
        QFusionStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

void ApplicationStyle::drawControl(QStyle::ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    Q_D (const QFusionStyle);

    switch (element) {
    case CE_MenuItem:
    case CE_MenuHMargin:
    case CE_MenuVMargin:
    case CE_MenuTearoff:
    case CE_MenuScroller:
        if (const QStyleOptionMenuItem *m
                = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
            painter->save();
            QStyleOptionMenuItem mi(*m);
            mi.rect.adjust(1, 1, -1, -1);
            const bool active = mi.state & State_Selected;
            if (active)
                painter->fillRect(mi.rect, mi.palette.highlight());
            if (element == CE_MenuTearoff) {
                painter->setPen(QPen(mi.palette.dark().color(), 1, Qt::DashLine));
                painter->drawLine(mi.rect.x() + 2, mi.rect.y() + mi.rect.height() / 2 - 1,
                                  mi.rect.x() + mi.rect.width() - 4,
                                  mi.rect.y() + mi.rect.height() / 2 - 1);
                painter->setPen(QPen(mi.palette.light().color(), 1, Qt::DashLine));
                painter->drawLine(mi.rect.x() + 2, mi.rect.y() + mi.rect.height() / 2,
                                  mi.rect.x() + mi.rect.width() - 4,
                                  mi.rect.y() + mi.rect.height() / 2);
            } else if (element == CE_MenuScroller) {
                const QSize scrollerSize = QSize(10, 8);
                const int scrollerVOffset = 5;
                const int left = mi.rect.x() + (mi.rect.width() - scrollerSize.width()) / 2;
                const int right = left + scrollerSize.width();
                int top;
                int bottom;
                if (option->state & State_DownArrow) {
                    bottom = mi.rect.y() + scrollerVOffset;
                    top = bottom + scrollerSize.height();
                } else {
                    bottom = mi.rect.bottom() - scrollerVOffset;
                    top = bottom - scrollerSize.height();
                }
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);
                QPainterPath path;
                path.moveTo(left, bottom);
                path.lineTo(right, bottom);
                path.lineTo((left + right) / 2, top);
                painter->fillPath(path, option->palette.buttonText());
                painter->restore();
            } else if (element != CE_MenuItem) {
                painter->restore();
                break;
            }
            if (mi.menuItemType == QStyleOptionMenuItem::Separator) {
                const QRect separatorRect = QRect(mi.rect.left(), mi.rect.center().y(), mi.rect.width(), 2);
                painter->fillRect(separatorRect, "#D5D5D5");
                painter->restore();
                break;
            }
            const int maxpmw = 20/*mi.maxIconWidth*/ ;
            const bool enabled = mi.state & State_Enabled;
            int xpos = mi.rect.x() + 18;
            int checkcol = maxpmw;
            if (!enabled)
                painter->setPen(mi.palette.text().color());
            else if (active)
                painter->setPen(mi.palette.highlightedText().color());
            else
                painter->setPen(mi.palette.buttonText().color());
            if (mi.checked) {
                QStyleOption checkmarkOpt;
                checkmarkOpt.initFrom(widget);
                const int mw = checkcol + macItemFrame;
                const int mh = mi.rect.height() + macItemFrame;
                const int xp = mi.rect.x() + macItemFrame;
                checkmarkOpt.rect = QRect(xp, mi.rect.y() - checkmarkOpt.fontMetrics.descent(), mw, mh);
                checkmarkOpt.state.setFlag(State_On, active);
                checkmarkOpt.state.setFlag(State_Enabled, enabled);
                checkmarkOpt.state |= State_Small;
                // We let drawPrimitive(PE_IndicatorMenuCheckMark) pick the right color
                checkmarkOpt.palette.setColor(QPalette::HighlightedText, painter->pen().color());
                checkmarkOpt.palette.setColor(QPalette::Text, painter->pen().color());
                proxy()->drawPrimitive(PE_IndicatorMenuCheckMark, &checkmarkOpt, painter, widget);
            }
            bool dis = !(mi.state & State_Enabled);
            bool act = mi.state & State_Selected;
            if (!mi.icon.isNull()) {
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap;

                int smallIconSize = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                QSize iconSize(smallIconSize, smallIconSize);

                if (const QComboBox *combo = qobject_cast<const QComboBox*>(widget))
                    iconSize = combo->iconSize();

                QWindow* window = nullptr;
                if (widget)
                    window = UtilityFunctions::window(widget);
                if (mi.checked)
                    pixmap = mi.icon.pixmap(window, iconSize, mode, QIcon::On);
                else
                    pixmap = mi.icon.pixmap(window, iconSize, mode);
                int pixw = pixmap.width() / pixmap.devicePixelRatio();
                int pixh = pixmap.height() / pixmap.devicePixelRatio();
                QRect cr(xpos, mi.rect.y(), checkcol, mi.rect.height());
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(cr.center());
                painter->drawPixmap(pmr.topLeft(), pixmap);
                xpos += pixw + 6;
            }

            QString s = mi.text;
            const auto text_flags = Qt::AlignVCenter | Qt::TextHideMnemonic
                    | Qt::TextSingleLine | Qt::AlignAbsolute;
            int yPos = mi.rect.y();
            const bool isSubMenu = mi.menuItemType == QStyleOptionMenuItem::SubMenu;
            const int tabwidth = isSubMenu ? 9 : mi.tabWidth;
            QString rightMarginText;
            if (isSubMenu)
                rightMarginText = QStringLiteral("\u25b6\ufe0e"); // U+25B6 U+FE0E: BLACK RIGHT-POINTING TRIANGLE
            // If present, save and remove embedded shorcut from text
            const int tabIndex = s.indexOf(QLatin1Char('\t'));
            if (tabIndex >= 0) {
                if (!isSubMenu) // ... but ignore it if it's a submenu.
                    rightMarginText = s.mid(tabIndex + 1);
                s = s.left(tabIndex);
            }
            if (!rightMarginText.isEmpty()) {
                painter->setFont(qt_app_fonts_hash()->value("QMenuItem", painter->font()));
                int xp = mi.rect.right() - tabwidth - macRightBorder + 2;
                if (!isSubMenu)
                    xp -= macItemHMargin + macItemFrame + 3; // Adjust for shortcut
                painter->drawText(xp, yPos, tabwidth, mi.rect.height(), text_flags | Qt::AlignRight, rightMarginText);
            }
            if (!s.isEmpty()) {
                const int xm = macItemFrame + maxpmw + macItemHMargin;
                QFont myFont = mi.font;
                myFont.setPixelSize(QFontInfo(mi.font).pixelSize());
                painter->setFont(myFont);
                painter->drawText(xpos, yPos, mi.rect.width() - xm - tabwidth + 1,
                                  mi.rect.height(), text_flags, s);
            }
            painter->restore();
        } break;
    case CE_ComboBoxLabel:
        if (const auto *cb
                = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            auto copy = *cb;
            copy.direction = Qt::LeftToRight;
            // The rectangle will be adjusted to SC_ComboBoxEditField with comboboxEditBounds()
            QFusionStyle::drawControl(CE_ComboBoxLabel, &copy, painter, widget);
        } break;
    case CE_PushButtonLabel:
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QRectF ir = button->rect;
            uint tf = Qt::AlignVCenter;
            if (styleHint(SH_UnderlineShortcut, button, widget))
                tf |= Qt::TextShowMnemonic;
            else
                tf |= Qt::TextHideMnemonic;
            if (!button->icon.isNull()) {
                //Center both icon and text
                QPointF point;
                QIcon::Mode mode = button->state & State_Enabled ? QIcon::Normal
                                                                 : QIcon::Disabled;
                if (mode == QIcon::Normal && button->state & State_HasFocus)
                    mode = QIcon::Active;
                QIcon::State state = QIcon::Off;
                if (button->state & State_On)
                    state = QIcon::On;

                QPixmap pixmap = button->icon.pixmap(UtilityFunctions::window(widget), button->iconSize, mode, state);
                if (button->state & State_Sunken)
                    pixmap = PaintUtils::renderOverlaidPixmap(pixmap, "#30000000");
                qreal w = pixmap.width() / pixmap.devicePixelRatio();
                qreal h = pixmap.height() / pixmap.devicePixelRatio();
                if (!button->text.isEmpty())
                    w += button->fontMetrics.boundingRect(option->rect, tf, button->text).width() + 3;
                point = QPointF(ir.x() + ir.width() / 2 - w / 2,
                                ir.y() + ir.height() / 2 - h / 2);
                w = pixmap.width() / pixmap.devicePixelRatio();
                if (button->direction == Qt::RightToLeft)
                    point.rx() += w;
                painter->drawPixmap(visualPos(button->direction, button->rect, point), pixmap, pixmap.rect());
                if (button->direction == Qt::RightToLeft)
                    ir.translate(ir.x() - point.x() - 2, 0);
                else
                    ir.translate(point.x() - ir.x() + w + 3, 0);
                // left-align text if there is
                if (!button->text.isEmpty())
                    tf |= Qt::AlignLeft;
            } else {
                tf |= Qt::AlignHCenter;
            }
            if (button->features & QStyleOptionButton::HasMenu)
                ir = ir.adjusted(0, 0, -proxy()->pixelMetric(PM_MenuButtonIndicator, button, widget), 0);

            // Draw item text
            QStyleOptionButton copy(*button);
            if (buttonStyle(widget) == Push) {
                bool isFlat = false;
                bool isDefault = false;
                bool isEnabled = option->state & QStyle::State_Enabled;
                bool isDown = (option->state & QStyle::State_Sunken) || (!highlightingDisabledForCheckedState(widget) && option->state & QStyle::State_On);
                bool hasFocus = (option->state & QStyle::State_HasFocus) && (option->state & QStyle::State_KeyboardFocusChange);

                if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
                    isFlat = button->features & QStyleOptionButton::Flat;
                    isDefault = button->features & QStyleOptionButton::DefaultButton;
                }

                if (isEnabled && (!isFlat || isDown)) {
                    if (isDown)
                        copy.palette.setColor(QPalette::ButtonText, "#e0ebfb");
                    else if (isDefault || hasFocus)
                        copy.palette.setColor(QPalette::ButtonText, Qt::white);
                }
            }
            drawItemText(painter, ir, tf, copy.palette, copy.state & State_Enabled,
                         copy.text, QPalette::ButtonText);
        } break;
    case CE_ToolButtonLabel:
        if (const QStyleOptionToolButton* opt
                = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            painter->save();
            QStyleOptionToolButton copy(*opt);
            QStyleOptionToolButton* toolbutton = &copy;
            if (hasVerticalParentToolBar(widget)) {
                painter->translate(0, widget->height());
                painter->rotate(-90);
            }
            if (widget && widget->parent() && widget->parent()->objectName() == "_q_ModeSelectorPane") {
                toolbutton->rect.adjust(4, 0, 0, 0);
                if (toolbutton->state & State_On || toolbutton->state & State_Sunken)
                    toolbutton->palette.setColor(QPalette::ButtonText, Qt::white);
            }
            QRect rect = toolbutton->rect;
            int shiftX = 0;
            int shiftY = 0;
            if (toolbutton->state & (State_Sunken | State_On)) {
                shiftX = proxy()->pixelMetric(PM_ButtonShiftHorizontal, toolbutton, widget);
                shiftY = proxy()->pixelMetric(PM_ButtonShiftVertical, toolbutton, widget);
            }
            // Arrow type always overrules and is always shown
            bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
            if (((!hasArrow && toolbutton->icon.isNull()) && !toolbutton->text.isEmpty())
                    || toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) {
                int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
                if (!proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                    alignment |= Qt::TextHideMnemonic;
                rect.translate(shiftX, shiftY);
                painter->setFont(toolbutton->font);
                proxy()->drawItemText(painter, rect, alignment, toolbutton->palette,
                                      option->state & State_Enabled, toolbutton->text,
                                      QPalette::ButtonText);
            } else {
                QPixmap pm;
                QSize pmSize = toolbutton->iconSize;
                if (!toolbutton->icon.isNull()) {
                    QIcon::State state = toolbutton->state & State_On ? QIcon::On : QIcon::Off;
                    QIcon::Mode mode;
                    if (widget && widget->parent() && widget->parent()->objectName()
                            == "_q_ModeSelectorPane" && toolbutton->state & State_Sunken) {
                        state = QIcon::On;
                    }
                    if (!(toolbutton->state & State_Enabled))
                        mode = QIcon::Disabled;
                    else if ((option->state & State_MouseOver) && (option->state & State_AutoRaise))
                        mode = QIcon::Active;
                    else
                        mode = QIcon::Normal;
                    pm = toolbutton->icon.pixmap(UtilityFunctions::window(widget),
                                                 toolbutton->rect.size().boundedTo(toolbutton->iconSize),
                                                 mode, state);
                    pmSize = pm.size() / pm.devicePixelRatio();
                }

                if (toolbutton->toolButtonStyle != Qt::ToolButtonIconOnly) {
                    painter->setFont(toolbutton->font);
                    QRect pr = rect,
                            tr = rect;
                    int alignment = Qt::TextShowMnemonic;
                    if (!proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                        alignment |= Qt::TextHideMnemonic;

                    if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                        pr.setHeight(pmSize.height() + 4); //### 4 is currently hardcoded in QToolButton::sizeHint()
                        tr.adjust(0, pr.height() - 1, 0, -1);
                        pr.translate(shiftX, shiftY);
                        if (!hasArrow) {
                            proxy()->drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
                        } else {
                            drawArrow(proxy(), toolbutton, pr, painter, widget);
                        }
                        alignment |= Qt::AlignCenter;
                    } else {
                        pr.setWidth(pmSize.width() + 4); //### 4 is currently hardcoded in QToolButton::sizeHint()
                        tr.adjust(pr.width(), 0, 0, 0);
                        pr.translate(shiftX, shiftY);
                        if (!hasArrow) {
                            proxy()->drawItemPixmap(painter, QStyle::visualRect(option->direction, rect, pr), Qt::AlignCenter, pm);
                        } else {
                            drawArrow(proxy(), toolbutton, pr, painter, widget);
                        }
                        alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                    }
                    tr.translate(shiftX, shiftY);
                    const QString text = d->toolButtonElideText(toolbutton, tr, alignment);
                    proxy()->drawItemText(painter, QStyle::visualRect(option->direction, rect, tr), alignment, toolbutton->palette,
                                          toolbutton->state & State_Enabled, text,
                                          QPalette::ButtonText);
                } else {
                    rect.translate(shiftX, shiftY);
                    if (hasArrow) {
                        drawArrow(proxy(), toolbutton, rect, painter, widget);
                    } else {
                        proxy()->drawItemPixmap(painter, rect, Qt::AlignCenter, pm);
                    }
                }
            }
            painter->restore();
        } break;
    case CE_ToolBar:
        if (const auto *cb = qstyleoption_cast<const QStyleOptionToolBar*>(option)) {
            painter->save();
            const QPointF finalPoint = (cb->state & State_Horizontal) ? QPointF(0, 1) : QPointF(1, 0);
            QLinearGradient gradient(QPointF(), finalPoint);
            gradient.setCoordinateMode(QGradient::ObjectMode);
            gradient.setColorAt(0, QStringLiteral("#ffffff"));
            gradient.setColorAt(1, QStringLiteral("#e3e3e3"));
            painter->setBrush(gradient);
            painter->setPen(QPen(QColor(QStringLiteral("#b6b6b6")), 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
            painter->drawRect(cb->rect);
            painter->restore();
        } break;
    case CE_RubberBand:
        if (qstyleoption_cast<const QStyleOptionRubberBand *>(option)) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, false);
            QPen pen(QColor(0, 0, 0, 200), 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
            pen.setCosmetic(true);
            painter->setPen(pen);
            painter->setBrush(QColor(0, 0, 0, 30));
            painter->drawRect(option->rect);
            painter->restore();
        } break;
    case CE_Splitter: {
        QRectF r(option->rect);
        if (option->state & State_Horizontal) {
            for (int i = -6; i < 12; i += 3)
                painter->fillRect(r.center().x(), r.center().y() + i, 1, 1, QColor(0, 0, 0, 150));
        } else {
            for (int i = -6; i < 12; i += 3)
                painter->fillRect(r.center().x() + i, r.center().y(), 1, 1, QColor(0, 0, 0, 150));
        }
    } break;
    default:
        QFusionStyle::drawControl(element, option, painter, widget);
        break;
    }
}

void ApplicationStyle::drawComplexControl(QStyle::ComplexControl control,
                                          const QStyleOptionComplex* option,
                                          QPainter* painter, const QWidget* widget) const
{
    switch (control) {
    case CC_ComboBox:
        if (auto combo = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            if (combo->subControls & SC_ComboBoxArrow) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);
                QStyleOptionComboBox copy = *combo;
                copy.subControls &= ~SC_ComboBoxArrow;
                QFusionStyle::drawComplexControl(control, &copy, painter, widget);
                QColor arrowColor = option->palette.buttonText().color();
                arrowColor.setAlpha(230);
                QRectF dr = proxy()->subControlRect(CC_ComboBox, combo, SC_ComboBoxArrow, widget);
                QPainterPath path;
                path.moveTo(0.5, 0.5);
                path.lineTo(3.5, 3.5);
                path.lineTo(6.5, 0.5);
                painter->translate(dr.topLeft() + QPointF(dr.width() / 2.0 - 3.5, dr.height() / 2.0 - 2.25));
                painter->setPen(QPen(arrowColor, 1.4, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
                painter->drawPath(path);
                painter->restore();
            }
        } break;
    default:
        QFusionStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}

void ApplicationStyle::drawItemText(QPainter* painter, const QRect& rect, int alignment,
                                    const QPalette& pal, bool enabled, const QString& text,
                                    QPalette::ColorRole textRole) const
{
    drawItemText(painter, QRectF(rect), alignment, pal, enabled, text, textRole);
}

void ApplicationStyle::drawItemText(QPainter* painter, const QRectF& rect, int alignment,
                                    const QPalette& pal, bool enabled, const QString& text,
                                    QPalette::ColorRole textRole) const
{
    if (text.isEmpty())
        return;

    QPen savedPen = painter->pen();
    if (textRole != QPalette::NoRole) {
        painter->setPen(QPen(pal.brush(enabled ? QPalette::Active : QPalette::Disabled, textRole),
                             savedPen.widthF()));
    }
    painter->drawText(rect, alignment, text);
    painter->setPen(savedPen);
}

void ApplicationStyle::polish(QWidget* w)
{
    if (qobject_cast<QMenu*>(w)
            || qobject_cast<QComboBoxPrivateContainer*>(w)
            || qobject_cast<QMdiSubWindow*>(w)) {

        if (!w->property("ow_flag_set").isValid()) {
            w->setProperty("ow_flag_set", true);
            for (QWidget* wd : w->findChildren<QWidget*>()) {
                if (wd != w)
                    wd->setStyleSheet("QWidget { background: transparent; }");
            }
            class A final : public QObject {
                bool eventFilter(QObject* w, QEvent* e) override
                {
                    if (e->type() == QEvent::Show) {
                        QWidget* wid = static_cast<QWidget*>(w);
                        QWidget* parentWid = wid->parentWidget();
                        wid->setWindowFlags(wid->windowFlags()
                                            | Qt::X11BypassWindowManagerHint
                                            | Qt::NoDropShadowWindowHint
                                            | Qt::FramelessWindowHint);
                        wid->removeEventFilter(this);
                        if (parentWid)
                            wid->show();
                    }
                    return false;
                }
            } static shadowRemover;
            w->installEventFilter(&shadowRemover);
        }
        w->setAttribute(Qt::WA_TranslucentBackground, true);
        w->setAutoFillBackground(false);
    }

    QFusionStyle::polish(w);

    if (qobject_cast<QScrollBar*>(w)) {
        w->setAttribute(Qt::WA_OpaquePaintEvent, false);
        w->setAttribute(Qt::WA_Hover, true);
        w->setMouseTracking(true);
    }
}

void ApplicationStyle::unpolish(QWidget* w)
{
    if (qobject_cast<QMenu*>(w) && !w->testAttribute(Qt::WA_SetPalette)) {
        QPalette pal = qApp->palette(w);
        w->setPalette(pal);
        w->setAttribute(Qt::WA_SetPalette, false);
        w->setWindowOpacity(1.0);
    }

    if (QComboBox *combo = qobject_cast<QComboBox*>(w)) {
        if (!combo->isEditable()) {
            if (QWidget *widget = combo->findChild<QComboBoxPrivateContainer*>())
                widget->setWindowOpacity(1.0);
        }
    }

    if (QFocusFrame *frame = qobject_cast<QFocusFrame*>(w))
        frame->setAttribute(Qt::WA_NoSystemBackground, true);

    QFusionStyle::unpolish(w);

    if (qobject_cast<QScrollBar*>(w)) {
        w->setAttribute(Qt::WA_OpaquePaintEvent, true);
        w->setAttribute(Qt::WA_Hover, false);
        w->setMouseTracking(false);
    }
}
