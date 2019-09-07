#include <designerpane.h>
#include <designerview.h>
#include <anchoreditor.h>
#include <paintutils.h>
#include <QMenu>
#include <QEvent>
#include <QToolBar>
#include <QVBoxLayout>

static QIcon icon(const QString& fileName, const QColor& color)
{
    QIcon icon;
    icon.addPixmap(PaintUtils::renderOverlaidPixmap(":/images/designer/" + fileName + ".svg", Qt::red), QIcon::Normal, QIcon::On);
    icon.addPixmap(PaintUtils::renderOverlaidPixmap(":/images/designer/" + fileName + ".svg", Qt::red), QIcon::Normal, QIcon::Off);
    icon.addPixmap(PaintUtils::renderOverlaidPixmap(":/images/designer/" + fileName + ".svg", Qt::red), QIcon::Selected, QIcon::On);
    icon.addPixmap(PaintUtils::renderOverlaidPixmap(":/images/designer/" + fileName + ".svg", Qt::red), QIcon::Selected, QIcon::Off);
    icon.addPixmap(PaintUtils::renderOverlaidPixmap(":/images/designer/" + fileName + ".svg", Qt::red), QIcon::Active, QIcon::On);
    icon.addPixmap(PaintUtils::renderOverlaidPixmap(":/images/designer/" + fileName + ".svg", Qt::red), QIcon::Active, QIcon::Off);
//    icon.addPixmap(PaintUtils::renderOverlaidPixmap(":/images/designer/" + fileName + ".svg", color));
    return icon;
}

DesignerPane::DesignerPane(QWidget* parent) : QWidget(parent)
  , m_menu(new QMenu(this))
  , m_toolBar(new QToolBar(this))
  , m_designerView(new DesignerView(this))
  , m_anchorEditor(new AnchorEditor(m_designerView->scene(), this))
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_toolBar);
    layout->addWidget(m_designerView);

    QAction* m_sendBackAction = m_menu->addAction(tr("Send to Back"));
    QAction* m_bringFrontAction = m_menu->addAction(tr("Bring to Front"));
    QAction* m_cutAction = m_menu->addAction(tr("Cut"));
    QAction* m_copyAction = m_menu->addAction(tr("Copy"));
    QAction* m_pasteAction = m_menu->addAction(tr("Paste"));
    QAction* m_selectAllAction = m_menu->addAction(tr("Select All"));
    QAction* m_deleteAction = m_menu->addAction(tr("Delete"));
    QAction* m_moveLeftAction = m_menu->addAction(tr("Move left"));
    QAction* m_moveRightAction = m_menu->addAction(tr("Move right"));
    QAction* m_moveUpAction = m_menu->addAction(tr("Move up"));
    QAction* m_moveDownAction = m_menu->addAction(tr("Move down"));

    m_sendBackAction->setIcon(icon("send-to-back", Qt::darkGray));
    m_bringFrontAction->setIcon(icon("bring-to-front", Qt::darkGray));
    m_cutAction->setIcon(icon("cut", Qt::darkGray));
    m_copyAction->setIcon(icon("copy", Qt::darkGray));
    m_pasteAction->setIcon(icon("paste", Qt::darkGray));
    m_selectAllAction->setIcon(icon("select-all", Qt::darkGray));
    m_deleteAction->setIcon(icon("delete", Qt::darkGray));
    m_moveLeftAction->setIcon(icon("move-left", Qt::darkGray));
    m_moveRightAction->setIcon(icon("move-right", Qt::darkGray));
    m_moveUpAction->setIcon(icon("move-up", Qt::darkGray));
    m_moveDownAction->setIcon(icon("move-down", Qt::darkGray));
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

void DesignerPane::contextMenuEvent(QContextMenuEvent* event)
{
    QWidget::contextMenuEvent(event);
    m_menu->exec();
}
