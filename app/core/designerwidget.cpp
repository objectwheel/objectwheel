#include <designerwidget.h>
#include <designerview.h>
#include <designerscene.h>
#include <controlcreationmanager.h>
#include <utilsicons.h>
#include <controlpreviewingmanager.h>
#include <saveutils.h>
#include <transparentstyle.h>
#include <signalchooserdialog.h>
#include <utilityfunctions.h>
#include <qmlcodeeditorwidget.h>
#include <parserutils.h>
#include <projectmanager.h>
#include <filemanager.h>
#include <qmlcodeeditor.h>
#include <qmlcodedocument.h>

#include <QDir>
#include <QToolBar>
#include <QToolButton>
#include <QPainter>
#include <QVBoxLayout>
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

QString methodName(const QString& signal)
{
    QString method(signal);
    method.replace(0, 1, method[0].toUpper());
    return method.prepend("on");
}

QString findText(qreal ratio)
{
    if (ratio == 0.1)
        return "10 %";
    else if (ratio == 0.25)
        return "25 %";
    else if (ratio == 0.50)
        return "50 %";
    else if (ratio == 0.75)
        return "75 %";
    else if (ratio == 0.90)
        return "90 %";
    else if (ratio == 1.0)
        return "100 %";
    else if (ratio == 1.25)
        return "125 %";
    else if (ratio == 1.50)
        return "150 %";
    else if (ratio == 1.75)
        return "175 %";
    else if (ratio == 2.0)
        return "200 %";
    else if (ratio == 3.0)
        return "300 %";
    else if (ratio == 5.0)
        return "500 %";
    else if (ratio == 10.0)
        return "1000 %";
    else
        return "100 %";
}

qreal roundRatio(qreal ratio)
{
    if (ratio < 0.1)
        return 0.1;
    else if (ratio >= 0.1 && ratio < 0.25)
        return 0.1;
    else if (ratio >= 0.25 && ratio < 0.5)
        return 0.25;
    else if (ratio >= 0.5 && ratio < 0.75)
        return 0.5;
    else if (ratio >= 0.75 && ratio < 0.9)
        return 0.75;
    else if (ratio >= 0.9 && ratio < 1.0)
        return 0.9;
    else
        return 1.0;
}

qreal findRatio(const QString& text)
{
    if (text == "10 %")
        return 0.1;
    else if (text == "25 %")
        return 0.25;
    else if (text == "50 %")
        return 0.50;
    else if (text == "75 %")
        return 0.75;
    else if (text == "90 %")
        return 0.90;
    else if (text == "100 %")
        return 1.0;
    else if (text == "125 %")
        return 1.25;
    else if (text == "150 %")
        return 1.50;
    else if (text == "175 %")
        return 1.75;
    else if (text == "200 %")
        return 2.0;
    else if (text == "300 %")
        return 3.0;
    else if (text == "500 %")
        return 5.0;
    else if (text == "1000 %")
        return 10.0;
    else
        return 1.0;
}

bool warnIfFileDoesNotExist(const QString& filePath)
{
    if (!exists(filePath)) {
        return QMessageBox::warning(
                    0,
                    QObject::tr("Oops"),
                    QObject::tr("File %1 does not exist.").arg(fname(filePath)));
    }
    return false;
}
}

DesignerWidget::DesignerWidget(QmlCodeEditorWidget* qmlCodeEditorWidget, QWidget *parent) : QWidget(parent)
  , m_lastScale(1.0)
  , m_signalChooserDialog(new SignalChooserDialog(this))
  , m_qmlCodeEditorWidget(qmlCodeEditorWidget)
  , m_layout(new QVBoxLayout(this))
  , m_designerScene(new DesignerScene(this))
  , m_designerView(new DesignerView(m_designerScene))
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
{
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_designerView);

    m_designerView->setRenderHint(QPainter::Antialiasing);
    m_designerView->setRubberBandSelectionMode(Qt::IntersectsItemShape);
    m_designerView->setDragMode(QGraphicsView::RubberBandDrag);
    m_designerView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_designerView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    m_designerView->setFrameShape(QFrame::NoFrame);
    m_designerView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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

    m_outlineButton->setCheckable(true);
    m_outlineButton->setChecked(m_designerScene->showOutlines());
    m_hideDockWidgetTitleBarsButton->setCheckable(true);
    m_hideDockWidgetTitleBarsButton->setChecked(false);
    m_snappingButton->setCheckable(true);
    m_snappingButton->setChecked(m_designerScene->snapping());

    m_refreshButton->setCursor(Qt::PointingHandCursor);
    m_clearButton->setCursor(Qt::PointingHandCursor);
    m_undoButton->setCursor(Qt::PointingHandCursor);
    m_redoButton->setCursor(Qt::PointingHandCursor);
    m_snappingButton->setCursor(Qt::PointingHandCursor);
    m_outlineButton->setCursor(Qt::PointingHandCursor);
    m_hideDockWidgetTitleBarsButton->setCursor(Qt::PointingHandCursor);
    m_fitButton->setCursor(Qt::PointingHandCursor);
    m_zoomlLevelCombobox->setCursor(Qt::PointingHandCursor);

    m_refreshButton->setToolTip("Refresh control previews on the Dashboard.");
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

    connect(m_snappingButton, &QToolButton::toggled, this, &DesignerWidget::onSnappingButtonClick);
    connect(m_outlineButton, &QToolButton::toggled, this, &DesignerWidget::onOutlineButtonClick);
    connect(m_hideDockWidgetTitleBarsButton, &QToolButton::toggled, this, &DesignerWidget::hideDockWidgetTitleBars);
    connect(m_zoomlLevelCombobox, &QComboBox::currentTextChanged, this, &DesignerWidget::onZoomLevelChange);
    connect(m_fitButton, &QToolButton::clicked, this, &DesignerWidget::onFitButtonClick);
    connect(m_refreshButton, &QToolButton::clicked, this, &DesignerWidget::onRefreshButtonClick);
    connect(m_clearButton, &QToolButton::clicked, this, &DesignerWidget::onClearButtonClick);

    TransparentStyle::attach(m_toolBar);

    m_undoButton->setFixedHeight(20);
    m_redoButton->setFixedHeight(20);
    m_clearButton->setFixedHeight(20);
    m_refreshButton->setFixedHeight(20);
    m_snappingButton->setFixedHeight(20);
    m_fitButton->setFixedHeight(20);
    m_outlineButton->setFixedHeight(20);
    m_hideDockWidgetTitleBarsButton->setFixedHeight(20);
    m_zoomlLevelCombobox->setFixedHeight(20);

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
}

void DesignerWidget::scaleScene(qreal ratio)
{
    m_designerView->scale((1.0 / m_lastScale) * ratio, (1.0 / m_lastScale) * ratio);
    m_lastScale = ratio;
}

DesignerScene* DesignerWidget::designerScene() const
{
    return m_designerScene;
}

void DesignerWidget::onSnappingButtonClick(bool value)
{
    m_designerScene->setSnapping(value);
}

void DesignerWidget::onOutlineButtonClick(bool value)
{
    m_designerScene->setShowOutlines(value);
}

void DesignerWidget::onFitButtonClick()
{
    static auto ratios = { 0.1, 0.25, 0.5, 0.75, 0.9, 1.0, 1.25, 1.50, 1.75, 2.0, 3.0, 5.0, 10.0 };
    auto diff = qMin(m_designerView->width() / m_designerScene->width(), m_designerView->height() / m_designerScene->height());
    for (auto ratio : ratios) {
        if (roundRatio(diff) == ratio)
            m_zoomlLevelCombobox->setCurrentText(findText(ratio));
    }
}

void DesignerWidget::onUndoButtonClick()
{
    // TODO:
}

void DesignerWidget::onRedoButtonClick()
{
    // TODO:
}

void DesignerWidget::onZoomLevelChange(const QString& text)
{
    qreal ratio = findRatio(text);
    scaleScene(ratio);
}

void DesignerWidget::onRefreshButtonClick()
{
    ControlPreviewingManager::scheduleRefresh(m_designerScene->currentForm()->uid());
}

void DesignerWidget::onClearButtonClick()
{
    if (!m_designerScene->currentForm())
        return;

    QMessageBox msgBox;
    msgBox.setText("<b>This will remove current scene's content.</b>");
    msgBox.setInformativeText("Do you want to continue?");
    msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Question);
    const int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Yes: { // FIXME
        //            m_designerScene->removeChildControlsOnly(m_designerScene->currentForm());
        //            SaveManager::removeChildControlsOnly(m_designerScene->currentForm());
        break;
    } default: {
        // Do nothing
        break;
    }
    }
}

void DesignerWidget::discharge()
{
    m_designerScene->discharge();
    m_designerView->discharge();
    m_signalChooserDialog->discharge();
    m_outlineButton->setChecked(m_designerScene->showOutlines());
    m_hideDockWidgetTitleBarsButton->setChecked(false);
    m_snappingButton->setChecked(m_designerScene->snapping());
    onZoomLevelChange("100 %");
}

QSize DesignerWidget::sizeHint() const
{
    return QSize(680, 680);
}

void DesignerWidget::onControlDoubleClick(Control* control)
{
    Q_ASSERT(m_designerScene->currentForm());

    if (control->hasErrors())
        return (void) QMessageBox::warning(this, tr("Oops"), tr("Control has errors, fix them first."));

    m_signalChooserDialog->setSignalList(control->events());

    int result = m_signalChooserDialog->exec();
    if (result == QDialog::Rejected)
        return;

    const QString& methodSign = QString::fromUtf8(METHOD_DECL)
            .arg(control->id())
            .arg(methodName(m_signalChooserDialog->currentSignal()));
    const QString& methodBody = QString::fromUtf8(METHOD_BODY).arg(methodSign);
    const QString& formSign = m_designerScene->currentForm()->main()
            ? "application"
            : m_designerScene->currentForm()->id();
    const QString& formJS = formSign + ".js";
    const QString& fullPath = SaveUtils::toGlobalDir(ProjectManager::dir()) + separator() + formJS;

    if (warnIfFileDoesNotExist(fullPath))
        return;

    m_qmlCodeEditorWidget->openGlobal(formJS);

    QmlCodeEditorWidget::GlobalDocument* document = m_qmlCodeEditorWidget->getGlobal(formJS);
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

void DesignerWidget::onInspectorItemDoubleClick(Control* control)
{
    m_qmlCodeEditorWidget->openInternal(control, "main.qml");
}

void DesignerWidget::onGlobalFileOpen(const QString& relativePath, int line, int column)
{
    m_qmlCodeEditorWidget->openGlobal(relativePath);
    m_qmlCodeEditorWidget->codeEditor()->gotoLine(line, column);
}

void DesignerWidget::onInternalFileOpen(Control* control, const QString& relativePath, int line, int column)
{
    m_qmlCodeEditorWidget->openInternal(control, relativePath);
    m_qmlCodeEditorWidget->codeEditor()->gotoLine(line, column);
}

void DesignerWidget::onControlDrop(Control* targetParentControl, const QString& controlRootPath, const QPointF& pos)
{
    m_designerScene->clearSelection();
    // NOTE: Use actual Control position for scene, since createControl deals with margins
    auto newControl = ControlCreationManager::createControl(targetParentControl, controlRootPath, pos);
    if (newControl)
        newControl->setSelected(true);
    else
        QMessageBox::critical(this, tr("Oops"), tr("Operation failed, control has problems."));
}

void DesignerWidget::onControlSelectionChange(const QList<Control*>& selectedControls)
{
    m_designerScene->clearSelection();
    for (Control* control : selectedControls)
        control->setSelected(true);
}