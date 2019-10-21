#include <dockbar.h>
#include <pinbar.h>
#include <utilityfunctions.h>

#include <QToolButton>
#include <QDockWidget>
#include <QLayout>

DockBar::DockBar(QWidget* parent) : QToolBar(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // Workaround for QToolBarLayout's obsolote serMargin function usage
    QMetaObject::invokeMethod(this, [=] {
        setContentsMargins(0, 0, 0, 0);
        layout()->setContentsMargins(0, 0, 0, 0); // They must be all same
        layout()->setSpacing(0);
    }, Qt::QueuedConnection);
}

void DockBar::addDockWidget(QDockWidget* dockWidget)
{
    auto pinBar = qobject_cast<PinBar*>(dockWidget->titleBarWidget());

    if (pinBar == 0)
        return;

    DockData data;
    data.dockWidget = dockWidget;
    data.button = new QToolButton(this);
    data.button->setIcon(pinBar->icon());
    data.button->setText(pinBar->title());
    data.button->setCursor(Qt::PointingHandCursor);
//    data.button->setFixedWidth(18);
    data.button->setToolTip(UtilityFunctions::toToolTip(tr("Show <b>%1 Pane</b>").arg(pinBar->title())));
//    if (dockWidget->isVisible())
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
