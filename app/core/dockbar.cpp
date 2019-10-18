#include <dockbar.h>
#include <pinbar.h>
#include <utilityfunctions.h>

#include <QDockWidget>
#include <QToolButton>

DockBar::DockBar(QWidget* parent) : QToolBar(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
}

void DockBar::addDockWidget(QDockWidget* dockWidget)
{
    DockData data;
    data.dockWidget = dockWidget;
    data.button = new QToolButton(this);
    data.button->setCursor(Qt::PointingHandCursor);
//    data.button->setFixedWidth(38);
    data.button->setIconSize({16, 16});
    if (auto pinBar = qobject_cast<PinBar*>(dockWidget->titleBarWidget())) {
        data.button->setIcon(pinBar->icon());
        data.button->setText(pinBar->title());
        data.button->setToolTip(UtilityFunctions::toToolTip(tr("Show <b>%1 Pane</b>").arg(pinBar->title())));
    }
//    if (dockWidget->isVisible())
    addWidget(data.button);
    m_dockWidgets.append(data);
}

QSize DockBar::sizeHint() const
{
    return QSize(20, QToolBar::sizeHint().height());
}

QSize DockBar::minimumSizeHint() const
{
    return QSize(20, QToolBar::minimumSizeHint().height());
}
