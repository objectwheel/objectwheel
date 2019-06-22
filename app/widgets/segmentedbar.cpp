#include <segmentedbar.h>
#include <paintutils.h>

#include <QLayout>
#include <QStyleOption>
#include <QStylePainter>
#include <QToolButton>
#include <QAction>
#include <QActionEvent>
#include <QMenu>
#include <QGuiApplication>
#include <QScreen>

static const char isDownProperty[] = "__SegmentedBar_isDown";
static const char originalCursorShapeProperty[] = "__SegmentedBar_originalCursorShape";

SegmentedBar::SegmentedBar(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setIconSize(QSize());
    setAttribute(Qt::WA_Hover);
}

void SegmentedBar::clear()
{
    const QList<QAction*>& actions = this->actions();
    for(int i = 0; i < actions.size(); ++i)
        removeAction(actions.at(i));
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
    if (index >= 0 && actions().size() > index)
        return actions().at(index);

    return nullptr;
}

QAction* SegmentedBar::addAction(const QIcon& icon)
{
    auto action = new QAction(this);
    action->setIcon(icon);
    QWidget::addAction(action);
    return action;
}

QAction* SegmentedBar::addAction(const QString& text)
{
    auto action = new QAction(this);
    action->setText(text);
    QWidget::addAction(action);
    return action;
}

QAction* SegmentedBar::addAction(const QString& text, const QIcon& icon)
{
    auto action = new QAction(this);
    action->setText(text);
    action->setIcon(icon);
    QWidget::addAction(action);
    return action;
}

QSize SegmentedBar::sizeHint() const
{
    const int preferredCellWidth = 2 * iconSize().width() + 3;
    return QSize(qMax(actions().size() * preferredCellWidth, minimumSizeHint().width()), 24);
}

QSize SegmentedBar::minimumSizeHint() const
{
    return QSize(qMax(24, actions().size() * 24), 24);
}

qreal SegmentedBar::cellWidth() const
{
    return width() / qreal(actions().size());
}

QPoint SegmentedBar::adjustedMenuPosition(QAction* action)
{
    QStyleOptionButton option;
    initStyleOption(action, &option);
    QRect rect = option.rect;
    QSize menuSize = action->menu()->sizeHint();
    QPoint globalPos = mapToGlobal(rect.topLeft());
    int x = globalPos.x();
    int y = globalPos.y();
    const QRect availableGeometry = QGuiApplication::primaryScreen()->availableGeometry();
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
    const int i = actions().indexOf(action);
    option->rect = QRect(i * cellWidth(), 0, cellWidth(), height());
    option->direction = layoutDirection();
    option->palette = palette();
    option->fontMetrics = fontMetrics();
    option->styleObject = action;
    option->features = QStyleOptionButton::None;
    option->state = QStyle::State_None;
    if (action->isEnabled())
        option->state |= QStyle::State_Enabled;
    if (option->rect.contains(mapFromGlobal(QCursor::pos())))
        option->state |= QStyle::State_MouseOver;
    if (window()->isActiveWindow())
        option->state |= QStyle::State_Active;
    if (action->menu())
        option->features |= QStyleOptionButton::HasMenu;
    if (action->property(isDownProperty).toBool() || (action->menu() && action->menu()->isVisible()))
        option->state |= QStyle::State_Sunken;
    else
        option->state |= QStyle::State_Raised;
    if (action->isChecked())
        option->state |= QStyle::State_On;
    option->text = action->text();
    option->icon = action->icon();
    option->iconSize = iconSize();
}

void SegmentedBar::mousePressEvent(QMouseEvent* event)
{
    event->ignore();

    if (event->button() != Qt::LeftButton)
        return;

    if (QAction* action = actionAt(event->pos())) {
        if (action->isEnabled()) {
            action->setProperty(isDownProperty, true);
            repaint();
            event->accept();
        }
    }
}

void SegmentedBar::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();

    if (QAction* action = actionAt(event->pos())) {
        if (action->property(isDownProperty).toBool()) {
            if (action->menu())
                action->menu()->exec(adjustedMenuPosition(action));
            if (!action->menu() && action->isCheckable())
                action->setChecked(!action->isChecked());
            emit actionTriggered(action);
            emit action->triggered(action->isChecked());
        }
    }

    for (QAction* action : actions())
        action->setProperty(isDownProperty, false);

    repaint();
}

void SegmentedBar::actionEvent(QActionEvent* event)
{
    switch (event->type()) {
    case QEvent::ActionAdded:
    case QEvent::ActionChanged:
    case QEvent::ActionRemoved:
        updateGeometry();
        update();
        break;
    default:
        Q_ASSERT_X(false, "SegmentedBar::actionEvent", "internal error");
    }
}

void SegmentedBar::paintEvent(QPaintEvent*)
{
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipping(true);

    for (QAction* action : actions()) {
        QStyleOptionButton option;
        initStyleOption(action, &option);

        // Draw background
        const QRectF r(option.rect);
        painter.setClipRect(r);
        painter.setFont(action->font());
        option.rect = rect();
        PaintUtils::drawPanelButtonBevel(&painter, option, false);
        option.rect = r.toRect();

        // Draw menu indicator, which is cropped by "clip rect"
        if (action != actions().last()
                && (option.features & QStyleOptionButton::HasMenu)
                && ((option.state & QStyle::State_MouseOver) || (option.state & QStyle::State_Sunken))) {
            int mbi = style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &option);
            QRectF ir = option.rect;
            QStyleOptionButton newBtn = option;
            newBtn.rect = QRect(ir.right() - mbi, ir.bottom() - mbi, mbi, mbi);
            style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &newBtn, &painter);
        }

        // Draw label (icon, texts etc..)
        painter.drawControl(QStyle::CE_PushButtonLabel, option);

        // Draw separators
        if (action != actions().last()) {
            if (option.state & QStyle::State_Sunken)
                painter.setPen("#22000000");
            else
                painter.setPen("#18000000");
            painter.drawLine(r.topRight() + QPointF(-0.5, 1.5),
                             r.bottomRight() - QPointF(0.5, 1.5));
        }
    }
}

bool SegmentedBar::event(QEvent* event)
{
    if (event->type() == QEvent::HoverMove) {
        auto e = static_cast<QHoverEvent*>(event);
        if (QAction* action = actionAt(e->pos())) {
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

    if (event->type() == QEvent::ToolTip) {
        auto e = static_cast<QHelpEvent*>(event);
        QAction* action = actionAt(e->pos());
        if (action)
            setToolTip(action->toolTip());
    }

    return QWidget::event(event);
}
