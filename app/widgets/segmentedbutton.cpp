#include <segmentedbutton.h>
#include <paintutils.h>

#include <QLayout>
#include <QStyleOption>
#include <QStylePainter>
#include <QToolButton>
#include <QAction>
#include <QActionEvent>

static const char isDownProperty[] = "__SegmentedButton_isDown";

SegmentedButton::SegmentedButton(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setIconSize(QSize());
}

void SegmentedButton::clear()
{
    const QList<QAction*>& actions = this->actions();
    for(int i = 0; i < actions.size(); ++i)
        removeAction(actions.at(i));
}

QSize SegmentedButton::iconSize() const
{
    return m_iconSize;
}

void SegmentedButton::setIconSize(const QSize& iconSize)
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

QAction* SegmentedButton::actionAt(int x, int y) const
{
    return actionAt({x, y});
}

QAction* SegmentedButton::actionAt(const QPoint& p) const
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

QAction* SegmentedButton::addAction(const QIcon& icon)
{
    auto action = new QAction(this);
    action->setIcon(icon);
    QWidget::addAction(action);
    return action;
}

QAction* SegmentedButton::addAction(const QString& text)
{
    auto action = new QAction(this);
    action->setText(text);
    QWidget::addAction(action);
    return action;
}

QAction* SegmentedButton::addAction(const QString& text, const QIcon& icon)
{
    auto action = new QAction(this);
    action->setText(text);
    action->setIcon(icon);
    QWidget::addAction(action);
    return action;
}

QSize SegmentedButton::sizeHint() const
{
    const int preferredCellWidth = 2 * iconSize().width() + 3;
    return QSize(qMax(actions().size() * preferredCellWidth, minimumSizeHint().width()), 24);
}

QSize SegmentedButton::minimumSizeHint() const
{
    return QSize(qMax(24, actions().size() * 24), 24);
}

qreal SegmentedButton::cellWidth() const
{
    return width() / qreal(actions().size());
}

void SegmentedButton::mousePressEvent(QMouseEvent* event)
{
    event->ignore();

    if (event->button() != Qt::LeftButton)
        return;

    if (QAction* action = actionAt(event->pos())) {
        action->setProperty(isDownProperty, true);
        repaint();
        event->accept();
    }
}

void SegmentedButton::mouseReleaseEvent(QMouseEvent* event)
{
    event->ignore();

    if (QAction* action = actionAt(event->pos())) {
        if (action->property(isDownProperty).toBool()) {
            event->accept();
            if (action->isCheckable())
                action->setChecked(!action->isChecked());
            repaint();
            emit actionTriggered(action);
            emit action->triggered(action->isChecked());
        }
    }

    for (QAction* action : actions())
        action->setProperty(isDownProperty, false);
}

void SegmentedButton::actionEvent(QActionEvent* event)
{
    switch (event->type()) {
    case QEvent::ActionAdded:
    case QEvent::ActionChanged:
    case QEvent::ActionRemoved:
        updateGeometry();
        update();
        break;
    default:
        Q_ASSERT_X(false, "SegmentedButton::actionEvent", "internal error");
    }
}

void SegmentedButton::paintEvent(QPaintEvent*)
{
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw plain background
    QStyleOptionButton option;
    option.initFrom(this);
    painter.fillRect(rect(), Qt::blue);
    painter.fillRect(option.rect, Qt::red);
    PaintUtils::drawPanelButtonBevel(&painter, option);



    //    QMap<QAction*, QStyleOptionButton> options;
    //    for (int i = 0; i < actions().size(); ++i) {
    //        QAction* action = actions().at(i);

    //        QStyleOptionButton option;
    //        option.initFrom(this);
    //        option.features = QStyleOptionButton::None;
    //        if (action->menu())
    //            option.features |= QStyleOptionButton::HasMenu;
    //        if (action->is || d->menuOpen)
    //            option.state |= QStyle::State_Sunken;
    //        if (d->checked)
    //            option.state |= QStyle::State_On;
    //        if (!d->flat && !d->down)
    //            option.state |= QStyle::State_Raised;
    //        option.text = d->text;
    //        option.icon = d->icon;
    //        option.iconSize = iconSize();
    //    }

    //    if (!pressedPath.isEmpty()) {
    //        painter.setClipping(true);
    //        painter.setClipPath(pressedPath);
    //        option.state |= QStyle::State_On;
    //        PaintUtils::drawPanelButtonBevel(&painter, option);
    //    }

    //    for (int i = 0; i < actions.size(); ++i) {
    //        QAction* action = actions.at(i);
    //        option.rect = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);
    //        painter.drawControl(QStyle::CE_PushButtonLabel, option);
    //    }
}
