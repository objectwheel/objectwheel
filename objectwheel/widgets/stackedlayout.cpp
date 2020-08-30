#include <stackedlayout.h>
#include <private/qlayout_p.h>

#include <QWidget>
#include <QCoreApplication>

StackedLayout::StackedLayout() : QLayout()
  , m_index(-1)
  , m_animationEnabled(true)
  , m_animation(new QVariantAnimation(this))
{
    m_animation->setStartValue(0);
    m_animation->setDuration(500);
    m_animation->setEasingCurve(QEasingCurve::OutExpo);
    connect(m_animation, &QVariantAnimation::valueChanged,
            this, &StackedLayout::onAnimationValueChanged);
    connect(m_animation, &QVariantAnimation::stateChanged,
            this, &StackedLayout::onAnimationStateChanged);
}

StackedLayout::StackedLayout(QWidget* parent) : QLayout(parent)
  , m_index(-1)
  , m_animationEnabled(true)
  , m_animation(new QVariantAnimation(this))
{
    m_animation->setStartValue(0);
    m_animation->setDuration(500);
    m_animation->setEasingCurve(QEasingCurve::OutExpo);
    connect(m_animation, &QVariantAnimation::valueChanged,
            this, &StackedLayout::onAnimationValueChanged);
    connect(m_animation, &QVariantAnimation::stateChanged,
            this, &StackedLayout::onAnimationStateChanged);
}

StackedLayout::StackedLayout(QLayout* parentLayout) : StackedLayout()
{
    if (parentLayout) {
        setParent(parentLayout);
        parentLayout->addItem(this);
    }
}

StackedLayout::~StackedLayout()
{
    qDeleteAll(m_list);
}

int StackedLayout::addWidget(QWidget* widget)
{
    return insertWidget(m_list.count(), widget);
}

int StackedLayout::insertWidget(int index, QWidget* widget)
{
    addChildWidget(widget);
    index = qMin(index, m_list.count());
    if (index < 0)
        index = m_list.count();
    QWidgetItem* wi = QLayoutPrivate::createWidgetItem(this, widget);
    m_list.insert(index, wi);
    invalidate();
    if (m_index < 0) {
        setCurrentIndex(index);
    } else {
        if (index <= m_index)
            ++m_index;
        widget->hide();
        widget->lower();
    }
    return index;
}

QWidget* StackedLayout::widget(int index) const
{
    if (index < 0 || index >= m_list.size())
        return nullptr;
    return m_list.at(index)->widget();
}

QWidget* StackedLayout::currentWidget() const
{
    return m_index >= 0 ? m_list.at(m_index)->widget() : nullptr;
}

int StackedLayout::currentIndex() const
{
    return m_index;
}

int StackedLayout::count() const
{
    return m_list.size();
}

bool StackedLayout::animationEnabled() const
{
    return m_animationEnabled;
}

void StackedLayout::setAnimationEnabled(bool animationEnabled)
{
    m_animationEnabled = animationEnabled;
}

int StackedLayout::animationDuration() const
{
    return m_animation->duration();
}

void StackedLayout::setAnimationDuration(int animationDuration)
{
    m_animation->setDuration(animationDuration);
}

QEasingCurve StackedLayout::animationEasingCurve() const
{
    return m_animation->easingCurve();
}

void StackedLayout::setAnimationEasingCurve(const QEasingCurve& animationEasingCurve)
{
    m_animation->setEasingCurve(animationEasingCurve);
}

void StackedLayout::addItem(QLayoutItem* item)
{
    if (QWidget* widget = item->widget()) {
        addWidget(widget);
        delete item;
    } else {
        qWarning("StackedLayout::addItem: Only widgets can be added");
    }
}

QSize StackedLayout::sizeHint() const
{
    if (currentWidget()) {
        const QMargins& m = contentsMargins();
        return currentWidget()->sizeHint() + QSize(m.left() + m.right(), m.top() + m.bottom());
    }
    return QSize(); // No implementation in the base class
}

QSize StackedLayout::minimumSize() const
{
    if (currentWidget()) {
        const QMargins& m = contentsMargins();
        return currentWidget()->minimumSizeHint() + QSize(m.left() + m.right(), m.top() + m.bottom());
    }
    return QLayout::minimumSize();
}

QLayoutItem* StackedLayout::itemAt(int index) const
{
    return m_list.value(index);
}

QLayoutItem* StackedLayout::takeAt(int index)
{
    if (index < 0 || index >= m_list.size())
        return nullptr;
    QLayoutItem* item = m_list.takeAt(index);
    if (m_animation->state() == QAbstractAnimation::Running
            && (item->widget() == m_prev || item->widget() == m_next)) {
        m_animation->stop();
    }
    if (index == m_index) {
        m_index = -1;
        if (m_list.count() > 0) {
            int newIndex = (index == m_list.count()) ? index - 1 : index;
            setCurrentIndex(newIndex);
        } else {
            emit currentChanged(-1);
        }
    } else if (index < m_index) {
        --m_index;
    }
    emit widgetRemoved(index);
    if (item->widget() && !QObjectPrivate::get(item->widget())->wasDeleted)
        item->widget()->hide();
    return item;
}

void StackedLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    if (QWidget* widget = currentWidget()) {
        if (m_animation->state() == QAbstractAnimation::Running) {
            if (m_animation->currentTime() == 0)
                widget->resize(contentsRect().size());
        } else {
            widget->setGeometry(contentsRect());
        }
    }
}

bool StackedLayout::hasHeightForWidth() const
{
    int n = count();
    for (int i = 0; i < n; ++i) {
        if (QLayoutItem* item = itemAt(i)) {
            if (item->hasHeightForWidth())
                return true;
        }
    }
    return false;
}

int StackedLayout::heightForWidth(int width) const
{
    int n = count();
    int hfw = 0;
    for (int i = 0; i < n; ++i) {
        if (QLayoutItem* item = itemAt(i)) {
            if (QWidget* w = item->widget())
                hfw = qMax(hfw, w->heightForWidth(width));
        }
    }
    hfw = qMax(hfw, minimumSize().height());
    return hfw;
}

void StackedLayout::setCurrentWidget(QWidget* widget, SlidingDirection direction)
{
    int index = indexOf(widget);
    if (index == -1) {
        qWarning("StackedLayout::setCurrentWidget: Widget %p not contained in stack", (void*) widget);
        return;
    }
    setCurrentIndex(index, direction);
}

void StackedLayout::setCurrentIndex(int index, SlidingDirection direction)
{
    if (m_animation->state() == QAbstractAnimation::Running)
        m_animation->stop();

    m_prev = currentWidget();
    m_next = widget(index);

    if (m_next.isNull() || m_next == m_prev)
        return;

    if (m_prev)
        m_prev->clearFocus();

    m_index = index;
    m_next->move(geometry().topRight());
    m_next->show();
    m_next->raise();

    if (m_prev && m_animationEnabled) {
        if (direction == Natural) {
            if (indexOf(m_next) > indexOf(m_prev))
                direction = Left;
            else
                direction = Right;
        }
        m_animation->setEndValue((direction == Left ? -1 : 1) * m_prev->width());
        m_animation->start();
    } else {
        onAnimationStateChanged(QAbstractAnimation::Stopped, QAbstractAnimation::Stopped);
    }

    emit currentChanged(index);
}

void StackedLayout::onAnimationValueChanged(const QVariant& value)
{
    if (m_next.isNull() || m_prev.isNull()) {
        m_animation->stop();
        return;
    }

    int ext = value.toInt();
    const QRect& cr = contentsRect();
    m_prev->move(cr.left() + ext, cr.top());
    m_next->move((ext > 0 ? cr.left() - m_next->width() : cr.left() + m_prev->width()) + ext, cr.top());
}

void StackedLayout::onAnimationStateChanged(QAbstractAnimation::State newState, QAbstractAnimation::State)
{
    if (newState == QAbstractAnimation::Running)
        return;

    QWidget* parent = parentWidget();
    QPointer<QWidget> fw = parent ? parent->window()->focusWidget() : 0;
    bool focusWasOnOldPage = fw && m_prev && m_prev->isAncestorOf(fw);

    if (m_prev)
        m_prev->hide();

    activate();

    // try to move focus onto the incoming widget
    // if it was somewhere on the outgoing widget.
    if (parent && focusWasOnOldPage && m_next) {
        // look for the best focus widget we can find
        if (QWidget* nfw = m_next->focusWidget()) {
            nfw->setFocus();
        } else {
            // second best: first child widget in the focus chain
            if (QWidget* i = fw) {
                while ((i = i->nextInFocusChain()) != fw) {
                    if (((i->focusPolicy() & Qt::TabFocus) == Qt::TabFocus)
                            && !i->focusProxy() && i->isVisibleTo(m_next) && i->isEnabled()
                            && m_next->isAncestorOf(i)) {
                        i->setFocus();
                        break;
                    }
                }
                // third best: incoming widget
                if (i == fw)
                    m_next->setFocus();
            }
        }
    }
}
