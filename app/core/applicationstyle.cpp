#include <applicationstyle.h>
#include <utilityfunctions.h>

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

#include <private/qapplication_p.h>
#include <private/qcombobox_p.h>

namespace {

const int macItemFrame          = 2;    // menu item frame width
const int macItemHMargin        = 3;    // menu item hor text margin
const int macRightBorder        = 12;   // right border on mac

const qreal comboBoxDefaultHeight[3] = {
    26, 22, 19
};

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

}

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

ApplicationStyle::ApplicationStyle() : QProxyStyle("fusion")
{
    Q_ASSERT(QStyleFactory::keys().contains("fusion", Qt::CaseInsensitive));
}

QSize ApplicationStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption* option, const QSize& contentsSize, const QWidget* widget) const
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
    case CT_ItemViewItem:
        if (const QStyleOptionViewItem *vopt
                = qstyleoption_cast<const QStyleOptionViewItem*>(option)) {
            sz = QProxyStyle::sizeFromContents(type, vopt, contentsSize, widget);
            sz.setHeight(sz.height() + 2);
        } break;
    default:
        sz = QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
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
        rect = QProxyStyle::subElementRect(element, option, widget);
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
                ret = QProxyStyle::subControlRect(control, option, subControl, widget);
                break;
            }
        } break;

    default:
        ret = QProxyStyle::subControlRect(control, option, subControl, widget);
        break;
    }

    return ret;
}

int ApplicationStyle::styleHint(QStyle::StyleHint hint, const QStyleOption* option,
                                const QWidget* widget, QStyleHintReturn* returnData) const
{
    switch (hint) {
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
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
}

int ApplicationStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    switch (metric) {
    case PM_ToolTipLabelFrameWidth:
        return 1;
    case PM_MenuVMargin:
        return 4;
    case PM_MenuScrollerHeight:
        return 15;
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
    case PE_PanelMenu: {
        painter->save();
        painter->setPen(option->palette.text().color().lighter(170));
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

        QFont f = widget->font();
        f.setPixelSize(option->fontMetrics.height());

        painter->setPen(pc);
        painter->setFont(f);
        painter->drawText(option->rect.adjusted(-2, 1, -2, 1), "\u2713", QTextOption(Qt::AlignCenter));
        painter->restore();
    } break;
    default:
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

void ApplicationStyle::drawControl(QStyle::ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
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
            if (!mi.icon.isNull()) {
                QIcon::Mode mode = (mi.state & State_Enabled) ? QIcon::Normal
                                                              : QIcon::Disabled;
                // Always be normal or disabled to follow the Mac style.
                int smallIconSize = proxy()->pixelMetric(PM_SmallIconSize);
                QSize iconSize(smallIconSize, smallIconSize);

                if (const QComboBox *comboBox = qobject_cast<const QComboBox*>(widget)) {
                    iconSize = comboBox->iconSize();
                }

                QWindow* window = nullptr;
                if (widget) {
                    Q_ASSERT(UtilityFunctions::window(widget));
                    window = UtilityFunctions::window(widget);
                }
                QPixmap pixmap = mi.icon.pixmap(window, iconSize, mode);
                int pixw = pixmap.width() / pixmap.devicePixelRatioF();
                int pixh = pixmap.height() / pixmap.devicePixelRatioF();
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
            QProxyStyle::drawControl(CE_ComboBoxLabel, &copy, painter, widget);
        } break;
    default:
        QProxyStyle::drawControl(element, option, painter, widget);
        break;
    }
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
                    wd->setStyleSheet("background: transparent;");
            }
            class A : public QObject {
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

    QProxyStyle::polish(w);

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

    QProxyStyle::unpolish(w);

    if (qobject_cast<QScrollBar*>(w)) {
        w->setAttribute(Qt::WA_OpaquePaintEvent, true);
        w->setAttribute(Qt::WA_Hover, false);
        w->setMouseTracking(false);
    }
}
