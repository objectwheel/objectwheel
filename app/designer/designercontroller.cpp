#include <designercontroller.h>
#include <designerpane.h>
#include <designerview.h>
#include <designerscene.h>
#include <saveutils.h>
#include <projectmanager.h>

#include <QJsonObject>
#include <QJsonDocument>
#include <QComboBox>
#include <QMenu>

DesignerController::DesignerController(DesignerPane* designerPane, QObject* parent) : QObject(parent)
  , m_designerPane(designerPane)
{
    connect(m_designerPane, &DesignerPane::customContextMenuRequested,
            this, &DesignerController::onCustomContextMenuRequest);
}

void DesignerController::charge()
{
    if (!ProjectManager::isStarted())
        return;

    const QJsonObject& object = QJsonDocument::fromBinaryData(SaveUtils::projectTheme(ProjectManager::dir())).object();
    const QString& theme = object.value("stylev2").toString();
    if (!theme.isEmpty()) {
        for (int i = 0; i < m_designerPane->themeComboBox()->count(); i++) {
            if (m_designerPane->themeComboBox()->itemText(i).contains(theme))
                m_designerPane->themeComboBox()->setCurrentIndex(i);
        }
    }
}

void DesignerController::discharge()
{
    m_designerPane->designerView()->scene()->clear();
}

void DesignerController::onCustomContextMenuRequest(const QPoint& pos)
{
    const DesignerView* view = m_designerPane->designerView();
    const DesignerScene* scene = view->scene();
    const QPoint& globalPos = m_designerPane->mapToGlobal(pos);

    m_menuTargetControl = scene->topLevelControl(view->mapToScene(view->viewport()->mapFromGlobal(globalPos)));
    m_designerPane->toggleSelectionAction()->setEnabled(m_menuTargetControl);
    m_designerPane->sendBackAction()->setEnabled(m_menuTargetControl);
    m_designerPane->bringFrontAction()->setEnabled(m_menuTargetControl);
    m_designerPane->cutAction()->setEnabled(m_menuTargetControl);
    m_designerPane->copyAction()->setEnabled(m_menuTargetControl);
    m_designerPane->deleteAction()->setEnabled(m_menuTargetControl);
    m_designerPane->moveLeftAction()->setEnabled(m_menuTargetControl);
    m_designerPane->moveRightAction()->setEnabled(m_menuTargetControl);
    m_designerPane->moveUpAction()->setEnabled(m_menuTargetControl);
    m_designerPane->moveDownAction()->setEnabled(m_menuTargetControl);

    m_designerPane->menu()->popup(globalPos);
}
