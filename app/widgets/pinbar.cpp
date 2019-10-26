#include <pinbar.h>
#include <paintutils.h>

#include <QToolButton>
#include <QLabel>
#include <QBoxLayout>
#include <QPainter>
#include <QDockWidget>
#include <QStyleOption>

PinBar::PinBar(QDockWidget* dockWidget) : QWidget(dockWidget)
  , m_dockWidget(dockWidget)
  , m_iconLabel(new QLabel(this))
  , m_titleLabel(new QLabel(this))
  , m_detachButton(new QToolButton(this))
  , m_closeButton(new QToolButton(this))
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_iconLabel->setFixedSize(16, 16);
    m_iconLabel->setPixmap(PaintUtils::pixmap(dockWidget->windowIcon(), QSize(16, 16)));

    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_titleLabel->setText(dockWidget->windowTitle());

    m_detachButton->setFixedSize(16, 16);
    m_detachButton->setIconSize(QSize(12, 12));
    m_detachButton->setToolTip(tr("Unpin the pane"));
    m_detachButton->setCursor(Qt::PointingHandCursor);
    m_detachButton->setIcon(QIcon(QStringLiteral(":/images/designer/unpin.svg")));

    m_closeButton->setFixedSize(16, 16);
    m_closeButton->setIconSize(QSize(12, 12));
    m_closeButton->setToolTip(tr("Close the pane"));
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setIcon(QIcon(QStringLiteral(":/images/designer/close.svg")));

    auto layout = new QHBoxLayout(this);
    layout->setSpacing(4);
    layout->setContentsMargins(6, 0, 2, 0);
    layout->addWidget(m_iconLabel, 0, Qt::AlignVCenter);
    layout->addWidget(m_titleLabel, 0, Qt::AlignVCenter);
    layout->addWidget(m_detachButton, 0, Qt::AlignVCenter);
    layout->addWidget(m_closeButton, 0, Qt::AlignVCenter);

    connect(m_closeButton, &QToolButton::clicked,
            m_dockWidget, &QDockWidget::hide);
    connect(m_closeButton, &QToolButton::clicked,
            this, [=] { emit dockWidgetHid(dockWidget); });
    connect(m_detachButton, &QToolButton::clicked,
            this, &PinBar::onDetachButtonClick);
    connect(m_dockWidget, &QDockWidget::topLevelChanged,
            this, &PinBar::onDockWidgetTopLevelChange);
}

QSize PinBar::sizeHint() const
{
    return QSize(QWidget::sizeHint().width(), 20);
}

QSize PinBar::minimumSizeHint() const
{
    return QSize(QWidget::minimumSizeHint().width(), 20);
}

void PinBar::onDetachButtonClick()
{
    m_dockWidget->setFloating(!m_dockWidget->isFloating());
}

void PinBar::onDockWidgetTopLevelChange()
{
    m_detachButton->setIcon(m_dockWidget->isFloating()
                            ? QIcon(QStringLiteral(":/images/designer/pin.svg"))
                            : QIcon(QStringLiteral(":/images/designer/unpin.svg")));
    m_detachButton->setToolTip(m_dockWidget->isFloating()
                               ? tr("Pin the pane")
                               : tr("Unpin the pane"));
}

void PinBar::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QStyleOptionToolBar option;
    option.initFrom(this);
    option.state |= QStyle::State_Horizontal;
    style()->drawControl(QStyle::CE_ToolBar, &option, &painter, this);
    painter.fillRect(QRectF(0, 0, 2.5, height()), palette().highlight());
}
