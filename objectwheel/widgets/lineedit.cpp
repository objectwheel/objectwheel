#include <lineedit.h>
#include <paintutils.h>
#include <private/qlineedit_p.h>
#include <QStylePainter>

const int QLineEditPrivate::verticalMargin(1);
const int QLineEditPrivate::horizontalMargin(2);

static int effectiveTextMargin(int defaultMargin, const QLineEditPrivate::SideWidgetEntryList &widgets,
                               const QLineEditPrivate::SideWidgetParameters &parameters)
{
    if (widgets.empty())
        return defaultMargin;
    return defaultMargin + (parameters.margin + parameters.widgetWidth) *
           int(std::count_if(widgets.begin(), widgets.end(),
                             [](const QLineEditPrivate::SideWidgetEntry &e) {
                                 return e.widget->isVisibleTo(e.widget->parentWidget()); }));
}
int QLineEditPrivate::effectiveLeftTextMargin() const
{
    return effectiveTextMargin(leftTextMargin, leftSideWidgetList(), sideWidgetParameters());
}
int QLineEditPrivate::effectiveRightTextMargin() const
{
    return effectiveTextMargin(rightTextMargin, rightSideWidgetList(), sideWidgetParameters());
}

QLineEditPrivate::SideWidgetParameters QLineEditPrivate::sideWidgetParameters() const
{
    Q_Q(const QLineEdit);
    SideWidgetParameters result;
    result.iconSize = q->style()->pixelMetric(QStyle::PM_SmallIconSize, 0, q);
    result.margin = result.iconSize / 4;
    result.widgetWidth = result.iconSize + 2;
    result.widgetHeight = result.iconSize + 2;
    return result;
}

/* LineEdit: This widget should never have initial focus
 * (ie, be the first widget of a dialog, else, the hint cannot be displayed.
 * As it is the only focusable control in the widget box, it clears the focus
 * policy and focusses explicitly on click only (note that setting Qt::ClickFocus
 * is not sufficient for that as an ActivationFocus will occur). */

LineEdit::LineEdit(QWidget* parent) : QLineEdit(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

void LineEdit::paintEvent(QPaintEvent*)
{
    Q_D(QLineEdit);

    QStylePainter p(this);
    QPalette pal = palette();
    p.setRenderHint(QPainter::Antialiasing);

    // Draw background
    QStyleOptionFrame option;
    initStyleOption(&option);
    // Draw outline
    QPainterPath outlinePath;
    outlinePath.addRoundedRect(option.rect, 4, 4);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#b1b1b1"));
    p.drawPath(outlinePath);
    // Draw inline
    QPainterPath inlinePath;
    inlinePath.addRoundedRect(QRectF(option.rect).adjusted(0.5, 0.5, -0.5, -0.5), 3.5, 3.5);
    p.setBrush(QColor("#f0f0f0"));
    p.drawPath(inlinePath);
    // Draw body
    QPainterPath bodyPath;
    bodyPath.addRoundedRect(QRectF(option.rect).adjusted(1, 1, -1, -1), 3, 3);
    p.setBrush(QColor("#ffffff"));
    p.drawPath(bodyPath);

    // Draw text and selection
    QRect r = style()->subElementRect(QStyle::SE_LineEditContents, &option, this);
    r.setX(r.x() + d->effectiveLeftTextMargin());
    r.setY(r.y() + d->topTextMargin);
    r.setRight(r.right() - d->effectiveRightTextMargin());
    r.setBottom(r.bottom() - d->bottomTextMargin);
    p.setClipRect(r);
    QFontMetrics fm = fontMetrics();
    Qt::Alignment va = QStyle::visualAlignment(d->control->layoutDirection(), QFlag(d->alignment));
    switch (va & Qt::AlignVertical_Mask) {
     case Qt::AlignBottom:
         d->vscroll = r.y() + r.height() - fm.height() - d->verticalMargin;
         break;
     case Qt::AlignTop:
         d->vscroll = r.y() + d->verticalMargin;
         break;
     default:
         //center
         d->vscroll = r.y() + (r.height() - fm.height() + 1) / 2;
         break;
    }
    QRect lineRect(r.x() + d->horizontalMargin, d->vscroll, r.width() - 2*d->horizontalMargin, fm.height());
    if (d->shouldShowPlaceholderText()) {
        if (!d->placeholderText.isEmpty()) {
            const Qt::LayoutDirection layoutDir = d->placeholderText.isRightToLeft() ? Qt::RightToLeft : Qt::LeftToRight;
            const Qt::Alignment alignPhText = QStyle::visualAlignment(layoutDir, QFlag(d->alignment));
            const QColor col = pal.placeholderText().color();
            QPen oldpen = p.pen();
            p.setPen(col);
            Qt::LayoutDirection oldLayoutDir = p.layoutDirection();
            p.setLayoutDirection(layoutDir);
            const QString elidedText = fm.elidedText(d->placeholderText, Qt::ElideRight, lineRect.width());
            p.drawText(lineRect, alignPhText, elidedText);
            p.setPen(oldpen);
            p.setLayoutDirection(oldLayoutDir);
        }
    }
    int cix = qRound(d->control->cursorToX());
    // horizontal scrolling. d->hscroll is the left indent from the beginning
    // of the text line to the left edge of lineRect. we update this value
    // depending on the delta from the last paint event; in effect this means
    // the below code handles all scrolling based on the textline (widthUsed),
    // the line edit rect (lineRect) and the cursor position (cix).
    int widthUsed = qRound(d->control->naturalTextWidth()) + 1;
    if (widthUsed <= lineRect.width()) {
        // text fits in lineRect; use hscroll for alignment
        switch (va & ~(Qt::AlignAbsolute|Qt::AlignVertical_Mask)) {
        case Qt::AlignRight:
            d->hscroll = widthUsed - lineRect.width() + 1;
            break;
        case Qt::AlignHCenter:
            d->hscroll = (widthUsed - lineRect.width()) / 2;
            break;
        default:
            // Left
            d->hscroll = 0;
            break;
        }
    } else if (cix - d->hscroll >= lineRect.width()) {
        // text doesn't fit, cursor is to the right of lineRect (scroll right)
        d->hscroll = cix - lineRect.width() + 1;
    } else if (cix - d->hscroll < 0 && d->hscroll < widthUsed) {
        // text doesn't fit, cursor is to the left of lineRect (scroll left)
        d->hscroll = cix;
    } else if (widthUsed - d->hscroll < lineRect.width()) {
        // text doesn't fit, text document is to the left of lineRect; align
        // right
        d->hscroll = widthUsed - lineRect.width() + 1;
    } else {
        //in case the text is bigger than the lineedit, the hscroll can never be negative
        d->hscroll = qMax(0, d->hscroll);
    }
    // the y offset is there to keep the baseline constant in case we have script changes in the text.
    QPoint topLeft = lineRect.topLeft() - QPoint(d->hscroll, d->control->ascent() - fm.ascent());
    // draw text, selections and cursors
#ifndef QT_NO_STYLE_STYLESHEET // FIXME
//    if (QStyleSheetStyle* cssStyle = qt_styleSheet(style())) {
//        cssStyle->styleSheetPalette(this, &option, &pal);
//    }
#endif
    p.setPen(pal.text().color());
    int flags = QWidgetLineControl::DrawText;
#ifdef QT_KEYPAD_NAVIGATION
    if (!QApplication::keypadNavigationEnabled() || hasEditFocus())
#endif
    if (d->control->hasSelectedText() || (d->cursorVisible && !d->control->inputMask().isEmpty() && !d->control->isReadOnly())){
        flags |= QWidgetLineControl::DrawSelections;
        // Palette only used for selections/mask and may not be in sync
        if (d->control->palette() != pal
           || d->control->palette().currentColorGroup() != pal.currentColorGroup())
            d->control->setPalette(pal);
    }
    // Asian users see an IM selection text as cursor on candidate
    // selection phase of input method, so the ordinary cursor should be
    // invisible if we have a preedit string.
    if (d->cursorVisible && !d->control->isReadOnly())
        flags |= QWidgetLineControl::DrawCursor;
    d->control->setCursorWidth(style()->pixelMetric(QStyle::PM_TextCursorWidth));
    d->control->draw(&p, topLeft, r, flags);
}

void LineEdit::focusInEvent(QFocusEvent* event)
{
    // Refuse the focus if the mouse it outside. In addition to the mouse
    // press logic, this prevents a re-focussing which occurs once
    // we actually had focus
    const Qt::FocusReason reason = event->reason();
    if (reason == Qt::ActiveWindowFocusReason || reason == Qt::PopupFocusReason) {
        const QPoint mousePos = mapFromGlobal(QCursor::pos());
        const bool refuse = !geometry().contains(mousePos);
        if (refuse) {
            event->ignore();
            return;
        }
    }
    QLineEdit::focusInEvent(event);
}

void LineEdit::mousePressEvent(QMouseEvent* event)
{
    if (!hasFocus()) // Explicitly focus on click.
        setFocus(Qt::OtherFocusReason);
    QLineEdit::mousePressEvent(event);
}

QSize LineEdit::sizeHint() const
{
    return QSize(QLineEdit::sizeHint().width(), 20);
}

QSize LineEdit::minimumSizeHint() const
{
    return QSize(QLineEdit::minimumSizeHint().width(), 20);
}
