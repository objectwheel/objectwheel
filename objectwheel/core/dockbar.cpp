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

bool DockBar::buttonExists(QDockWidget* dockWidget) const
{
    for (const DockData& data : qAsConst(m_dockDataList)) {
        if (data.dockWidget == dockWidget)
            return true;
    }
    return false;
}

void DockBar::addDockWidget(QDockWidget* dockWidget)
{
    using namespace UtilityFunctions;
    auto button = new QToolButton(this);
    button->setCheckable(true);
    button->setFixedWidth(20);
    button->setIcon(dockWidget->windowIcon());
    button->setText(dockWidget->windowTitle());
    button->setCursor(Qt::PointingHandCursor);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setToolTip(toToolTip(tr("Hide <b>%1 Pane</b>").arg(dockWidget->windowTitle())));
    connect(button, &QToolButton::clicked,
            this, [=] { emit dockWidgetButtonClicked(dockWidget, button->isChecked()); });
    connect(button, &QToolButton::toggled,
            this, [=] {
        if (button->isChecked())
            button->setToolTip(toToolTip(tr("Show <b>%1 Pane</b>").arg(dockWidget->windowTitle())));
        else
            button->setToolTip(toToolTip(tr("Hide <b>%1 Pane</b>").arg(dockWidget->windowTitle())));
    });
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

void DockBar::setDockWidgetButtonChecked(QDockWidget* dockWidget, bool checked) const
{
    for (const DockData& data : qAsConst(m_dockDataList)) {
        if (data.dockWidget == dockWidget) {
            if (auto button = static_cast<QToolButton*>(widgetForAction(data.action)))
                button->setChecked(checked);
            break;
        }
    }
}

QSize DockBar::sizeHint() const
{
    return QSize(22, QToolBar::sizeHint().height());
}

QSize DockBar::minimumSizeHint() const
{
    return QSize(22, QToolBar::minimumSizeHint().height());
}

void DockBar::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setPen("#b6b6b6");
    p.setBrush(palette().window());
    p.drawRect(rect());
}
