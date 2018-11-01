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

#include <private/qapplication_p.h>
#include <private/qcombobox_p.h>

namespace {

const int macItemFrame          = 2;    // menu item frame width
const int macItemHMargin        = 3;    // menu item hor text margin
const int macRightBorder        = 12;   // right border on mac

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
}

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
        if (const QStyleOptionMenuItem *mi
                = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
            painter->save();
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
                painter->restore();
                break;
            }
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                const QRect separatorRect = QRect(mi->rect.left(), mi->rect.center().y(), mi->rect.width(), 2);
                painter->fillRect(separatorRect, "#D5D5D5");
                painter->restore();
                break;
            }
            const int maxpmw = 20/*mi->maxIconWidth*/ ;
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

                QWindow* window = nullptr;
                if (widget) {
                    Q_ASSERT(UtilityFunctions::window(widget));
                    window = UtilityFunctions::window(widget);
                }
                QPixmap pixmap = mi->icon.pixmap(window, iconSize, mode);
                int pixw = pixmap.width() / pixmap.devicePixelRatioF();
                int pixh = pixmap.height() / pixmap.devicePixelRatioF();
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
