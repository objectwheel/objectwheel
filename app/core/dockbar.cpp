#include <dockbar.h>
#include <utilityfunctions.h>

#include <QToolButton>
#include <QDockWidget>
#include <QLayout>
#include <QPainter>

DockBar::DockBar(QWidget* parent) : QToolBar(parent)
{
    setVisible(false);
    setFocusPolicy(Qt::NoFocus);
    layout()->setSpacing(2);
    layout()->setContentsMargins(1, 1, 1, 1);
}

void DockBar::addDockWidget(QDockWidget* dockWidget)
{
    auto button = new QToolButton(this);
    button->setFixedWidth(18);
    button->setIcon(dockWidget->windowIcon());
    button->setText(dockWidget->windowTitle());
    button->setCursor(Qt::PointingHandCursor);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setToolTip(UtilityFunctions::toToolTip(tr("Show <b>%1 Pane</b>").arg(dockWidget->windowTitle())));
    connect(button, &QToolButton::clicked, dockWidget, &QDockWidget::show);
    connect(button, &QToolButton::clicked, this, [=] { emit dockWidgetShown(dockWidget); });
    DockData data;
    data.dockWidget = dockWidget;
    data.action = addWidget(button);
    m_dockDataList.append(data);
    setVisible(true);
}

void DockBar::removeDockWidget(QDockWidget* dockWidget)
{
    for (int i = 0; i < m_dockDataList.size(); ++i) {
        const DockData& data = m_dockDataList.at(i);
        if (data.dockWidget == dockWidget) {
            auto button = widgetForAction(data.action);
            removeAction(data.action);
            if (button)
                delete button;
            m_dockDataList.removeAt(i);
            break;
        }
    }
    if (m_dockDataList.isEmpty())
        setVisible(false);
}

QSize DockBar::sizeHint() const
{
    return QSize(20, QToolBar::sizeHint().height());
}

QSize DockBar::minimumSizeHint() const
{
    return QSize(20, QToolBar::minimumSizeHint().height());
}

void DockBar::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setPen("#b6b6b6");
    p.setBrush(palette().window());
    p.drawRect(rect());
}
