#include <segmentedbar.h>
#include <private/qwidget_p.h>
#include <delayer.h>

#include <QStyleOption>
#include <QStylePainter>
#include <QAction>
#include <QActionEvent>
#include <QMenu>
#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>

// TODO: This class is incomplete
static const char downProperty[] = "_q_SegmentedBar_down";
static const char animatedActionProperty[] = "_q_SegmentedBar_animatedAction";
static const char originalCursorShapeProperty[] = "_q_SegmentedBar_originalCursorShape";

SegmentedBar::SegmentedBar(QWidget* parent) : QWidget(parent)
  , m_isActionTriggerDisabled(false)
{
    setIconSize(QSize());
    // Used to enable hover events on the widget, which
    // makes calls to update() function (defined in QWidget)
    // Thus we can paint menu indicator whenever a mouse hovers
    // Normally, for instance, for QAbstractButton, this
    // attribute is enabled in ie QFusionStyle::polish, so
    // it is hardcoded and in order to have the same effect
    // on our custom button we gotta enable it here.
    setAttribute(Qt::WA_Hover);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void SegmentedBar::clear()
{
    QList<QAction*> actions = this->actions();
    for(QAction* action : actions)
        removeAction(action);
}

void SegmentedBar::click(QAction* action)
{
    if (!isEnabled())
        return;

    if (actions().contains(action)) {
        if (!action->isEnabled())
            return;
        bool changeState = true;
        if (action->isChecked()
                && action->actionGroup()
                && action->actionGroup()->checkedAction() == action
                && action->actionGroup()->isExclusive()) {
            // the checked button of an exclusive or autoexclusive group cannot be unchecked
            changeState = false;
        }
        if (action->menu())
            action->menu()->exec(adjustedMenuPosition(action));
        if (changeState && action->isCheckable())
            action->setChecked(!action->isChecked());
        action->setProperty(downProperty, false);
        emit actionTriggered(action);
        m_isActionTriggerDisabled = true;
        emit action->triggered(action->isChecked());
        m_isActionTriggerDisabled = false;
        update();
    }
}

void SegmentedBar::animateClick(QAction* action, int msec)
{
    if (!isEnabled())
        return;
    if (!action->isEnabled())
        return;
    action->setProperty(downProperty, true);
    setProperty(animatedActionProperty, qintptr(action));
    repaint();
    m_animateTimer.start(msec, this);
}

QSize SegmentedBar::iconSize() const
{
    return m_iconSize;
}

void SegmentedBar::setIconSize(const QSize& iconSize)
{
    QSize sz = iconSize;
    if (!sz.isValid()) {
        const int metric = style()->pixelMetric(QStyle::PM_ToolBarIconSize, 0, this);
        sz = QSize(metric, metric);
    }
    if (m_iconSize != sz) {
        m_iconSize = sz;
        setMinimumSize(0, 0);
        updateGeometry();
        update();
        emit iconSizeChanged(m_iconSize);
    }
}

QAction* SegmentedBar::actionAt(int x, int y) const
{
    return actionAt({x, y});
}

QAction* SegmentedBar::actionAt(const QPoint& p) const
{
    if (p.y() < 0)
        return nullptr;

    if (p.y() >= height())
        return nullptr;

    if (p.x() < 0)
        return nullptr;

    if (p.x() >= width())
        return nullptr;

    const int index = p.x() / cellWidth();
    if (index >= 0 && visibleActionCount() > index)
        return visibleActions().at(index);

    return nullptr;
}

QAction* SegmentedBar::addAction(const QIcon& icon)
{
    return addAction(QString(), icon);
}

QAction* SegmentedBar::addAction(const QString& text)
{
    auto action = new QAction(text, this);
    addAction(action);
    return action;
}

QAction* SegmentedBar::addAction(const QString& text, const QIcon& icon)
{
    auto action = new QAction(icon, text, this);
    addAction(action);
    return action;
}

QSize SegmentedBar::sizeHint() const
{
    ensurePolished(); // Since we make calcs based on text width
    int preferredCellWidth = iconSize().width() + 21;
    int textWidth = 0;
    for (const QAction* action : visibleActions()) {
        if (!action->text().isEmpty()) {
            int w = QFontMetrics(action->font()).horizontalAdvance(action->text());
            if (w > textWidth)
                textWidth = w;
        }
    }
    if (textWidth > 0)
        preferredCellWidth += textWidth + 3;
    return QSize(qMax(visibleActionCount() * preferredCellWidth, 24), 24);
}

QSize SegmentedBar::minimumSizeHint() const
{
    return sizeHint();
}

qreal SegmentedBar::cellWidth() const
{
    return width() / qreal(visibleActionCount());
}

int SegmentedBar::visibleActionCount() const
{
    int i = 0;
    for (const QAction* action : actions()) {
        if (action->isVisible())
            i++;
    }
    return i;
}

QList<QAction*> SegmentedBar::visibleActions() const
{
    QList<QAction*> visibleActions;
    for (QAction* action : actions()) {
        if (action->isVisible())
            visibleActions.append(action);
    }
    return visibleActions;
}

QPoint SegmentedBar::adjustedMenuPosition(QAction* action) const
{
    QStyleOptionButton option;
    initStyleOption(action, &option);
    QRect rect = option.rect;
    QSize menuSize = action->menu()->sizeHint();
    QPoint globalPos = mapToGlobal(rect.topLeft());
    int x = globalPos.x();
    int y = globalPos.y();
    QRect availableGeometry = QWidgetPrivate::screenGeometry(this);
    if (availableGeometry.isNull())
        availableGeometry = QApplication::desktop()->availableGeometry(this);
    if (y + rect.height() + menuSize.height() <= availableGeometry.bottom())
        y += rect.height();
    else if (y - menuSize.height() >= availableGeometry.y())
        y -= menuSize.height();
    if (layoutDirection() == Qt::RightToLeft)
        x += rect.width() - menuSize.width();
    return QPoint(x,y);
}

void SegmentedBar::initStyleOption(QAction* action, QStyleOptionButton* option) const
{
    option->rect = QRect(visibleActions().indexOf(action) * cellWidth(), 0, cellWidth(), height());
    option->direction = layoutDirection();
    option->palette = palette();
    option->fontMetrics = QFontMetrics(action->font());
    option->iconSize = iconSize();
    option->icon = action->icon();
    option->text = action->text();
    option->styleObject = action;
    option->features = QStyleOptionButton::None;
    option->state = QStyle::State_None;
    if (isEnabled() && action->isEnabled())
        option->state |= QStyle::State_Enabled;
    if (option->rect.contains(mapFromGlobal(QCursor::pos())))
        option->state |= QStyle::State_MouseOver;
    if (window()->isActiveWindow())
        option->state |= QStyle::State_Active;
    if (action->menu())
        option->features |= QStyleOptionButton::HasMenu;
    if (action->property(downProperty).toBool() || (action->menu() && action->menu()->isVisible()))
        option->state |= QStyle::State_Sunken;
    else
        option->state |= QStyle::State_Raised;
    if (action->isChecked())
        option->state |= QStyle::State_On;
}

void SegmentedBar::mousePressEvent(QMouseEvent* event)
{
    event->ignore();

    if (event->button() != Qt::LeftButton)
        return;

    if (QAction* action = actionAt(event->pos())) {
        if (action->isEnabled()) {
            action->setProperty(downProperty, true);
            repaint();
            event->accept();
        }
    }
}

void SegmentedBar::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();

    if (QAction* action = actionAt(event->pos())) {
        if (action->property(downProperty).toBool())
            click(action);
    }

    for (QAction* action : visibleActions())
        action->setProperty(downProperty, false);

    repaint();
}

void SegmentedBar::actionEvent(QActionEvent* event)
{
    QAction* action = event->action();
    switch (event->type()) {
    case QEvent::ActionAdded:
        connect(action, &QAction::triggered, this, [=] {
            if (!m_isActionTriggerDisabled) {
                action->setProperty(downProperty, true);
                repaint();
                actionTriggered(action);
                Delayer::delay(100); // FIXME: Improve this
                action->setProperty(downProperty, false);
                update();
            }
        });
        updateGeometry();
        update();
        break;
    case QEvent::ActionRemoved:
        action->disconnect(this);
        updateGeometry();
        update();
        break;
    case QEvent::ActionChanged:
        updateGeometry();
        update();
        break;
    default:
        Q_ASSERT_X(false, "SegmentedBar::actionEvent", "internal error");
    }
}

void SegmentedBar::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_animateTimer.timerId()) {
        m_animateTimer.stop();
        click((QAction*) property(animatedActionProperty).value<qintptr>());
    } else {
        QWidget::timerEvent(event);
    }
}

void SegmentedBar::paintEvent(QPaintEvent*)
{
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipping(true);

    for (QAction* action : visibleActions()) {
        QStyleOptionButton option;
        initStyleOption(action, &option);
        painter.setFont(action->font());
        painter.setClipRect(option.rect);

        // Draw background
        QStyleOptionButton copy(option);
        copy.rect = rect();
        style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &copy, &painter, this);

        // Draw menu indicator, which is cropped by "clip rect"
        if (action != visibleActions().last()
                && option.features & QStyleOptionButton::HasMenu
                && (option.state & QStyle::State_MouseOver || option.state & QStyle::State_Sunken)) {
            int mbi = style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &option);
            QRectF ir = option.rect;
            QStyleOptionButton newBtn = option;
            newBtn.rect = QRect(ir.right() - mbi, ir.bottom() - mbi, mbi, mbi);
            painter.drawPrimitive(QStyle::PE_IndicatorArrowDown, newBtn);
        }

        // Draw label (icon, texts etc..)
        painter.drawControl(QStyle::CE_PushButtonLabel, option);

        // Draw separators
        if (action != visibleActions().last()) {
            if (option.state & QStyle::State_Sunken)
                painter.setPen("#22000000");
            else
                painter.setPen("#18000000");
            painter.drawLine(QRectF(option.rect).topRight() + QPointF(-0.5, 1.5),
                             QRectF(option.rect).bottomRight() - QPointF(0.5, 1.5));
        }
    }
}

bool SegmentedBar::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::HoverMove: {
        if (isEnabled()) {
            const auto e = static_cast<QHoverEvent*>(event);
            if (const QAction* action = actionAt(e->pos())) {
                if (!action->isEnabled() && cursor().shape() != Qt::ArrowCursor) {
                    setProperty(originalCursorShapeProperty, int(cursor().shape()));
                    setCursor(Qt::ArrowCursor);
                } else if (action->isEnabled()) {
                    bool ok = false;
                    Qt::CursorShape recentCursorShape
                            = Qt::CursorShape(property(originalCursorShapeProperty).toInt(&ok));
                    if (ok && recentCursorShape != cursor().shape())
                        setCursor(recentCursorShape);
                }
            }
            update();
        }
    } break;

    case QEvent::ToolTip: {
        const auto e = static_cast<QHelpEvent*>(event);
        if (const QAction* action = actionAt(e->pos()))
            setToolTip(action->toolTip());
    } break;

    default:
        break;
    }

    return QWidget::event(event);
}
