#include <dockbar.h>
#include <utilityfunctions.h>

#include <QToolButton>
#include <QDockWidget>
#include <QLayout>
#include <QPainter>

DockBar::DockBar(QWidget* parent) : QToolBar(parent)
{
    setFocusPolicy(Qt::NoFocus);
    layout()->setSpacing(2);
    layout()->setContentsMargins(1, 1, 1, 1);
}

void DockBar::addDockWidget(QDockWidget* dockWidget)
{
    auto button = new QToolButton(this);
    button->setCheckable(true);
    button->setFixedWidth(18);
    button->setIcon(dockWidget->windowIcon());
    button->setText(dockWidget->windowTitle());
    button->setCursor(Qt::PointingHandCursor);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setToolTip(UtilityFunctions::toToolTip(tr("Show <b>%1 Pane</b>").arg(dockWidget->windowTitle())));
    connect(button, &QToolButton::clicked, this, [=] { emit dockWidgetButtonClicked(dockWidget, button->isChecked()); });
    DockData data;
    data.dockWidget = dockWidget;
    data.action = addWidget(button);
    m_dockDataList.append(data);
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
}

void DockBar::setDockWidgetButtonChecked(QDockWidget* dockWidget, bool checked)
{
    for (int i = 0; i < m_dockDataList.size(); ++i) {
        const DockData& data = m_dockDataList.at(i);
        if (data.dockWidget == dockWidget) {
            if (auto button = static_cast<QToolButton*>(widgetForAction(data.action)))
                button->setChecked(checked);
            break;
        }
    }
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
