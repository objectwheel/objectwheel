#include <transparentstyle.h>

#include <QStyleFactory>
#include <QStyleOptionSpinBox>
#include <QPainter>
#include <qdrawutil.h>
#include <QtWidgets>

#include <private/qapplication_p.h>
#include <private/qcombobox_p.h>
#include <private/qstylehelper_p.h>

namespace {

const int macItemFrame          = 2;    // menu item frame width
const int macItemHMargin        = 3;    // menu item hor text margin
const int macRightBorder        = 12;   // right border on mac
const int PushButtonLeftOffset  = 6;
const int PushButtonRightOffset = 12;

const qreal pushButtonDefaultHeight[3] = {
    32, 28, 16
};

const qreal comboBoxDefaultHeight[3] = {
    26, 22, 19
};

QRectF comboboxEditBounds(const QRectF& outerBounds)
{
    QRectF ret = outerBounds;
    ret = ret.adjusted(-5, 0, -24, 0).translated(3, 2);
    ret.setHeight(14);
    return ret;
}

QRectF adjustedControlFrame(const QRectF& rect)
{
    QRectF frameRect;
    const auto frameSize = QSizeF(-1, comboBoxDefaultHeight[2]);
    // Center in the style option's rect.
    frameRect = QRectF(QPointF(0, (rect.height() - frameSize.height()) / 2.0),
                       QSizeF(rect.width(), frameSize.height()));
    frameRect = frameRect.translated(rect.topLeft());
    frameRect = frameRect.adjusted(0, 0, -6, 0).translated(4, 0);
    return frameRect;
}

QRect comboboxInnerBounds(const QRect& outerBounds)
{
    return outerBounds.adjusted(3, 3, -1, -5);
}

QPainterPath windowPanelPath(const QRectF& r)
{
    static const qreal CornerPointOffset = 5.5;
    static const qreal CornerControlOffset = 2.1;
    QPainterPath path;
    // Top-left corner
    path.moveTo(r.left(), r.top() + CornerPointOffset);
    path.cubicTo(r.left(), r.top() + CornerControlOffset,
                 r.left() + CornerControlOffset, r.top(),
                 r.left() + CornerPointOffset, r.top());
    // Top-right corner
    path.lineTo(r.right() - CornerPointOffset, r.top());
    path.cubicTo(r.right() - CornerControlOffset, r.top(),
                 r.right(), r.top() + CornerControlOffset,
                 r.right(), r.top() + CornerPointOffset);
    // Bottom-right corner
    path.lineTo(r.right(), r.bottom() - CornerPointOffset);
    path.cubicTo(r.right(), r.bottom() - CornerControlOffset,
                 r.right() - CornerControlOffset, r.bottom(),
                 r.right() - CornerPointOffset, r.bottom());
    // Bottom-right corner
    path.lineTo(r.left() + CornerPointOffset, r.bottom());
    path.cubicTo(r.left() + CornerControlOffset, r.bottom(),
                 r.left(), r.bottom() - CornerControlOffset,
                 r.left(), r.bottom() - CornerPointOffset);
    path.lineTo(r.left(), r.top() + CornerPointOffset);
    return path;
}

void setLayoutItemMargins(int left, int top, int right, int bottom, QRect *rect,
                          Qt::LayoutDirection dir)
{
    if (dir == Qt::RightToLeft)
        rect->adjust(-right, top, -left, bottom);
    else
        rect->adjust(left, top, right, bottom);
}
}

TransparentStyle::TransparentStyle::TransparentStyle(QObject* parent)
{
    setParent(parent);
}

void TransparentStyle::polish(QWidget* w)
{
    if (qobject_cast<QMenu*>(w)
            || qobject_cast<QComboBoxPrivateContainer*>(w)
            || qobject_cast<QMdiSubWindow*>(w)) {
        w->setAttribute(Qt::WA_TranslucentBackground, true);
        w->setAutoFillBackground(false);
    }

    QCommonStyle::polish(w);

    if (qobject_cast<QScrollBar*>(w)) {
        w->setAttribute(Qt::WA_OpaquePaintEvent, false);
        w->setAttribute(Qt::WA_Hover, true);
        w->setMouseTracking(true);
    }
}

void TransparentStyle::unpolish(QWidget* w)
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

    QCommonStyle::unpolish(w);

    if (qobject_cast<QScrollBar*>(w)) {
        w->setAttribute(Qt::WA_OpaquePaintEvent, true);
        w->setAttribute(Qt::WA_Hover, false);
        w->setMouseTracking(false);
    }
}

QRect TransparentStyle::subElementRect(QStyle::SubElement element, const QStyleOption* option, const QWidget* widget) const
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
        rect = QCommonStyle::subElementRect(element, option, widget);
    }

    return rect;
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
            sc = QCommonStyle::hitTestComplexControl(control, cmb, point, widget);
            if (!cmb->editable && sc != QStyle::SC_None)
                sc = SC_ComboBoxArrow;  // A bit of a lie, but what we want
        } break;
    default:
        sc = QCommonStyle::hitTestComplexControl(control, option, point, widget);
        break;
    }

    return sc;
}

QSize TransparentStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption* option,
                                         const QSize& contentsSize, const QWidget* widget) const
{
    QSize sz(contentsSize);

    switch (type) {
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *mi
                = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
            int maxpmw = mi->maxIconWidth;
            const QComboBox *comboBox = qobject_cast<const QComboBox*>(widget);
            int w = sz.width(),
                    h = sz.height();
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                w = 10;
                h = 2; // Menu separator height
            } else {
                h = mi->fontMetrics.height() + 2;
                if (!mi->icon.isNull()) {
                    if (comboBox) {
                        const QSize &iconSize = comboBox->iconSize();
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
        if (!sz.isEmpty())
            sz += QSize(12, 4); // Constants from QWindowsStyle
        break;
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
            return sz;
        } break;
    case CT_Menu:
        if (proxy() == this) {
            sz = contentsSize;
        } else {
            QStyleHintReturnMask menuMask;
            QStyleOption myOption = *option;
            myOption.rect.setSize(sz);
            if (proxy()->styleHint(SH_Menu_Mask, &myOption, widget, &menuMask))
                sz = menuMask.region.boundingRect().size();
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
    case CT_ItemViewItem:
        if (const QStyleOptionViewItem *vopt
                = qstyleoption_cast<const QStyleOptionViewItem*>(option)) {
            sz = QCommonStyle::sizeFromContents(type, vopt, contentsSize, widget);
            sz.setHeight(sz.height() + 2);
        } break;
    default:
        sz = QCommonStyle::sizeFromContents(type, option, contentsSize, widget);
        break;
    }

    return sz;
}

QRect TransparentStyle::subControlRect(QStyle::ComplexControl control,
                                       const QStyleOptionComplex* option,
                                       QStyle::SubControl subControl, const QWidget* widget) const
{
    QRect ret;

    switch (control) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *combo
                = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            const auto editRect = comboboxEditBounds(adjustedControlFrame(combo->rect));
            switch (subControl) {
            case SC_ComboBoxEditField:
                ret = editRect.toAlignedRect();
                break;
            case SC_ComboBoxArrow:
                ret = editRect.toAlignedRect();
                ret.setX(ret.x() + ret.width());
                ret.setWidth(combo->rect.right() - ret.right());
                break;
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
                break;
            }
        } break;
    default:
        ret = QCommonStyle::subControlRect(control, option, subControl, widget);
        break;
    }

    return ret;
}

int TransparentStyle::styleHint(QStyle::StyleHint hint, const QStyleOption* option,
                                const QWidget* widget, QStyleHintReturn* returnData) const
{
    switch (hint) {
    case SH_ComboBox_AllowWheelScrolling:
        return false;
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
        return QCommonStyle::styleHint(hint, option, widget, returnData);
    }
}

int TransparentStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option,
                                  const QWidget* widget) const
{
    switch (metric) {
    case PM_ComboBoxFrameWidth:
    case PM_SpinBoxFrameWidth:
        return 0;
    case PM_MenuVMargin:
        return 4;
    case PM_MenuScrollerHeight:
        return 15;
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutVerticalSpacing:
        return -1;
    case PM_SmallIconSize:
        return int(QStyleHelper::dpiScaled(16.));
    case PM_LargeIconSize:
        return int(QStyleHelper::dpiScaled(32.));
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
        return QCommonStyle::pixelMetric(metric, option, widget);
    }
}

void TransparentStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option,
                                     QPainter* painter, const QWidget* widget) const
{
    switch (element) {
    case PE_PanelLineEdit:
        break; // Skip for transparent spinbox line edit
    case PE_PanelMenu: {
        painter->save();
        painter->fillRect(option->rect, Qt::transparent);
        painter->setPen(Qt::transparent);
        painter->setBrush(option->palette.window());
        painter->setRenderHint(QPainter::Antialiasing, true);
        const QPainterPath path = windowPanelPath(option->rect);
        painter->drawPath(path);
        painter->restore();
    } break;
    case PE_IndicatorMenuCheckMark: {
        painter->save();
        QColor pc;
        if (option->state & State_On)
            pc = option->palette.highlightedText().color();
        else
            pc = option->palette.text().color();

        QFont f = widget->font();
        f.setPixelSize(option->fontMetrics.height());

        painter->setPen(pc);
        painter->setFont(f);
        painter->drawText(option->rect.adjusted(-2, 1, -2, 1), "\u2713", QTextOption(Qt::AlignCenter));
        painter->restore();
    } break;
    case PE_IndicatorToolBarSeparator: {
        QPainterPath path;
        if (option->state & State_Horizontal) {
            int xpoint = option->rect.center().x();
            path.moveTo(xpoint + 0.5, option->rect.top() + 1);
            path.lineTo(xpoint + 0.5, option->rect.bottom());
        } else {
            int ypoint = option->rect.center().y();
            path.moveTo(option->rect.left() + 2 , ypoint + 0.5);
            path.lineTo(option->rect.right() + 1, ypoint + 0.5);
        }
        QPainterPathStroker theStroker;
        theStroker.setCapStyle(Qt::FlatCap);
        theStroker.setDashPattern(QVector<qreal>() << 1 << 2);
        path = theStroker.createStroke(path);
        painter->fillPath(path, QColor(0, 0, 0, 119));
    } break;
    default:
        QCommonStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

void TransparentStyle::drawControl(QStyle::ControlElement element, const QStyleOption* option,
                                   QPainter* painter, const QWidget* widget) const
{
    switch (element) {
    case CE_MenuItem:
    case CE_MenuHMargin:
    case CE_MenuVMargin:
    case CE_MenuTearoff:
    case CE_MenuScroller:
        if (const QStyleOptionMenuItem *mi
                = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
            painter->save();
            QWindow *window = widget && widget->window() ? widget->window()->windowHandle() :
                                                           /*QStyleHelper::styleObjectWindow(option->styleObject)*/0;
            const bool active = mi->state & State_Selected;
            if (active)
                painter->fillRect(mi->rect, mi->palette.highlight());
            if (element == CE_MenuTearoff) {
                painter->setPen(QPen(mi->palette.dark().color(), 1, Qt::DashLine));
                painter->drawLine(mi->rect.x() + 2, mi->rect.y() + mi->rect.height() / 2 - 1,
                                  mi->rect.x() + mi->rect.width() - 4,
                                  mi->rect.y() + mi->rect.height() / 2 - 1);
                painter->setPen(QPen(mi->palette.light().color(), 1, Qt::DashLine));
                painter->drawLine(mi->rect.x() + 2, mi->rect.y() + mi->rect.height() / 2,
                                  mi->rect.x() + mi->rect.width() - 4,
                                  mi->rect.y() + mi->rect.height() / 2);
            } else if (element == CE_MenuScroller) {
                const QSize scrollerSize = QSize(10, 8);
                const int scrollerVOffset = 5;
                const int left = mi->rect.x() + (mi->rect.width() - scrollerSize.width()) / 2;
                const int right = left + scrollerSize.width();
                int top;
                int bottom;
                if (option->state & State_DownArrow) {
                    bottom = mi->rect.y() + scrollerVOffset;
                    top = bottom + scrollerSize.height();
                } else {
                    bottom = mi->rect.bottom() - scrollerVOffset;
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
                break;
            }
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                const QRect separatorRect = QRect(mi->rect.left(), mi->rect.center().y(), mi->rect.width(), 2);
                painter->fillRect(separatorRect, "#D5D5D5");
                break;
            }
            const int maxpmw = mi->maxIconWidth;
            const bool enabled = mi->state & State_Enabled;
            int xpos = mi->rect.x() + 18;
            int checkcol = maxpmw;
            if (!enabled)
                painter->setPen(mi->palette.text().color());
            else if (active)
                painter->setPen(mi->palette.highlightedText().color());
            else
                painter->setPen(mi->palette.buttonText().color());
            if (mi->checked) {
                QStyleOption checkmarkOpt;
                checkmarkOpt.initFrom(widget);
                const int mw = checkcol + macItemFrame;
                const int mh = mi->rect.height() + macItemFrame;
                const int xp = mi->rect.x() + macItemFrame;
                checkmarkOpt.rect = QRect(xp, mi->rect.y() - checkmarkOpt.fontMetrics.descent(), mw, mh);
                checkmarkOpt.state.setFlag(State_On, active);
                checkmarkOpt.state.setFlag(State_Enabled, enabled);
                checkmarkOpt.state |= State_Small;
                // We let drawPrimitive(PE_IndicatorMenuCheckMark) pick the right color
                checkmarkOpt.palette.setColor(QPalette::HighlightedText, painter->pen().color());
                checkmarkOpt.palette.setColor(QPalette::Text, painter->pen().color());
                proxy()->drawPrimitive(PE_IndicatorMenuCheckMark, &checkmarkOpt, painter, widget);
            }
            if (!mi->icon.isNull()) {
                QIcon::Mode mode = (mi->state & State_Enabled) ? QIcon::Normal
                                                               : QIcon::Disabled;
                // Always be normal or disabled to follow the Mac style.
                int smallIconSize = proxy()->pixelMetric(PM_SmallIconSize);
                QSize iconSize(smallIconSize, smallIconSize);

                if (const QComboBox *comboBox = qobject_cast<const QComboBox*>(widget)) {
                    iconSize = comboBox->iconSize();
                }

                QPixmap pixmap = mi->icon.pixmap(window, iconSize, mode);
                int pixw = pixmap.width() / pixmap.devicePixelRatio();
                int pixh = pixmap.height() / pixmap.devicePixelRatio();
                QRect cr(xpos, mi->rect.y(), checkcol, mi->rect.height());
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(cr.center());
                painter->drawPixmap(pmr.topLeft(), pixmap);
                xpos += pixw + 6;
            }
            QString s = mi->text;
            const auto text_flags = Qt::AlignVCenter | Qt::TextHideMnemonic
                    | Qt::TextSingleLine | Qt::AlignAbsolute;
            int yPos = mi->rect.y();
            const bool isSubMenu = mi->menuItemType == QStyleOptionMenuItem::SubMenu;
            const int tabwidth = isSubMenu ? 9 : mi->tabWidth;
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
                int xp = mi->rect.right() - tabwidth - macRightBorder + 2;
                if (!isSubMenu)
                    xp -= macItemHMargin + macItemFrame + 3; // Adjust for shortcut
                painter->drawText(xp, yPos, tabwidth, mi->rect.height(), text_flags | Qt::AlignRight, rightMarginText);
            }
            if (!s.isEmpty()) {
                const int xm = macItemFrame + maxpmw + macItemHMargin;
                QFont myFont = mi->font;
                myFont.setPointSizeF(QFontInfo(mi->font).pointSizeF());
                painter->setFont(myFont);
                painter->drawText(xpos, yPos, mi->rect.width() - xm - tabwidth + 1,
                                  mi->rect.height(), text_flags, s);
            }
            painter->restore();
        } break;
    case CE_ComboBoxLabel:
        if (const auto *cb
                = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            auto comboCopy = *cb;
            comboCopy.direction = Qt::LeftToRight;
            // The rectangle will be adjusted to SC_ComboBoxEditField with comboboxEditBounds()
            QCommonStyle::drawControl(CE_ComboBoxLabel, &comboCopy, painter, widget);
        } break;
    default:
        QCommonStyle::drawControl(element, option, painter, widget);
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
                copy.rect.adjust(1, 1, -2, 1);
                proxy()->drawPrimitive(pe, &copy, painter, widget);
            }
            if (so->subControls & SC_SpinBoxDown) {
                copy.subControls = SC_SpinBoxDown;
                copy.state = so->state;
                QPalette pal2 = so->palette;
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
                copy.rect.adjust(1, -1, -2, -1);
                proxy()->drawPrimitive(pe, &copy, painter, widget);
            }
            painter->restore();
        } break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb
                = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            painter->save();
            if ((cmb->subControls & SC_ComboBoxFrame) && (cmb->state & State_MouseOver))
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
                arrowOpt.rect = ar.adjusted(3, 1, 2, -1);
                arrowOpt.state = flags;
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, painter, widget);
            }
            painter->restore();
        } break;
    default:
        QCommonStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}