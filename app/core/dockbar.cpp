#include <dockbar.h>
#include <pinbar.h>
#include <utilityfunctions.h>

#include <QToolButton>
#include <QDockWidget>
#include <QLayout>

DockBar::DockBar(QWidget* parent) : QToolBar(parent)
{
    setFocusPolicy(Qt::NoFocus);
    //    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    setIconSize({16, 16});

    // Workaround for QToolBarLayout's obsolote serMargin function usage
    QMetaObject::invokeMethod(this, [=] {
        setContentsMargins(0, 0, 0, 0);
        layout()->setContentsMargins(0, 0, 0, 0); // They must be all same
        layout()->setSpacing(2);
    }, Qt::QueuedConnection);
}

void DockBar::addDockWidget(QDockWidget* dockWidget)
{
    auto pinBar = qobject_cast<PinBar*>(dockWidget->titleBarWidget());

    if (pinBar == 0)
        return;

    auto button = new QToolButton(this);
    button->setIcon(pinBar->icon());
    button->setText(pinBar->title());
    button->setCursor(Qt::PointingHandCursor);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    DockData data;
    data.dockWidget = dockWidget;
    data.action = addWidget(button);
//    data.action->setFixedWidth(18);
//    data.action->setIconSize({16, 16});
    data.action->setToolTip(UtilityFunctions::toToolTip(tr("Show <b>%1 Pane</b>").arg(pinBar->title())));
//    if (dockWidget->isVisible())
        m_dockWidgets.append(data);
}

//QSize DockBar::sizeHint() const
//{
//    return QSize(20, QToolBar::sizeHint().height());
//}

//QSize DockBar::minimumSizeHint() const
//{
//    return QSize(20, QToolBar::minimumSizeHint().height());
//}
