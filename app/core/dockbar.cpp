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
    DockData data;
    data.dockWidget = dockWidget;
    data.button = new QToolButton(this);
    data.button->setFixedWidth(18);
    data.button->setIcon(dockWidget->windowIcon());
    data.button->setText(dockWidget->windowTitle());
    data.button->setCursor(Qt::PointingHandCursor);
    data.button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    data.button->setToolTip(UtilityFunctions::toToolTip(tr("Show <b>%1 Pane</b>").arg(dockWidget->windowTitle())));
    m_dockWidgets.append(data);
    addWidget(data.button);
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
