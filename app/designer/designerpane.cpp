#include <designerpane.h>
#include <designerview.h>
#include <signaleditor.h>
#include <anchoreditor.h>

#include <QMenu>
#include <QEvent>
#include <QToolBar>

DesignerPane::DesignerPane(QWidget* parent) : QWidget(parent)
  , m_menu(new QMenu(this))
  , m_toolBar(new QToolBar(this))
  , m_designerView(new DesignerView(this))
  , m_signalEditor(new SignalEditor(this))
  , m_anchorEditor(new AnchorEditor(m_designerView->scene(), this))
{

}

QMenu* DesignerPane::menu() const
{
    return m_menu;
}

QToolBar* DesignerPane::toolBar() const
{
    return m_toolBar;
}

DesignerView* DesignerPane::designerView() const
{
    return m_designerView;
}

AnchorEditor* DesignerPane::anchorEditor() const
{
    return m_anchorEditor;
}

SignalEditor* DesignerPane::signalEditor() const
{
    return m_signalEditor;
}

QSize DesignerPane::sizeHint() const
{
    return {680, 680};
}

QSize DesignerPane::minimumSizeHint() const
{
    return {0, 0};
}

void DesignerPane::updateIcons()
{
    //    using namespace PaintUtils;
    //    m_runDevicesButton->setIcon(QIcon(":/images/devices.png"));
    //    m_runButton->setIcon(renderButtonIcon(":/images/run.svg", palette()));
    //    m_stopButton->setIcon(renderButtonIcon(":/images/stop.svg", palette()));
    //    m_preferencesButton->setIcon(renderButtonIcon(":/images/settings.svg", palette()));
    //    m_projectsButton->setIcon(renderButtonIcon(":/images/projects.svg", palette()));

    //    QStringList iconFileNames {
    //        ":/images/left.svg",
    //        ":/images/bottom.svg",
    //        ":/images/right.svg"
    //    };
    //    for (int i = 0; i < iconFileNames.size(); ++i) {
    //        QAction* action = m_segmentedBar->actions().at(i);
    //        action->setIcon(renderButtonIcon(iconFileNames.at(i), palette()));
    //    }
}

void DesignerPane::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::ApplicationFontChange
            || event->type() == QEvent::PaletteChange) {
        updateIcons();
    }
    QWidget::changeEvent(event);
}
