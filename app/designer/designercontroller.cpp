#include <designercontroller.h>
#include <designerpane.h>
#include <designerview.h>
#include <designerscene.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <controlpropertymanager.h>
#include <control.h>
#include <anchoreditor.h>

#include <QJsonObject>
#include <QJsonDocument>
#include <QComboBox>
#include <QMenu>

DesignerController::DesignerController(DesignerPane* designerPane, QObject* parent) : QObject(parent)
  , m_designerPane(designerPane)
{
    const DesignerScene* scene = m_designerPane->designerView()->scene();
    connect(m_designerPane, &DesignerPane::customContextMenuRequested,
            this, &DesignerController::onCustomContextMenuRequest);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::doubleClicked,
            this, &DesignerController::onControlDoubleClick);

//    connect(m_anchorEditor, &AnchorEditor::anchored, this, [=] (AnchorLine::Type sourceLineType, const AnchorLine& targetLine) {
//        if (targetLine.isValid()) {
//            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                               "anchors." + anchorLineText(sourceLineType),
//                                               fixedTargetId(m_anchorEditor->sourceControl(), targetLine.control())
//                                               + "." + anchorLineText(targetLine.type()),
//                                               ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
//        } else {
//            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                               "anchors." + anchorLineText(sourceLineType),
//                                               QString(), ControlPropertyManager::SaveChanges);
//            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                               "anchors." + anchorLineText(sourceLineType),
//                                               "undefined", ControlPropertyManager::UpdateRenderer);
//        }
//    });
//    connect(m_anchorEditor, &AnchorEditor::filled, this, [=] (Control* control) {
//        if (control) {
//            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                               "anchors.fill",
//                                               fixedTargetId(m_anchorEditor->sourceControl(), control),
//                                               ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
//        } else {
//            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                               "anchors.fill", QString(), ControlPropertyManager::SaveChanges);
//            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                               "anchors.fill", "undefined", ControlPropertyManager::UpdateRenderer);
//        }
//    });
//    connect(m_anchorEditor, &AnchorEditor::centered, this, [=] (Control* control, bool overlay) {
//        if (control) {
//            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                               "anchors.centerIn",
//                                               fixedTargetId(m_anchorEditor->sourceControl(), control),
//                                               ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
//        } else {
//            if (m_anchorEditor->sourceControl()->popup() && overlay) {
//                ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                                   "anchors.centerIn", "Overlay.overlay",
//                                                   ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
//            } else {
//                ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                                   "anchors.centerIn", QString(), ControlPropertyManager::SaveChanges);
//                ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                                   "anchors.centerIn", "undefined", ControlPropertyManager::UpdateRenderer);
//            }
//        }
//    });
//    connect(m_anchorEditor, &AnchorEditor::cleared, this, [=] {
//        for (const QString& name : UtilityFunctions::anchorLineNames()) {
//            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                               name, QString(),
//                                               ControlPropertyManager::SaveChanges);
//            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                               name, "undefined",
//                                               ControlPropertyManager::UpdateRenderer);
//        }
//        for (const QString& name : UtilityFunctions::anchorPropertyNames()) {
//            if (name == "anchors.leftMargin"
//                    || name == "anchors.rightMargin"
//                    || name == "anchors.topMargin"
//                    || name == "anchors.bottomMargin") {
//                ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                                   name, QString(), ControlPropertyManager::SaveChanges);
//                ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                                   name, "undefined", ControlPropertyManager::UpdateRenderer);
//            } else {
//                ControlPropertyManager::setProperty(m_anchorEditor->sourceControl(),
//                                                    name, QString(),
//                                                    name.contains("alignWhenCentered") ? 1 : 0,
//                                                    ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
//            }
//        }
//    });
//    connect(m_anchorEditor, &AnchorEditor::marginOffsetEdited, this, [=] (AnchorLine::Type sourceLineType, qreal marginOffset) {
//        if (marginOffset == 0 && (sourceLineType == AnchorLine::Left
//                                  || sourceLineType == AnchorLine::Right
//                                  || sourceLineType == AnchorLine::Top
//                                  || sourceLineType == AnchorLine::Bottom)) {
//            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                               "anchors." + marginOffsetText(sourceLineType),
//                                               QString(), ControlPropertyManager::SaveChanges);
//            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
//                                               "anchors." + marginOffsetText(sourceLineType),
//                                               "undefined", ControlPropertyManager::UpdateRenderer);
//        } else {
//            ControlPropertyManager::setProperty(m_anchorEditor->sourceControl(),
//                                                "anchors." + marginOffsetText(sourceLineType),
//                                                marginOffset == 0 ? QString() : QString::number(marginOffset), marginOffset,
//                                                ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
//        }
//    });
//    connect(m_anchorEditor, &AnchorEditor::marginsEdited, this, [=] (qreal margins) {
//        ControlPropertyManager::setProperty(m_anchorEditor->sourceControl(),
//                                            "anchors.margins",
//                                            margins == 0 ? QString() : QString::number(margins), margins,
//                                            ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
//    });
    connect(m_designerPane->anchorEditor(), &AnchorEditor::alignmentActivated,
            this, &DesignerController::onAnchorAlignmentActivation);
    connect(m_designerPane->anchorEditor(), &AnchorEditor::sourceControlActivated,
            this, &DesignerController::onAnchorSourceControlActivation);
    connect(scene, &DesignerScene::anchorEditorActivated,
            this, &DesignerController::onAnchorEditorActivation);
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

void DesignerController::onControlDoubleClick(Control* control, Qt::MouseButtons buttons)
{
    DesignerScene* scene = m_designerPane->designerView()->scene();
    if (QGraphicsItem* mouseGrabber = scene->mouseGrabberItem())
        mouseGrabber->ungrabMouse();
    if (buttons & Qt::LeftButton)
        emit codeEditorTriggered(control, SaveUtils::controlMainQmlFileName());
    if (buttons & Qt::RightButton) {
        Control* sourceControl = control;
        Control* targetControl = control->parentControl();
        if (DesignerScene::isAnchorViable(sourceControl, targetControl)) {
            const QList<Control*>& selection = scene->selectedControls();
            scene->clearSelection();
            sourceControl->setSelected(true);
            m_designerPane->anchorEditor()->setSourceControl(sourceControl);
            m_designerPane->anchorEditor()->setPrimaryTargetControl(targetControl);
            m_designerPane->anchorEditor()->refresh();
            m_designerPane->anchorEditor()->exec();
            scene->clearSelection();
            for (Control* control : selection)
                control->setSelected(true);
        }
    }
}

void DesignerController::onAnchorClear()
{

}

void DesignerController::onAnchorSourceControlActivation()
{
    m_designerPane->designerView()->scene()->clearSelection();
    m_designerPane->anchorEditor()->sourceControl()->setSelected(true);
}

void DesignerController::onAnchor(AnchorLine::Type sourceLineType, const AnchorLine& targetLine)
{

}

void DesignerController::onAnchorFill(Control* control)
{

}

void DesignerController::onAnchorCenter(Control* control, bool overlay)
{

}

void DesignerController::onAnchorMarginOffsetEdit(AnchorLine::Type sourceLineType, qreal marginOffset)
{

}

void DesignerController::onAnchorMarginsEdit(qreal margins)
{

}

void DesignerController::onAnchorAlignmentActivation(bool aligned)
{
    ControlPropertyManager::setProperty(m_designerPane->anchorEditor()->sourceControl(),
                                        "anchors.alignWhenCentered",
                                        aligned ? QString() : "false", aligned,
                                        ControlPropertyManager::SaveChanges
                                        | ControlPropertyManager::UpdateRenderer);
}

void DesignerController::onAnchorEditorActivation(Control* sourceControl, Control* targetControl)
{
    DesignerScene* scene = m_designerPane->designerView()->scene();
    const QList<Control*>& selection = scene->selectedControls();
    scene->clearSelection();
    sourceControl->setSelected(true);
    m_designerPane->anchorEditor()->setSourceControl(sourceControl);
    m_designerPane->anchorEditor()->setPrimaryTargetControl(targetControl);
    m_designerPane->anchorEditor()->refresh();
    m_designerPane->anchorEditor()->exec();
    scene->clearSelection();
    for (Control* control : selection)
        control->setSelected(true);
}
