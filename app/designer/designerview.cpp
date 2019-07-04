#include <designerview.h>
#include <designerview.h>
#include <designerscene.h>
#include <utilsicons.h>
#include <controlcreationmanager.h>
#include <controlpropertymanager.h>
#include <controlremovingmanager.h>
#include <controlrenderingmanager.h>
#include <saveutils.h>
#include <transparentstyle.h>
#include <signalchooserdialog.h>
#include <utilityfunctions.h>
#include <qmlcodeeditorwidget.h>
#include <parserutils.h>
#include <projectmanager.h>
#include <qmlcodeeditor.h>
#include <qmlcodedocument.h>
#include <designersettings.h>
#include <scenesettings.h>

#include <QMenu>
#include <QDir>
#include <QToolBar>
#include <QToolButton>
#include <QPainter>
#include <QMessageBox>
#include <QComboBox>
#include <QInputDialog>

// 1. Control name; 2. Method name
#define METHOD_DECL "%1_%2"
#define FORM_DECL "%1_onCompleted"
#define METHOD_BODY \
    "\n\n"\
    "function %1() {\n"\
    "    // Do something...\n"\
    "}"

namespace {

// TODO: Improve copy-paste positioning. "Pasting into a sub control and" "positioning wherever you
// right click".. implement those 2 features in future

struct CopyPaste final
{
    enum ActionType { Invalid, Copy, Cut };

    CopyPaste() = delete;
    CopyPaste(const CopyPaste&) = delete;
    CopyPaste &operator=(const CopyPaste&) = delete;

    static inline ActionType actionType()
    { return s_actionType; }
    static inline QList<QPointer<Control>> controls()
    { return s_controls; }
    static inline bool isValid()
    { return s_actionType != Invalid && !s_controls.isEmpty(); }
    static inline void invalidate()
    { s_actionType = Invalid; s_controls.clear(); }
    static inline void setControls(const QList<QPointer<Control>>& value, ActionType actionType)
    { s_controls = value; s_actionType = actionType; }

private:
    static ActionType s_actionType;
    static QList<QPointer<Control>> s_controls;
};
QList<QPointer<Control>> CopyPaste::s_controls;
CopyPaste::ActionType CopyPaste::s_actionType = CopyPaste::Invalid;

QString methodName(const QString& signal)
{
    QString method(signal);
    method.replace(0, 1, method[0].toUpper());
    return method.prepend("on");
}

bool warnIfFileDoesNotExist(const QString& filePath)
{
    if (!QFileInfo::exists(filePath)) {
        return UtilityFunctions::showMessage(
                    nullptr, QObject::tr("Oops"),
                    QObject::tr("File %1 does not exist.").arg(QFileInfo(filePath).fileName()));
    }
    return false;
}
}

DesignerView::DesignerView(QmlCodeEditorWidget* qmlCodeEditorWidget, QWidget *parent) : QGraphicsView(new DesignerScene, parent)
  , m_lastScale(1.0)
  , m_signalChooserDialog(new SignalChooserDialog(this))
  , m_qmlCodeEditorWidget(qmlCodeEditorWidget)
  , m_toolBar(new QToolBar)
  , m_undoButton(new QToolButton)
  , m_redoButton(new QToolButton)
  , m_clearButton(new QToolButton)
  , m_refreshButton(new QToolButton)
  , m_snappingButton(new QToolButton)
  , m_fitButton(new QToolButton)
  , m_outlineButton(new QToolButton)
  , m_hideDockWidgetTitleBarsButton(new QToolButton)
  , m_zoomlLevelCombobox(new QComboBox)
  , m_menu(new QMenu(this))
  , m_sendBackAct(new QAction(this))
  , m_bringFrontAct(new QAction(this))
  , m_undoAct(new QAction(this))
  , m_redoAct(new QAction(this))
  , m_cutAct(new QAction(this))
  , m_copyAct(new QAction(this))
  , m_pasteAct(new QAction(this))
  , m_selectAllAct(new QAction(this))
  , m_deleteAct(new QAction(this))
  , m_moveUpAct(new QAction(this))
  , m_moveDownAct(new QAction(this))
  , m_moveRightAct(new QAction(this))
  , m_moveLeftAct(new QAction(this))
{
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setRubberBandSelectionMode(Qt::IntersectsItemShape);
    setDragMode(QGraphicsView::RubberBandDrag);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    setFrameShape(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_zoomlLevelCombobox->addItem("10 %");
    m_zoomlLevelCombobox->addItem("25 %");
    m_zoomlLevelCombobox->addItem("50 %");
    m_zoomlLevelCombobox->addItem("75 %");
    m_zoomlLevelCombobox->addItem("90 %");
    m_zoomlLevelCombobox->addItem("100 %");
    m_zoomlLevelCombobox->addItem("125 %");
    m_zoomlLevelCombobox->addItem("150 %");
    m_zoomlLevelCombobox->addItem("175 %");
    m_zoomlLevelCombobox->addItem("200 %");
    m_zoomlLevelCombobox->addItem("300 %");
    m_zoomlLevelCombobox->addItem("500 %");
    m_zoomlLevelCombobox->addItem("1000 %");
    m_zoomlLevelCombobox->setCurrentIndex(5);
    m_zoomlLevelCombobox->setMinimumWidth(100);
    m_zoomlLevelCombobox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    const SceneSettings* settings = DesignerSettings::instance()->sceneSettings();
    m_outlineButton->setCheckable(true);
    m_outlineButton->setChecked(scene()->showOutlines());
    m_hideDockWidgetTitleBarsButton->setCheckable(true);
    m_hideDockWidgetTitleBarsButton->setChecked(false);
    m_snappingButton->setCheckable(true);
    m_snappingButton->setChecked(settings->snappingEnabled);

    m_refreshButton->setCursor(Qt::PointingHandCursor);
    m_clearButton->setCursor(Qt::PointingHandCursor);
    m_undoButton->setCursor(Qt::PointingHandCursor);
    m_redoButton->setCursor(Qt::PointingHandCursor);
    m_snappingButton->setCursor(Qt::PointingHandCursor);
    m_outlineButton->setCursor(Qt::PointingHandCursor);
    m_hideDockWidgetTitleBarsButton->setCursor(Qt::PointingHandCursor);
    m_fitButton->setCursor(Qt::PointingHandCursor);
    m_zoomlLevelCombobox->setCursor(Qt::PointingHandCursor);

    m_refreshButton->setToolTip("Refresh control rendering on the Dashboard.");
    m_clearButton->setToolTip("Clear controls on the Dashboard.");
    m_undoButton->setToolTip("Undo action.");
    m_redoButton->setToolTip("Redo action.");
    m_snappingButton->setToolTip("Enable snapping to help aligning of controls to each others.");
    m_outlineButton->setToolTip("Show outline frame for controls.");
    m_hideDockWidgetTitleBarsButton->setToolTip("Hide/Show title bars of Panes.");
    m_fitButton->setToolTip("Fit scene into the Dashboard.");
    m_zoomlLevelCombobox->setToolTip("Change zoom level.");

    m_refreshButton->setIcon(Utils::Icons::RELOAD.icon());
    m_clearButton->setIcon(Utils::Icons::CLEAN_TOOLBAR.icon());
    m_undoButton->setIcon(Utils::Icons::UNDO_TOOLBAR.icon());
    m_redoButton->setIcon(Utils::Icons::REDO_TOOLBAR.icon());
    m_snappingButton->setIcon(Utils::Icons::SNAPPING_TOOLBAR.icon());
    m_outlineButton->setIcon(Utils::Icons::BOUNDING_RECT.icon());
    m_hideDockWidgetTitleBarsButton->setIcon(Utils::Icons::CLOSE_SPLIT_TOP.icon());
    m_fitButton->setIcon(Utils::Icons::FITTOVIEW_TOOLBAR.icon());

    connect(m_snappingButton, &QToolButton::toggled, this, &DesignerView::onSnappingButtonClick);
    connect(m_outlineButton, &QToolButton::toggled, this, &DesignerView::onOutlineButtonClick);
    connect(m_hideDockWidgetTitleBarsButton, &QToolButton::toggled, this, &DesignerView::hideDockWidgetTitleBars);
    connect(m_zoomlLevelCombobox, &QComboBox::currentTextChanged, this, &DesignerView::onZoomLevelChange);
    connect(m_fitButton, &QToolButton::clicked, this, &DesignerView::onFitButtonClick);
    connect(m_refreshButton, &QToolButton::clicked, this, &DesignerView::onRefreshButtonClick);
    connect(m_clearButton, &QToolButton::clicked, this, &DesignerView::onClearButtonClick);

    m_undoButton->setFixedHeight(20);
    m_redoButton->setFixedHeight(20);
    m_clearButton->setFixedHeight(20);
    m_refreshButton->setFixedHeight(20);
    m_snappingButton->setFixedHeight(20);
    m_fitButton->setFixedHeight(20);
    m_outlineButton->setFixedHeight(20);
    m_hideDockWidgetTitleBarsButton->setFixedHeight(20);
    m_zoomlLevelCombobox->setFixedHeight(20);

    m_toolBar->move(0, 0);
    m_toolBar->resize(200, 24);
    m_toolBar->setFixedHeight(24);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({2, 2}));
    m_toolBar->addWidget(m_undoButton);
    m_toolBar->addWidget(m_redoButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({1, 1}));
    m_toolBar->addSeparator();
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({1, 1}));
    m_toolBar->addWidget(m_refreshButton);
    m_toolBar->addWidget(m_clearButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({1, 1}));
    m_toolBar->addSeparator();
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({1, 1}));
    m_toolBar->addWidget(m_snappingButton);
    m_toolBar->addWidget(m_outlineButton);
    m_toolBar->addWidget(m_fitButton);
    m_toolBar->addWidget(m_zoomlLevelCombobox);
    m_toolBar->addWidget(UtilityFunctions::createSpacerWidget(Qt::Horizontal));
    m_toolBar->addWidget(m_hideDockWidgetTitleBarsButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({2, 2}));

    TransparentStyle::attach(m_toolBar);


    viewport()->setAutoFillBackground(false);

    m_sendBackAct->setText(tr("Send to Back"));
    m_bringFrontAct->setText(tr("Bring to Front"));
    m_undoAct->setText(tr("Undo"));
    m_undoAct->setShortcut(QKeySequence::Undo);
    m_redoAct->setText(tr("Redo"));
    m_redoAct->setShortcut(QKeySequence::Redo);
    m_cutAct->setText(tr("Cut"));
    m_cutAct->setShortcut(QKeySequence::Cut);
    m_copyAct->setText(tr("Copy"));
    m_copyAct->setShortcut(QKeySequence::Copy);
    m_pasteAct->setText(tr("Paste"));
    m_pasteAct->setShortcut(QKeySequence::Paste);
    m_deleteAct->setText(tr("Delete"));
    m_deleteAct->setShortcut(QKeySequence::Delete);
#if defined(Q_OS_MACOS)
    m_deleteAct->setShortcuts(QList<QKeySequence>() << (Qt::CTRL + Qt::Key_Backspace) << QKeySequence::Delete);
#endif
    m_selectAllAct->setText(tr("Select All"));
    m_selectAllAct->setShortcut(QKeySequence::SelectAll);
    m_moveUpAct->setShortcut(Qt::Key_Up);
    m_moveDownAct->setShortcut(Qt::Key_Down);
    m_moveRightAct->setShortcut(Qt::Key_Right);
    m_moveLeftAct->setShortcut(Qt::Key_Left);

    m_menu->addAction(m_sendBackAct);
    m_menu->addAction(m_bringFrontAct);
    m_menu->addSeparator();
    m_menu->addAction(m_undoAct);
    m_menu->addAction(m_redoAct);
    m_menu->addSeparator();
    m_menu->addAction(m_cutAct);
    m_menu->addAction(m_copyAct);
    m_menu->addAction(m_pasteAct);
    m_menu->addAction(m_deleteAct);
    m_menu->addSeparator();
    m_menu->addAction(m_selectAllAct);

    connect(m_undoAct, &QAction::triggered, this, &DesignerView::onUndoAction);
    connect(m_redoAct, &QAction::triggered, this, &DesignerView::onRedoAction);
    connect(m_cutAct, &QAction::triggered, this, &DesignerView::onCutAction);
    connect(m_copyAct, &QAction::triggered, this, &DesignerView::onCopyAction);
    connect(m_pasteAct, &QAction::triggered, this, &DesignerView::onPasteAction);
    connect(m_deleteAct, &QAction::triggered, this, &DesignerView::onDeleteAction);
    connect(m_selectAllAct, &QAction::triggered, this, &DesignerView::onSelectAllAction);
    connect(m_moveUpAct, &QAction::triggered, this, &DesignerView::onMoveUpAction);
    connect(m_moveDownAct, &QAction::triggered, this, &DesignerView::onMoveDownAction);
    connect(m_moveRightAct, &QAction::triggered, this, &DesignerView::onMoveRightAction);
    connect(m_moveLeftAct, &QAction::triggered, this, &DesignerView::onMoveLeftAction);
    connect(m_sendBackAct, &QAction::triggered, this, &DesignerView::onSendBackAction);
    connect(m_bringFrontAct, &QAction::triggered, this, &DesignerView::onBringFrontAction);

    addAction(m_undoAct);
    addAction(m_redoAct);
    addAction(m_cutAct);
    addAction(m_copyAct);
    addAction(m_pasteAct);
    addAction(m_deleteAct);
    addAction(m_selectAllAct);
    addAction(m_moveUpAct);
    addAction(m_moveDownAct);
    addAction(m_moveRightAct);
    addAction(m_moveLeftAct);
}

void DesignerView::scaleScene(qreal ratio)
{
    if (m_lastScale != ratio) {
        scale((1.0 / m_lastScale) * ratio, (1.0 / m_lastScale) * ratio);
        m_lastScale = ratio;
        emit scalingRatioChanged();
    }
}

qreal DesignerView::scalingRatio() const
{
    return m_lastScale;
}

void DesignerView::onSnappingButtonClick(bool value)
{
    scene()->setSnapping(value);
}

void DesignerView::onOutlineButtonClick(bool value)
{
    scene()->setShowOutlines(value);
}

void DesignerView::onFitButtonClick()
{
    qreal diff = qMin(width() / scene()->width(), height() / scene()->height());
    for (qreal level : UtilityFunctions::zoomLevels()) {
        if (UtilityFunctions::roundZoomLevel(diff) == level)
            m_zoomlLevelCombobox->setCurrentText(UtilityFunctions::zoomLevelToText(level));
    }
}

void DesignerView::onUndoButtonClick()
{
    // TODO:
}

void DesignerView::onRedoButtonClick()
{
    // TODO:
}

void DesignerView::onZoomLevelChange(const QString& text)
{
    qreal ratio = UtilityFunctions::textToZoomLevel(text);
    scaleScene(ratio);
}

void DesignerView::onRefreshButtonClick()
{
    ControlRenderingManager::scheduleRefresh(scene()->currentForm()->uid());
}

void DesignerView::onClearButtonClick()
{
    if (!scene()->currentForm())
        return;

    int ret = UtilityFunctions::showMessage(
                this, tr("This will remove the current scene's content"),
                tr("Do you want to continue?"),
                QMessageBox::Question,
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);

    switch (ret) {
    case QMessageBox::Yes: { // FIXME
        //            scene()->removeChildControlsOnly(scene()->currentForm());
        //            SaveManager::removeChildControlsOnly(scene()->currentForm());
        break;
    } default: {
        // Do nothing
        break;
    }
    }
}

void DesignerView::discharge()
{
    const SceneSettings* settings = DesignerSettings::instance()->sceneSettings();
    scene()->discharge();
    update();
    m_signalChooserDialog->discharge();
    m_outlineButton->setChecked(scene()->showOutlines());
    m_hideDockWidgetTitleBarsButton->setChecked(false);
    m_snappingButton->setChecked(settings->snappingEnabled);
    onZoomLevelChange("100 %");
}

QSize DesignerView::sizeHint() const
{
    return QSize(680, 680);
}

void DesignerView::onInspectorItemDoubleClick(Control* control)
{
    Q_ASSERT(scene()->currentForm());

    if (control->hasErrors()) {
        UtilityFunctions::showMessage(
                    this, tr("Oops"),
                    tr("Control has got errors, solve these problems first."));
        return;
    }

    m_signalChooserDialog->setSignalList(control->events());

    int result = m_signalChooserDialog->exec();
    if (result == QDialog::Rejected)
        return;

    const QString& methodSign = QString::fromUtf8(METHOD_DECL)
            .arg(control->id())
            .arg(methodName(m_signalChooserDialog->currentSignal()));
    const QString& methodBody = QString::fromUtf8(METHOD_BODY).arg(methodSign);
    const QString& formSign = scene()->currentForm()->id();
    const QString& formJS = formSign + ".js";
    const QString& fullPath = SaveUtils::toProjectAssetsDir(ProjectManager::dir()) + '/' + formJS;

    if (warnIfFileDoesNotExist(fullPath))
        return;

    m_qmlCodeEditorWidget->openAssets(formJS);

    QmlCodeEditorWidget::AssetsDocument* document = m_qmlCodeEditorWidget->getAssets(formJS);
    Q_ASSERT(document);

    int pos = ParserUtils::methodLine(document->document, fullPath, methodSign);
    if (pos < 0) {
        const QString& connection =
                control->id() + "." + m_signalChooserDialog->currentSignal() + ".connect(" + methodSign + ")";
        const QString& loaderSign = QString::fromUtf8(FORM_DECL).arg(formSign);
        ParserUtils::addConnection(document->document, fullPath, loaderSign, connection);
        pos = ParserUtils::addMethod(document->document, fullPath, methodBody);
        pos += 3;
    }

    m_qmlCodeEditorWidget->codeEditor()->gotoLine(pos);
}

void DesignerView::onControlDoubleClick(Control* control)
{
    m_qmlCodeEditorWidget->openDesigns(control, SaveUtils::controlMainQmlFileName());
}

void DesignerView::onAssetsFileOpen(const QString& relativePath, int line, int column)
{
    m_qmlCodeEditorWidget->openAssets(relativePath);
    m_qmlCodeEditorWidget->codeEditor()->gotoLine(line, column);
}

void DesignerView::onDesignsFileOpen(Control* control, const QString& relativePath, int line, int column)
{
    m_qmlCodeEditorWidget->openDesigns(control, relativePath);
    m_qmlCodeEditorWidget->codeEditor()->gotoLine(line, column);
}

void DesignerView::onControlDrop(Control* targetParentControl, const QString& controlRootPath, const QPointF& pos)
{
    scene()->clearSelection();
    // NOTE: Use actual Control position for scene, since createControl deals with margins
    auto newControl = ControlCreationManager::createControl(targetParentControl, controlRootPath, pos);
    if (newControl) {
        newControl->setSelected(true);
    } else {
        UtilityFunctions::showMessage(this, tr("Oops"),
                                      tr("Operation failed, control has got problems."),
                                      QMessageBox::Critical);
    }
}

void DesignerView::onControlSelectionChange(const QList<Control*>& selectedControls)
{
    scene()->clearSelection();
    for (Control* control : selectedControls)
        control->setSelected(true);
}

DesignerScene* DesignerView::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsView::scene());
}

void DesignerView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    scene()->centralize();
}

void DesignerView::contextMenuEvent(QContextMenuEvent* event)
{
    QGraphicsView::contextMenuEvent(event);

    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());

    if (selectedControls.isEmpty()) {
        m_sendBackAct->setDisabled(true);
        m_bringFrontAct->setDisabled(true);
        m_cutAct->setDisabled(true);
        m_copyAct->setDisabled(true);
        m_deleteAct->setDisabled(true);
    } else {
        m_sendBackAct->setDisabled(false);
        m_bringFrontAct->setDisabled(false);
        m_cutAct->setDisabled(false);
        m_copyAct->setDisabled(false);
        m_deleteAct->setDisabled(false);
    }
    m_menu->exec(event->globalPos());
}

void DesignerView::onUndoAction()
{
    //TODO
}

void DesignerView::onRedoAction()
{
    //TODO
}

void DesignerView::onCutAction()
{
    QList<Control*> controls(scene()->selectedControls());
    controls.removeOne(scene()->currentForm());

    const QList<Control*> copy(controls);
    for (const Control* control : copy) {
        for (Control* childControl : control->childControls())
            controls.removeOne(childControl);
    }

    QList<QPointer<Control>> controlPtrList;
    for (Control* control : controls)
        controlPtrList.append(QPointer<Control>(control));

    CopyPaste::setControls(controlPtrList, CopyPaste::Cut);
}

void DesignerView::onCopyAction()
{
    QList<Control*> controls(scene()->selectedControls());
    controls.removeOne(scene()->currentForm());

    const QList<Control*> copy(controls);
    for (const Control* control : copy) {
        for (Control* childControl : control->childControls())
            controls.removeOne(childControl);
    }

    QList<QPointer<Control>> controlPtrList;
    for (Control* control : controls)
        controlPtrList.append(QPointer<Control>(control));

    CopyPaste::setControls(controlPtrList, CopyPaste::Copy);
}

void DesignerView::onPasteAction()
{
    if (!CopyPaste::isValid())
        return;

    const CopyPaste::ActionType actionType = CopyPaste::actionType();
    const QList<QPointer<Control>>& controls = CopyPaste::controls();

    if (actionType == CopyPaste::Cut)
        CopyPaste::invalidate();

    scene()->clearSelection();

    for (const QPointer<Control>& control : controls) {
        if (control.isNull())
            continue;
        Q_ASSERT(!control->form());

        Control* newControl = nullptr;
        if (actionType == CopyPaste::Cut) {
            ControlPropertyManager::setParent(control, scene()->currentForm(),
                                              ControlPropertyManager::SaveChanges
                                              | ControlPropertyManager::UpdateRenderer);
            ControlRenderingManager::scheduleRefresh(scene()->currentForm()->uid());
        } else {
            newControl = ControlCreationManager::createControl(scene()->currentForm(),
                                                               control->dir(),
                                                               control->pos() + QPointF(5, 5));
        }

        if (newControl)
            newControl->setSelected(true);
    }
}

void DesignerView::onDeleteAction()
{ //FIXME: Do not delete if docs are open within QML Editor
    QList<Control*> selectedControls = scene()->selectedControls();
    selectedControls.removeAll(scene()->currentForm());
    ControlRemovingManager::removeControls(selectedControls);
}

void DesignerView::onSelectAllAction()
{
    auto currentForm = scene()->currentForm();
    for (auto control : currentForm->childControls())
        control->setSelected(true);
}

void DesignerView::onMoveUpAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());
    for (auto control : selectedControls)
        control->moveBy(0, - 1);
}

void DesignerView::onMoveDownAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());
    for (auto control : selectedControls)
        control->moveBy(0, 1);
}

void DesignerView::onMoveRightAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());
    for (auto control : selectedControls)
        control->moveBy(1, 0);
}

void DesignerView::onMoveLeftAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());
    for (auto control : selectedControls)
        control->moveBy(- 1, 0);
}

void DesignerView::onSendBackAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());
    for (auto control : selectedControls) {
        ControlPropertyManager::setZ(control, scene()->currentForm()->lowerZValue() - 1,
                                     ControlPropertyManager::SaveChanges
                                     | ControlPropertyManager::UpdateRenderer);
    }
}

void DesignerView::onBringFrontAction()
{
    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());
    for (auto control : selectedControls) {
        ControlPropertyManager::setZ(control, scene()->currentForm()->higherZValue() + 1,
                                     ControlPropertyManager::SaveChanges
                                     | ControlPropertyManager::UpdateRenderer);
    }
}
