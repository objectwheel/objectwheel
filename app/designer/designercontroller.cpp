#include <designercontroller.h>
#include <designerpane.h>
#include <designerview.h>
#include <designerscene.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <controlpropertymanager.h>
#include <form.h>
#include <anchoreditor.h>
#include <utilityfunctions.h>
#include <designersettings.h>
#include <scenesettings.h>

#include <QToolButton>
#include <QJsonObject>
#include <QJsonDocument>
#include <QComboBox>
#include <QMenu>

static QString cleanId(const Control* sourceControl, const Control* targetControl)
{
    if (targetControl->window() || targetControl->popup())
        return QStringLiteral("parent");
    if (sourceControl->parentControl() == targetControl)
        return QStringLiteral("parent");
    return targetControl->id();
}

static QString anchorLineText(AnchorLine::Type type)
{
    switch (type) {
    case AnchorLine::Left:
        return QStringLiteral("left");
    case AnchorLine::Right:
        return QStringLiteral("right");
    case AnchorLine::Top:
        return QStringLiteral("top");
    case AnchorLine::Bottom:
        return QStringLiteral("bottom");
    case AnchorLine::Baseline:
        return QStringLiteral("baseline");
    case AnchorLine::HorizontalCenter:
        return QStringLiteral("horizontalCenter");
    case AnchorLine::VerticalCenter:
        return QStringLiteral("verticalCenter");
    case AnchorLine::Fill:
        return QStringLiteral("fill");
    case AnchorLine::Center:
        return QStringLiteral("centerIn");
    default:
        return QString();
    }
}

static QString marginOffsetText(AnchorLine::Type type)
{
    switch (type) {
    case AnchorLine::Left:
        return QStringLiteral("leftMargin");
    case AnchorLine::Right:
        return QStringLiteral("rightMargin");
    case AnchorLine::Top:
        return QStringLiteral("topMargin");
    case AnchorLine::Bottom:
        return QStringLiteral("bottomMargin");
    case AnchorLine::Baseline:
        return QStringLiteral("baselineOffset");
    case AnchorLine::HorizontalCenter:
        return QStringLiteral("horizontalCenterOffset");
    case AnchorLine::VerticalCenter:
        return QStringLiteral("verticalCenterOffset");
    default:
        return QString();
    }
}

DesignerController::DesignerController(DesignerPane* designerPane, QObject* parent) : QObject(parent)
  , m_designerPane(designerPane)
  , m_menuTargetControl(nullptr)
{
    const SceneSettings* settings = DesignerSettings::sceneSettings();
    const DesignerScene* scene = m_designerPane->designerView()->scene();

    m_designerPane->themeComboBox()->addItem(QStringLiteral("Default"));
    m_designerPane->themeComboBox()->addItem(QStringLiteral("Fusion"));
    m_designerPane->themeComboBox()->addItem(QStringLiteral("Imagine"));
    m_designerPane->themeComboBox()->addItem(QStringLiteral("Material"));
    m_designerPane->themeComboBox()->addItem(QStringLiteral("Universal"));

    m_designerPane->zoomLevelComboBox()->addItems(UtilityFunctions::zoomTexts());
    m_designerPane->zoomLevelComboBox()->setCurrentText(UtilityFunctions::zoomLevelToText(settings->sceneZoomLevel));

    m_designerPane->anchorsButton()->setChecked(false);
    m_designerPane->snappingButton()->setChecked(settings->snappingEnabled);
    m_designerPane->gridViewButton()->setChecked(settings->showGridViewDots);
    m_designerPane->guidelinesButton()->setChecked(settings->showGuideLines);

    connect(m_designerPane, &DesignerPane::customContextMenuRequested,
            this, &DesignerController::onCustomContextMenuRequest);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::doubleClicked,
            this, &DesignerController::onControlDoubleClick);

    connect(m_designerPane->anchorEditor(), &AnchorEditor::anchored,
            this, &DesignerController::onAnchor);
    connect(m_designerPane->anchorEditor(), &AnchorEditor::filled,
            this, &DesignerController::onAnchorFill);
    connect(m_designerPane->anchorEditor(), &AnchorEditor::centered,
            this, &DesignerController::onAnchorCenter);
    connect(m_designerPane->anchorEditor(), &AnchorEditor::cleared,
            this, &DesignerController::onAnchorClear);
    connect(m_designerPane->anchorEditor(), &AnchorEditor::marginOffsetEdited,
            this, &DesignerController::onAnchorMarginOffsetEdit);
    connect(m_designerPane->anchorEditor(), &AnchorEditor::marginsEdited,
            this, &DesignerController::onAnchorMarginsEdit);
    connect(m_designerPane->anchorEditor(), &AnchorEditor::alignmentActivated,
            this, &DesignerController::onAnchorAlignmentActivation);
    connect(m_designerPane->anchorEditor(), &AnchorEditor::sourceControlActivated,
            this, &DesignerController::onAnchorSourceControlActivation);
    connect(scene, &DesignerScene::anchorEditorActivated,
            this, &DesignerController::onAnchorEditorActivation);

    connect(m_designerPane->refreshButton(), &QToolButton::toggled,
            this, &DesignerController::onRefreshButtonClick);
    connect(m_designerPane->clearButton(), &QToolButton::toggled,
            this, &DesignerController::onClearButtonClick);
    connect(m_designerPane->anchorsButton(), &QToolButton::toggled,
            this, &DesignerController::onAnchorsButtonClick);
    connect(m_designerPane->snappingButton(), &QToolButton::toggled,
            this, &DesignerController::onSnappingButtonClick);
    connect(m_designerPane->gridViewButton(), &QToolButton::toggled,
            this, &DesignerController::onGridViewButtonClick);
    connect(m_designerPane->guidelinesButton(), &QToolButton::toggled,
            this, &DesignerController::onGuidelinesButtonClick);
    connect(m_designerPane->sceneSettingsButton(), &QToolButton::toggled,
            this, &DesignerController::onSceneSettingsButtonClick);
    connect(m_designerPane->themeSettingsButton(), &QToolButton::toggled,
            this, &DesignerController::onThemeSettingsButtonClick);
    connect(m_designerPane->zoomLevelComboBox(), qOverload<const QString&>(&QComboBox::activated),
            this, &DesignerController::onZoomLevelComboBoxActivation);
    connect(m_designerPane->themeComboBox(), qOverload<const QString&>(&QComboBox::activated),
            this, &DesignerController::onThemeComboBoxActivation);

    connect(m_designerPane->toggleSelectionAction(), &QAction::triggered,
            this, &DesignerController::onToggleSelectionActionTrigger);
    connect(m_designerPane->selectAllAction(), &QAction::triggered,
            this, &DesignerController::onSelectAllActionTrigger);
    connect(m_designerPane->sendBackAction(), &QAction::triggered,
            this, &DesignerController::onSendBackActionTrigger);
    connect(m_designerPane->bringFrontAction(), &QAction::triggered,
            this, &DesignerController::onBringFrontActionTrigger);
    connect(m_designerPane->cutAction(), &QAction::triggered,
            this, &DesignerController::onCutActionTrigger);
    connect(m_designerPane->copyAction(), &QAction::triggered,
            this, &DesignerController::onCopyActionTrigger);
    connect(m_designerPane->pasteAction(), &QAction::triggered,
            this, &DesignerController::onPasteActionTrigger);
    connect(m_designerPane->deleteAction(), &QAction::triggered,
            this, &DesignerController::onDeleteActionTrigger);
    connect(m_designerPane->deleteAllAction(), &QAction::triggered,
            this, &DesignerController::onClearButtonClick);
    connect(m_designerPane->moveLeftAction(), &QAction::triggered,
            this, &DesignerController::onMoveLeftActionTrigger);
    connect(m_designerPane->moveRightAction(), &QAction::triggered,
            this, &DesignerController::onMoveRightActionTrigger);
    connect(m_designerPane->moveUpAction(), &QAction::triggered,
            this, &DesignerController::onMoveUpActionTrigger);
    connect(m_designerPane->moveDownAction(), &QAction::triggered,
            this, &DesignerController::onMoveDownActionTrigger);
}

void DesignerController::charge()
{
    if (!ProjectManager::isStarted())
        return;

    m_designerPane->themeComboBox()->setCurrentIndex(0);

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
    m_designerPane->menu()->exec(globalPos);
    m_menuTargetControl = nullptr;
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
    for (const QString& name : UtilityFunctions::anchorLineNames()) {
        ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                           name, QString(),
                                           ControlPropertyManager::SaveChanges);
        ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                           name, "undefined",
                                           ControlPropertyManager::UpdateRenderer);
    }
    for (const QString& name : UtilityFunctions::anchorPropertyNames()) {
        if (name == "anchors.leftMargin"
                || name == "anchors.rightMargin"
                || name == "anchors.topMargin"
                || name == "anchors.bottomMargin") {
            ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                               name, QString(), ControlPropertyManager::SaveChanges);
            ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                               name, "undefined", ControlPropertyManager::UpdateRenderer);
        } else {
            ControlPropertyManager::setProperty(m_designerPane->anchorEditor()->sourceControl(),
                                                name, QString(),
                                                name.contains("alignWhenCentered") ? 1 : 0,
                                                ControlPropertyManager::SaveChanges |
                                                ControlPropertyManager::UpdateRenderer);
        }
    }
}

void DesignerController::onAnchorSourceControlActivation()
{
    m_designerPane->designerView()->scene()->clearSelection();
    m_designerPane->anchorEditor()->sourceControl()->setSelected(true);
}

void DesignerController::onAnchor(AnchorLine::Type sourceLineType, const AnchorLine& targetLine)
{
    if (targetLine.isValid()) {
        ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                           "anchors." + anchorLineText(sourceLineType),
                                           cleanId(m_designerPane->anchorEditor()->sourceControl(),
                                                   targetLine.control())
                                           + "." + anchorLineText(targetLine.type()),
                                           ControlPropertyManager::SaveChanges |
                                           ControlPropertyManager::UpdateRenderer);
    } else {
        ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                           "anchors." + anchorLineText(sourceLineType),
                                           QString(), ControlPropertyManager::SaveChanges);
        ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                           "anchors." + anchorLineText(sourceLineType),
                                           "undefined", ControlPropertyManager::UpdateRenderer);
    }
}

void DesignerController::onAnchorFill(Control* control)
{
    if (control) {
        ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                           "anchors.fill",
                                           cleanId(m_designerPane->anchorEditor()->sourceControl(), control),
                                           ControlPropertyManager::SaveChanges |
                                           ControlPropertyManager::UpdateRenderer);
    } else {
        ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                           "anchors.fill", QString(),
                                           ControlPropertyManager::SaveChanges);
        ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                           "anchors.fill", "undefined",
                                           ControlPropertyManager::UpdateRenderer);
    }
}

void DesignerController::onAnchorCenter(Control* control, bool overlay)
{
    if (control) {
        ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                           "anchors.centerIn",
                                           cleanId(m_designerPane->anchorEditor()->sourceControl(), control),
                                           ControlPropertyManager::SaveChanges |
                                           ControlPropertyManager::UpdateRenderer);
    } else {
        if (m_designerPane->anchorEditor()->sourceControl()->popup() && overlay) {
            ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                               "anchors.centerIn", "Overlay.overlay",
                                               ControlPropertyManager::SaveChanges |
                                               ControlPropertyManager::UpdateRenderer);
        } else {
            ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                               "anchors.centerIn", QString(),
                                               ControlPropertyManager::SaveChanges);
            ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                               "anchors.centerIn", "undefined",
                                               ControlPropertyManager::UpdateRenderer);
        }
    }
}

void DesignerController::onAnchorMarginOffsetEdit(AnchorLine::Type sourceLineType, qreal marginOffset)
{
    if (marginOffset == 0 && (sourceLineType == AnchorLine::Left
                              || sourceLineType == AnchorLine::Right
                              || sourceLineType == AnchorLine::Top
                              || sourceLineType == AnchorLine::Bottom)) {
        ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                           "anchors." + marginOffsetText(sourceLineType),
                                           QString(), ControlPropertyManager::SaveChanges);
        ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                           "anchors." + marginOffsetText(sourceLineType),
                                           "undefined", ControlPropertyManager::UpdateRenderer);
    } else {
        ControlPropertyManager::setProperty(m_designerPane->anchorEditor()->sourceControl(),
                                            "anchors." + marginOffsetText(sourceLineType),
                                            marginOffset == 0 ? QString() : QString::number(marginOffset), marginOffset,
                                            ControlPropertyManager::SaveChanges |
                                            ControlPropertyManager::UpdateRenderer);
    }
}

void DesignerController::onAnchorMarginsEdit(qreal margins)
{
    ControlPropertyManager::setProperty(m_designerPane->anchorEditor()->sourceControl(),
                                        "anchors.margins",
                                        margins == 0 ? QString() : QString::number(margins), margins,
                                        ControlPropertyManager::SaveChanges |
                                        ControlPropertyManager::UpdateRenderer);
}

void DesignerController::onAnchorAlignmentActivation(bool aligned)
{
    ControlPropertyManager::setProperty(m_designerPane->anchorEditor()->sourceControl(),
                                        "anchors.alignWhenCentered",
                                        aligned ? QString() : "false", aligned,
                                        ControlPropertyManager::SaveChanges |
                                        ControlPropertyManager::UpdateRenderer);
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

void DesignerController::onRefreshButtonClick()
{

}

void DesignerController::onClearButtonClick()
{

}

void DesignerController::onAnchorsButtonClick()
{

}

void DesignerController::onSnappingButtonClick()
{

}

void DesignerController::onGridViewButtonClick()
{

}

void DesignerController::onGuidelinesButtonClick()
{

}

void DesignerController::onSceneSettingsButtonClick()
{

}

void DesignerController::onThemeSettingsButtonClick()
{

}

void DesignerController::onZoomLevelComboBoxActivation(const QString& currentText)
{

}

void DesignerController::onThemeComboBoxActivation(const QString& currentText)
{

}

void DesignerController::onToggleSelectionActionTrigger()
{

}

void DesignerController::onSelectAllActionTrigger()
{

}

void DesignerController::onSendBackActionTrigger()
{

}

void DesignerController::onBringFrontActionTrigger()
{

}

void DesignerController::onCutActionTrigger()
{

}

void DesignerController::onCopyActionTrigger()
{

}

void DesignerController::onPasteActionTrigger()
{

}

void DesignerController::onDeleteActionTrigger()
{

}

void DesignerController::onMoveLeftActionTrigger()
{

}

void DesignerController::onMoveRightActionTrigger()
{

}

void DesignerController::onMoveUpActionTrigger()
{

}
#include <private/qgraphicsitem_p.h>
void DesignerController::onMoveDownActionTrigger()
{
    DesignerScene* scene = m_designerPane->designerView()->scene();
    QSet<Control*> movableSelectedAncestorControls;
    const QList<Control*>& selectedControls = scene->selectedControls();

    if (m_menuTargetControl == 0) {
        if (!selectedControls.isEmpty()) {
            Control* ancestor = selectedControls.first();
            Control* myMovableSelectedAncestorControl = ancestor;
            while (Control* parent = ancestor->parentControl()) {
                if (parent->isSelected() && (parent->flags() & Control::ItemIsMovable))
                    myMovableSelectedAncestorControl = parent;
                ancestor = parent;
            }

            for (Control* selectedControl : selectedControls) {
                if (selectedControl->flags() & Control::ItemIsMovable) {
                    if (!QGraphicsItemPrivate::movableAncestorIsSelected(selectedControl))
                        movableSelectedAncestorControls.insert(selectedControl);
                }
            }

            movableSelectedAncestorControls.remove(myMovableSelectedAncestorControl);

            const QList<Control*>& ancestorSiblings = myMovableSelectedAncestorControl->siblings();
            for (Control* movableSelectedAncestorControl : movableSelectedAncestorControls.toList()) {
                if (!ancestorSiblings.contains(movableSelectedAncestorControl)) {
                    for (Control* childControl : movableSelectedAncestorControl->childControls())
                        childControl->setSelected(false);
                    movableSelectedAncestorControl->setSelected(false);
                    movableSelectedAncestorControls.remove(movableSelectedAncestorControl);
                }
            }

            movableSelectedAncestorControls.insert(myMovableSelectedAncestorControl);
        }
    } else {
        movableSelectedAncestorControls.insert(m_menuTargetControl);
    }

    for (Control* movableSelectedAncestorControl : movableSelectedAncestorControls) {
        if (movableSelectedAncestorControl->type() != Form::Type) {
            ControlPropertyManager::setPos(movableSelectedAncestorControl,
                                           movableSelectedAncestorControl->pos() +
                                           QPointF(0, DesignerSettings::sceneSettings()->gridSize),
                                           ControlPropertyManager::SaveChanges |
                                           ControlPropertyManager::UpdateRenderer |
                                           ControlPropertyManager::CompressedCall |
                                           ControlPropertyManager::DontApplyDesigner);
        }
    }
}
