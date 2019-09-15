#include <designercontroller.h>
#include <designerpane.h>
#include <designerview.h>
#include <designerscene.h>
#include <saveutils.h>
#include <form.h>
#include <anchoreditor.h>
#include <utilityfunctions.h>
#include <designersettings.h>
#include <scenesettings.h>
#include <windowmanager.h>
#include <preferenceswindow.h>
#include <modemanager.h>
#include <projectmanager.h>
#include <controlpropertymanager.h>
#include <controlremovingmanager.h>
#include <controlrenderingmanager.h>
#include <controlcreationmanager.h>

#include <private/qgraphicsitem_p.h>

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
{
    const DesignerScene* scene = m_designerPane->designerView()->scene();

    m_designerPane->themeComboBox()->addItem(QStringLiteral("Default"));
    m_designerPane->themeComboBox()->addItem(QStringLiteral("Fusion"));
    m_designerPane->themeComboBox()->addItem(QStringLiteral("Imagine"));
    m_designerPane->themeComboBox()->addItem(QStringLiteral("Material"));
    m_designerPane->themeComboBox()->addItem(QStringLiteral("Universal"));
    m_designerPane->zoomLevelComboBox()->addItems(UtilityFunctions::zoomTexts());

    onSceneSettingsChange();

    connect(DesignerSettings::instance(), &DesignerSettings::sceneSettingsChanged,
            this, &DesignerController::onSceneSettingsChange);
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

    connect(m_designerPane->refreshButton(), &QToolButton::clicked,
            this, &DesignerController::onRefreshButtonClick);
    connect(m_designerPane->shrinkSceneButton(), &QToolButton::clicked,
            this, &DesignerController::onShrinkSceneButtonClick);
    connect(m_designerPane->clearButton(), &QToolButton::clicked,
            this, &DesignerController::onClearButtonClick);
    connect(m_designerPane->anchorsButton(), &QToolButton::clicked,
            this, &DesignerController::onAnchorsButtonClick);
    connect(m_designerPane->snappingButton(), &QToolButton::clicked,
            this, &DesignerController::onSnappingButtonClick);
    connect(m_designerPane->gridViewButton(), &QToolButton::clicked,
            this, &DesignerController::onGridViewButtonClick);
    connect(m_designerPane->guidelinesButton(), &QToolButton::clicked,
            this, &DesignerController::onGuidelinesButtonClick);
    connect(m_designerPane->sceneSettingsButton(), &QToolButton::clicked,
            this, &DesignerController::onSceneSettingsButtonClick);
    connect(m_designerPane->themeSettingsButton(), &QToolButton::clicked,
            this, &DesignerController::onThemeSettingsButtonClick);
    connect(m_designerPane->zoomLevelComboBox(), qOverload<const QString&>(&QComboBox::activated),
            this, &DesignerController::onZoomLevelComboBoxActivation);
    connect(m_designerPane->themeComboBox(), qOverload<const QString&>(&QComboBox::activated),
            this, &DesignerController::projectThemeActivated);

    connect(m_designerPane->invertSelectionAction(), &QAction::triggered,
            this, &DesignerController::onInvertSelectionActionTrigger);
    connect(m_designerPane->selectAllAction(), &QAction::triggered,
            this, &DesignerController::onSelectAllActionTrigger);
    connect(m_designerPane->refreshAction(), &QAction::triggered,
            this, &DesignerController::onRefreshButtonClick);
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

void DesignerController::onSceneSettingsChange()
{
    const SceneSettings* settings = DesignerSettings::sceneSettings();
    m_designerPane->zoomLevelComboBox()->setCurrentText(UtilityFunctions::zoomLevelToText(settings->sceneZoomLevel));
    m_designerPane->anchorsButton()->setChecked(settings->showAllAnchors);
    m_designerPane->snappingButton()->setChecked(settings->snappingEnabled);
    m_designerPane->gridViewButton()->setChecked(settings->showGridViewDots);
    m_designerPane->guidelinesButton()->setChecked(settings->showGuideLines);
}

void DesignerController::onCustomContextMenuRequest(const QPoint& pos)
{
    const DesignerView* view = m_designerPane->designerView();
    const QPoint& globalPos = m_designerPane->mapToGlobal(pos);
    const QPointF& scenePos = view->mapToScene(view->viewport()->mapFromGlobal(globalPos));
    DesignerScene* scene = view->scene();

    if (scene->currentForm() == 0)
        return;

    Control* controlUnderCursor = scene->topLevelControl(scenePos);
    if (controlUnderCursor && !controlUnderCursor->isSelected()) {
        scene->clearSelection();
        controlUnderCursor->setSelected(true);
    }

    const QList<Control*>& selectedControls = scene->selectedControls();
    int selectedSize = selectedControls.size();
    bool onlyForm = selectedSize == 1 && selectedControls.first()->type() == Form::Type;

    m_copyPaste.setPos(scenePos);
    m_designerPane->sendBackAction()->setEnabled(selectedSize == 1 && !onlyForm);
    m_designerPane->bringFrontAction()->setEnabled(selectedSize == 1 && !onlyForm);
    m_designerPane->cutAction()->setEnabled(selectedSize > 0 && !onlyForm);
    m_designerPane->copyAction()->setEnabled(selectedSize > 0 && !onlyForm);
    m_designerPane->pasteAction()->setEnabled(m_copyPaste.isValid());
    m_designerPane->deleteAction()->setEnabled(selectedSize > 0 && !onlyForm);
    m_designerPane->moveLeftAction()->setEnabled(selectedSize > 0);
    m_designerPane->moveRightAction()->setEnabled(selectedSize > 0);
    m_designerPane->moveUpAction()->setEnabled(selectedSize > 0);
    m_designerPane->moveDownAction()->setEnabled(selectedSize > 0);
    m_designerPane->menu()->exec(globalPos);
    m_copyPaste.setPos(QPointF());
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
            scene->setAnchorVisibility(scene->anchorVisibility() | DesignerScene::VisibleForAllControlsDueToAnchorEditor);
            scene->clearSelection();
            sourceControl->setSelected(true);
            m_designerPane->anchorEditor()->setSourceControl(sourceControl);
            m_designerPane->anchorEditor()->setPrimaryTargetControl(targetControl);
            m_designerPane->anchorEditor()->refresh();
            m_designerPane->anchorEditor()->exec();
            scene->clearSelection();
            scene->setAnchorVisibility(scene->anchorVisibility() & ~DesignerScene::VisibleForAllControlsDueToAnchorEditor);
            for (Control* control : selection)
                control->setSelected(true);
        }
    }
}

void DesignerController::onAnchorClear()
{
    const QStringList& lines = UtilityFunctions::anchorLineNames();
    for (const QString& name : lines) {
        ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                           name, QString(),
                                           ControlPropertyManager::SaveChanges);
        ControlPropertyManager::setBinding(m_designerPane->anchorEditor()->sourceControl(),
                                           name, "undefined",
                                           ControlPropertyManager::UpdateRenderer);
    }
    const QStringList& properties = UtilityFunctions::anchorPropertyNames();
    for (const QString& name : properties) {
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
    scene->setAnchorVisibility(scene->anchorVisibility() | DesignerScene::VisibleForAllControlsDueToAnchorEditorConnection);
    m_designerPane->anchorEditor()->setSourceControl(sourceControl);
    m_designerPane->anchorEditor()->setPrimaryTargetControl(targetControl);
    m_designerPane->anchorEditor()->refresh();
    m_designerPane->anchorEditor()->exec();
    scene->setAnchorVisibility(scene->anchorVisibility() & ~DesignerScene::VisibleForAllControlsDueToAnchorEditorConnection);
    scene->clearSelection();
    for (Control* control : selection)
        control->setSelected(true);
}

void DesignerController::onRefreshButtonClick()
{
    if (Form* currentForm = m_designerPane->designerView()->scene()->currentForm())
        ControlRenderingManager::scheduleRefresh(currentForm->uid());
}

void DesignerController::onShrinkSceneButtonClick()
{
    m_designerPane->designerView()->scene()->shrinkSceneRect();
}

void DesignerController::onClearButtonClick()
{
    const DesignerScene* scene = m_designerPane->designerView()->scene();

    if (!scene->currentForm())
        return;

    QMessageBox::StandardButton ret = UtilityFunctions::showMessage(
                m_designerPane,
                tr("Do you want to continue?"),
                tr("This will delete all the controls on the current form."),
                QMessageBox::Question,
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);

    switch (ret) {
    case QMessageBox::Yes:
        ControlRemovingManager::removeControls(scene->currentForm()->childControls(false), true);
        break;
    default:
        break;
    }
}

void DesignerController::onAnchorsButtonClick()
{
    SceneSettings* settings = DesignerSettings::sceneSettings();
    bool showAllAnchors = m_designerPane->anchorsButton()->isChecked();
    if (settings->showAllAnchors != showAllAnchors) {
        settings->showAllAnchors = showAllAnchors;
        settings->write();
    }
}

void DesignerController::onSnappingButtonClick()
{
    SceneSettings* settings = DesignerSettings::sceneSettings();
    bool snappingEnabled = m_designerPane->snappingButton()->isChecked();
    if (settings->snappingEnabled != snappingEnabled) {
        settings->snappingEnabled = snappingEnabled;
        settings->write();
    }
}

void DesignerController::onGridViewButtonClick()
{
    SceneSettings* settings = DesignerSettings::sceneSettings();
    bool showGridViewDots = m_designerPane->gridViewButton()->isChecked();
    if (settings->showGridViewDots != showGridViewDots) {
        settings->showGridViewDots = showGridViewDots;
        settings->write();
    }
}

void DesignerController::onGuidelinesButtonClick()
{
    SceneSettings* settings = DesignerSettings::sceneSettings();
    bool showGuideLines = m_designerPane->guidelinesButton()->isChecked();
    if (settings->showGuideLines != showGuideLines) {
        settings->showGuideLines = showGuideLines;
        settings->write();
    }
}

void DesignerController::onSceneSettingsButtonClick()
{
    WindowManager::preferencesWindow()->show();
    WindowManager::preferencesWindow()->raise();
    WindowManager::preferencesWindow()->activateWindow();
    WindowManager::preferencesWindow()->updateGeometry();
    WindowManager::preferencesWindow()->setCurrentWidget(PreferencesWindow::SceneSettingsWidget);
}

void DesignerController::onThemeSettingsButtonClick()
{
    ModeManager::setMode(ModeManager::Options);
}

void DesignerController::onZoomLevelComboBoxActivation(const QString& currentText)
{
    SceneSettings* settings = DesignerSettings::sceneSettings();
    qreal sceneZoomLevel = UtilityFunctions::textToZoomLevel(currentText);
    if (settings->sceneZoomLevel != sceneZoomLevel) {
        settings->sceneZoomLevel = sceneZoomLevel;
        settings->write();
    }
}

void DesignerController::onInvertSelectionActionTrigger()
{
    if (Form* currentForm = m_designerPane->designerView()->scene()->currentForm()) {
        currentForm->setSelected(!currentForm->isSelected());
        const QList<Control*>& childControls = currentForm->childControls();
        for (Control* control : childControls)
            control->setSelected(!control->isSelected());
    }
}

void DesignerController::onSelectAllActionTrigger()
{
    if (Form* currentForm = m_designerPane->designerView()->scene()->currentForm()) {
        currentForm->setSelected(true);
        const QList<Control*>& childControls = currentForm->childControls();
        for (Control* control : childControls)
            control->setSelected(true);
    }
}

void DesignerController::onSendBackActionTrigger()
{
    const QList<Control*>& selectedControls = m_designerPane->designerView()->scene()->selectedControls();
    if (selectedControls.size() != 1)
        return;
    Control* control = selectedControls.first();
    if (control->parentControl() == 0)
        return;
    ControlPropertyManager::Options options = ControlPropertyManager::NoOption;
    if (control->gui() && !control->window() && !control->popup()) {
        options = ControlPropertyManager::SaveChanges
                | ControlPropertyManager::UpdateRenderer
                | ControlPropertyManager::CompressedCall
                | ControlPropertyManager::DontApplyDesigner;
    }
    qreal lowerZ = DesignerScene::lowerZ(control->parentControl());
    // FIXME: Add this whenever we are able to manage raising or lowering
    // Controls based on their indexes,
    // if (lowerZ != control->zValue())
    ControlPropertyManager::setZ(control, lowerZ - 1, options);
}

void DesignerController::onBringFrontActionTrigger()
{
    const QList<Control*>& selectedControls = m_designerPane->designerView()->scene()->selectedControls();
    if (selectedControls.size() != 1)
        return;
    Control* control = selectedControls.first();
    if (control->parentControl() == 0)
        return;
    ControlPropertyManager::Options options = ControlPropertyManager::NoOption;
    if (control->gui() && !control->window() && !control->popup()) {
        options = ControlPropertyManager::SaveChanges
                | ControlPropertyManager::UpdateRenderer
                | ControlPropertyManager::CompressedCall
                | ControlPropertyManager::DontApplyDesigner;
    }
    qreal higherZ = DesignerScene::higherZ(control->parentControl());
    // FIXME: Add this whenever we are able to manage raising or lowering
    // Controls based on their indexes,
    // if (higherZ != control->zValue())
    ControlPropertyManager::setZ(control, higherZ + 1, options);
}

void DesignerController::onCutActionTrigger()
{
    m_copyPaste.invalidate();
    m_copyPaste.setControls(selectedAncestorControls());
    m_copyPaste.setActionType(CopyPaste::Cut);
}

void DesignerController::onCopyActionTrigger()
{
    m_copyPaste.invalidate();
    m_copyPaste.setControls(selectedAncestorControls());
    m_copyPaste.setActionType(CopyPaste::Copy);
}

void DesignerController::onPasteActionTrigger()
{
    if (!m_copyPaste.isValid())
        return;

    DesignerScene* scene = m_designerPane->designerView()->scene();
    Control* parentControl = scene->currentForm();
    const CopyPaste::ActionType actionType = m_copyPaste.actionType();
    const QList<Control*>& controls = m_copyPaste.controls();
    const QPointF& copyPos = m_copyPaste.pos();

    if (actionType == CopyPaste::Cut)
        m_copyPaste.invalidate();

    const QList<Control*>& selectedControls = scene->selectedControls();
    if (selectedControls.size() == 1)
        parentControl = selectedControls.first();
    const QPointF margins(parentControl->margins().left(), parentControl->margins().top());

    bool clearSelection = false;
    QList<Control*> newControls;
    for (Control* control : controls) {
        if (actionType == CopyPaste::Cut) {
            if (control == parentControl)
                continue;
            if (control->parentControl() == parentControl)
                continue;
            QPointF pos = control->mapToItem(parentControl, QPointF(10, 10) - margins);
            if (!copyPos.isNull())
                pos = parentControl->mapFromScene(copyPos);
            scene->reparentControl(control, parentControl, DesignerScene::snapPosition(pos));
            clearSelection = true;
            newControls.append(control);
        } else {
            // NOTE: Move the item position backwards as much as next parent margins are
            // Because it will be followed by a ControlPropertyManager::setParent call and it
            // will move the item by setting a transform on it according to its parent margin
            m_copyPaste.increaseCount();
            qreal xy = 10 * m_copyPaste.count();
            QPointF pos = control->mapToItem(parentControl, QPointF(xy, xy) - margins);
            if (!copyPos.isNull())
                pos = parentControl->mapFromScene(copyPos);
            Control* newControl = ControlCreationManager::createControl(
                        parentControl, control->dir(), DesignerScene::snapPosition(pos),
                        control->size(), control->pixmap());
            if (newControl) {
                clearSelection = true;
                newControls.append(newControl);
            }
        }
    }

    if (clearSelection) {
        scene->clearSelection();
        for (Control* newControl : qAsConst(newControls))
            newControl->setSelected(true);
    }
}

void DesignerController::onDeleteActionTrigger()
{
    ControlRemovingManager::removeControls(m_designerPane->designerView()->scene()->selectedControls(), true);
}

void DesignerController::onMoveLeftActionTrigger()
{
    const DesignerScene* scene = m_designerPane->designerView()->scene();
    const QList<Control*>& controls = movableSelectedAncestorControls(scene->selectedControls());
    for (Control* control : controls) {
        ControlPropertyManager::Options options = ControlPropertyManager::NoOption;
        if (control->type() != Form::Type) {
            options = ControlPropertyManager::SaveChanges
                    | ControlPropertyManager::UpdateRenderer
                    | ControlPropertyManager::CompressedCall
                    | ControlPropertyManager::DontApplyDesigner;
        }
        ControlPropertyManager::setPos(control, control->pos() +
                                       QPointF(-DesignerSettings::sceneSettings()->gridSize, 0),
                                       options);
    }
}

void DesignerController::onMoveRightActionTrigger()
{
    const DesignerScene* scene = m_designerPane->designerView()->scene();
    const QList<Control*>& controls = movableSelectedAncestorControls(scene->selectedControls());
    for (Control* control : controls) {
        ControlPropertyManager::Options options = ControlPropertyManager::NoOption;
        if (control->type() != Form::Type) {
            options = ControlPropertyManager::SaveChanges
                    | ControlPropertyManager::UpdateRenderer
                    | ControlPropertyManager::CompressedCall
                    | ControlPropertyManager::DontApplyDesigner;
        }
        ControlPropertyManager::setPos(control, control->pos() +
                                       QPointF(DesignerSettings::sceneSettings()->gridSize, 0),
                                       options);
    }
}

void DesignerController::onMoveUpActionTrigger()
{
    const DesignerScene* scene = m_designerPane->designerView()->scene();
    const QList<Control*>& controls = movableSelectedAncestorControls(scene->selectedControls());
    for (Control* control : controls) {
        ControlPropertyManager::Options options = ControlPropertyManager::NoOption;
        if (control->type() != Form::Type) {
            options = ControlPropertyManager::SaveChanges
                    | ControlPropertyManager::UpdateRenderer
                    | ControlPropertyManager::CompressedCall
                    | ControlPropertyManager::DontApplyDesigner;
        }
        ControlPropertyManager::setPos(control, control->pos() +
                                       QPointF(0, -DesignerSettings::sceneSettings()->gridSize),
                                       options);
    }
}

void DesignerController::onMoveDownActionTrigger()
{
    const DesignerScene* scene = m_designerPane->designerView()->scene();
    const QList<Control*>& controls = movableSelectedAncestorControls(scene->selectedControls());
    for (Control* control : controls) {
        ControlPropertyManager::Options options = ControlPropertyManager::NoOption;
        if (control->type() != Form::Type) {
            options = ControlPropertyManager::SaveChanges
                    | ControlPropertyManager::UpdateRenderer
                    | ControlPropertyManager::CompressedCall
                    | ControlPropertyManager::DontApplyDesigner;
        }
        ControlPropertyManager::setPos(control, control->pos() +
                                       QPointF(0, DesignerSettings::sceneSettings()->gridSize),
                                       options);
    }
}

QList<Control*> DesignerController::selectedAncestorControls() const
{
    const DesignerScene* scene = m_designerPane->designerView()->scene();
    QList<Control*> selectedControls(scene->selectedControls());
    selectedControls.removeOne(scene->currentForm());

    const QList<Control*> copy(selectedControls);
    for (const Control* control : copy) {
        const QList<Control*>& childControls = control->childControls();
        for (Control* childControl : childControls)
            selectedControls.removeOne(childControl);
    }

    return selectedControls;
}

QList<Control*> DesignerController::movableSelectedAncestorControls(const QList<Control*>& selectedControls) const
{
    QSet<Control*> movableSelectedAncestorControls;

    if (selectedControls.isEmpty())
        return movableSelectedAncestorControls.toList();

    const DesignerScene* scene = m_designerPane->designerView()->scene();

    if (scene->currentForm() && scene->currentForm()->isSelected()) {
        movableSelectedAncestorControls.insert(scene->currentForm());
        return movableSelectedAncestorControls.toList();
    }

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
    const QList<Control*>& controls = movableSelectedAncestorControls.toList();
    for (Control* movableSelectedAncestorControl : controls) {
        if (!ancestorSiblings.contains(movableSelectedAncestorControl)) {
            const QList<Control*>& childControls = movableSelectedAncestorControl->childControls();
            for (Control* childControl : childControls)
                childControl->setSelected(false);
            movableSelectedAncestorControl->setSelected(false);
            movableSelectedAncestorControls.remove(movableSelectedAncestorControl);
        }
    }

    movableSelectedAncestorControls.insert(myMovableSelectedAncestorControl);
    return movableSelectedAncestorControls.toList();
}
