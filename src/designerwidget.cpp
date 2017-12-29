#include <designerwidget.h>
#include <formscene.h>
#include <formview.h>
#include <controlscene.h>
#include <controlview.h>
#include <qmleditorview.h>
#include <control.h>
#include <fit.h>
#include <css.h>
#include <loadingindicator.h>
#include <savebackend.h>
#include <outputwidget.h>
#include <controlwatcher.h>
#include <savetransaction.h>
#include <mainwindow.h>

#include <QWidget>
#include <QList>
#include <QVBoxLayout>
#include <QTimer>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QToolButton>
#include <QMenu>
#include <QComboBox>
#include <QMessageBox>
#include <QSplitter>
#include <QQuickStyle>
//#include <QOpenGLWidget>

#define INTERVAL_ERROR_CHECK (1000)
#define cW (ControlWatcher::instance())

// TODO: Prevent NoSkin option for main form
// FIXME: Bugs about Control GUI Editor

static qreal roundRatio(qreal ratio)
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

static qreal findRatio(const QString& text)
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

static QString findText(qreal ratio)
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

static void scaleScene(qreal ratio)
{
    if (DesignerWidget::_mode == FormGui) {
        formView.scale((1.0 / lastScaleOfWv) * ratio, (1.0 / lastScaleOfWv) * ratio);
        lastScaleOfWv = ratio;
    } else {
        controlView.scale((1.0 / lastScaleOfCv) * ratio, (1.0 / lastScaleOfCv) * ratio);
        lastScaleOfCv = ratio;
    }
}

void DesignerWidgetPrivate::handleIndicatorChanges()
{
    loadingIndicator.setRunning(SaveBackend::parserWorking() || PreviewBackend::working());
}

void DesignerWidgetPrivate::handleSnappingClick(bool value)
{
    if (DesignerWidget::_mode == FormGui)
        formScene.setSnapping(value);
    else if (DesignerWidget::_mode == ControlGui)
        controlScene.setSnapping(value);
}

void DesignerWidgetPrivate::handleShowOutlineClick(bool value)
{
    if (DesignerWidget::_mode == FormGui ||
        DesignerWidget::_mode == ControlGui) {
        formScene.setShowOutlines(value);
        controlScene.setShowOutlines(value);
    }
}

void DesignerWidgetPrivate::handleFitInSceneClick()
{
    auto ratios = { 0.1, 0.25, 0.5, 0.75, 0.9, 1.0, 1.25, 1.50, 1.75, 2.0, 3.0, 5.0, 10.0 };
    auto diff = DesignerWidget::_mode == FormGui ?
                    qMin(formView.width() / formScene.width(),
                         formView.height() / formScene.height()) :
                    qMin(controlView.width() / controlScene.width(),
                         controlView.height() / controlScene.height());;
    for (auto ratio : ratios)
        if (roundRatio(diff) == ratio)
            zoomlLevelCombobox.setCurrentText(findText(ratio));
}

void DesignerWidgetPrivate::handleThemeChange(const QString& text)
{
//    qmlClearTypeRegistrations();
//    QQuickStyle::setStyle(text);
//    qmlRegisterType();
//    handleRefreshPreviewClick();
//    formScene.mainForm()->refresh();
//    for (auto control : formScene.mainForm()->childControls())
//        control->refresh();
//    controlScene.mainControl()->refresh();
//    for (auto control : controlScene.mainControl()->childControls())
//        control->refresh();
}

void DesignerWidgetPrivate::handleZoomLevelChange(const QString& text)
{
    qreal ratio = findRatio(text);
    scaleScene(ratio);
}

void DesignerWidgetPrivate::handlePhonePortraitButtonClick()
{
    auto form = DesignerWidget::formScene()->mainForm();
    form->setSkin(Skin::PhonePortrait);
    SaveBackend::setProperty(form, TAG_SKIN, Skin::PhonePortrait);
    phonePortraitButton.setDisabled(true);
    phoneLandscapeButton.setChecked(false);
    desktopSkinButton.setChecked(false);
    noSkinButton.setChecked(false);
    phoneLandscapeButton.setEnabled(true);
    desktopSkinButton.setEnabled(true);
    noSkinButton.setEnabled(true);
    if (formScene.mainControl())
        formScene.mainControl()->centralize();
}

void DesignerWidgetPrivate::handlePhoneLandscapeButtonClick()
{
    auto form = DesignerWidget::formScene()->mainForm();
    form->setSkin(Skin::PhoneLandscape);
    SaveBackend::setProperty(form, TAG_SKIN, Skin::PhoneLandscape);
    phoneLandscapeButton.setDisabled(true);
    phonePortraitButton.setChecked(false);
    desktopSkinButton.setChecked(false);
    noSkinButton.setChecked(false);
    phonePortraitButton.setEnabled(true);
    desktopSkinButton.setEnabled(true);
    noSkinButton.setEnabled(true);
    if (formScene.mainControl())
        formScene.mainControl()->centralize();
}

void DesignerWidgetPrivate::handleDesktopSkinButtonClick()
{
    auto form = DesignerWidget::formScene()->mainForm();
    form->setSkin(Skin::Desktop);
    SaveBackend::setProperty(form, TAG_SKIN, Skin::Desktop);
    desktopSkinButton.setDisabled(true);
    phoneLandscapeButton.setChecked(false);
    phonePortraitButton.setChecked(false);
    noSkinButton.setChecked(false);
    phonePortraitButton.setEnabled(true);
    phoneLandscapeButton.setEnabled(true);
    noSkinButton.setEnabled(true);
    if (formScene.mainControl())
        formScene.mainControl()->centralize();
}

void DesignerWidgetPrivate::handleNoSkinButtonClick()
{
    auto form = DesignerWidget::formScene()->mainForm();
    form->setSkin(Skin::NoSkin);
    SaveBackend::setProperty(form, TAG_SKIN, Skin::NoSkin);
    noSkinButton.setDisabled(true);
    phoneLandscapeButton.setChecked(false);
    desktopSkinButton.setChecked(false);
    phonePortraitButton.setChecked(false);
    phonePortraitButton.setEnabled(true);
    phoneLandscapeButton.setEnabled(true);
    desktopSkinButton.setEnabled(true);
    if (formScene.mainControl())
        formScene.mainControl()->centralize();
}

void DesignerWidgetPrivate::handleRefreshPreviewClick()
{
    if (DesignerWidget::_mode == FormGui) {
        formScene.mainForm()->refresh();
        for (auto control : formScene.mainForm()->childControls())
            control->refresh();
    } else {
        controlScene.mainControl()->refresh();
        for (auto control : controlScene.mainControl()->childControls())
            control->refresh();
    }
}

void DesignerWidgetPrivate::handleClearControls()
{
    auto scene = parent->currentScene();
    if (!scene || !scene->mainControl())
        return;

    QMessageBox msgBox;
    msgBox.setText("<b>This will remove current scene's content.</b>");
    msgBox.setInformativeText("Do you want to continue?");
    msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Question);
    const int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes: {
            scene->removeChildControlsOnly(scene->mainControl());
            SaveBackend::removeChildControlsOnly(scene->mainControl());
            break;
        } default: {
            // Do nothing
            break;
        }
    }
}

void DesignerWidgetPrivate::handleEditorModeButtonClick()
{
    DesignerWidget::setMode(CodeEdit);
}

void DesignerWidgetPrivate::handleCGuiModeButtonClick()
{
    DesignerWidget::setMode(ControlGui);
}

void DesignerWidgetPrivate::handleWGuiModeButtonClick()
{
    DesignerWidget::setMode(FormGui);
}
void DesignerWidgetPrivate::handlePlayButtonClick()
{
    ExecError error = SaveBackend::execProject();

    QMessageBox box;
    box.setText("<b>Some went wrong.</b>");
    box.setStandardButtons(QMessageBox::Ok);
    box.setDefaultButton(QMessageBox::Ok);
    box.setIcon(QMessageBox::Warning);
    switch (error.type) {
        case CommonError:
            box.setInformativeText("Database corrupted, change your application skin. "
                                   "If it doesn't work, contact to support for further help.");
            box.exec();
            break;

        case ChildIsWindowError:
            box.setInformativeText("Child controls can not be a 'Window' (or derived) type."
                                   " Only forms could be 'Window' type.");
            box.exec();
            break;

        case MasterIsNonGui:
            box.setInformativeText("Master controls can not be a non-ui control (such as Timer or QtObject).");
            box.exec();
            break;

        case FormIsNonGui:
            box.setInformativeText("Forms can not be a non-ui control (such as Timer or QtObject)."
                                   "Check your forms and make sure they are some 'Window' or 'Item' derived type.");
            box.exec();
            break;

        case MainFormIsntWindowError:
            box.setInformativeText("Main form has to be a 'Window' derived type. "
                                   "Please change its type to a 'Window' derived class.");
            box.exec();
            break;

        case MultipleWindowsForMobileError:
            box.setInformativeText("Mobile applications can not contain multiple windows. "
                                   "Please either change the type of secondary windows' type to a non 'Window' derived class, "
                                   "or change your application skin to something else (Desktop for instance) by changing the skin of main form.");
            box.exec();
            break;

        case NoMainForm:
            box.setInformativeText("There is no main application window. Probably database has corrupted, "
                                   "please contact to support, or start a new project over.");
            box.exec();
            break;

        case CodeError: {
            box.setInformativeText(QString("Following control has some errors: <b>%1</b>").
              arg(error.id));
            QString detailedText;
            for (auto err : error.errors)
                detailedText += QString("Line %1, column %2: %3").
                  arg(err.line()).arg(err.column()).arg(err.description());
            box.setDetailedText(detailedText);
            box.exec();
            break;
        }

        default:
            break;
    }
}

void DesignerWidgetPrivate::handleBuildButtonClick()
{
    WindowManager::instance()->show(WindowManager::Builds);
}

void DesignerWidgetPrivate::handleModeChange()
{
    if (DesignerWidget::_mode == FormGui) {
        wGuiModeButton.setChecked(true);
        wGuiModeButton.setDisabled(true);
        cGuiModeButton.setChecked(false);
        editorModeButton.setChecked(false);
        cGuiModeButton.setEnabled(true);
        editorModeButton.setEnabled(true);

        auto form = DesignerWidget::formScene()->mainForm();
        if (form) {
            if (form->skin() == Skin::Desktop) {
                desktopSkinButton.setChecked(true);
                handleDesktopSkinButtonClick();
            } else if (form->skin() == Skin::NoSkin) {
                noSkinButton.setChecked(true);
                handleNoSkinButtonClick();
            } else if (form->skin() == Skin::PhonePortrait) {
                phonePortraitButton.setChecked(true);
                handlePhonePortraitButtonClick();
            } else {
                phoneLandscapeButton.setChecked(true);
                handlePhoneLandscapeButtonClick();
            }
        }

        snappingButton.setChecked(formScene.snapping());
        snappingButton.setEnabled(true);
        refreshPreviewButton.setEnabled(true);
        clearFormButton.setEnabled(true);
        showOutlineButton.setChecked(formScene.showOutlines());
        showOutlineButton.setEnabled(true);
        fitInSceneButton.setEnabled(true);
        layItVertButton.setEnabled(true);
        layItHorzButton.setEnabled(true);
        layItGridButton.setEnabled(true);
        breakLayoutButton.setEnabled(true);
        zoomlLevelCombobox.setCurrentText(findText(lastScaleOfWv));
        auto sizes = splitter.sizes();
        QSize size;
        if (controlView.isVisible())
            size = controlView.size();
        else
            size = qmlEditorView.size();
        sizes[splitter.indexOf(&formView)] = size.height();
        controlView.hide();
        qmlEditorView.hide();
        formView.show();
        toolbar.show();
        splitter.setSizes(sizes);
        parent->_currentScene = &formScene;
    } else if (DesignerWidget::_mode == ControlGui) {
        cGuiModeButton.setChecked(true);
        cGuiModeButton.setDisabled(true);
        editorModeButton.setChecked(false);
        wGuiModeButton.setChecked(false);
        editorModeButton.setEnabled(true);
        wGuiModeButton.setEnabled(true);
        noSkinButton.setChecked(true);
        phoneLandscapeButton.setChecked(false);
        phonePortraitButton.setChecked(false);
        desktopSkinButton.setChecked(false);
        noSkinButton.setDisabled(true);
        phonePortraitButton.setDisabled(true);
        phoneLandscapeButton.setDisabled(true);
        desktopSkinButton.setDisabled(true);
        if (controlScene.mainControl())
            controlScene.mainControl()->centralize();
        snappingButton.setChecked(controlScene.snapping());
        snappingButton.setEnabled(true);
        refreshPreviewButton.setEnabled(true);
        clearFormButton.setEnabled(true);
        showOutlineButton.setChecked(controlScene.showOutlines());
        showOutlineButton.setEnabled(true);
        fitInSceneButton.setEnabled(true);
        layItVertButton.setEnabled(true);
        layItHorzButton.setEnabled(true);
        layItGridButton.setEnabled(true);
        breakLayoutButton.setEnabled(true);
        zoomlLevelCombobox.setCurrentText(findText(lastScaleOfCv));
        auto sizes = splitter.sizes();
        QSize size;
        if (formView.isVisible())
            size = formView.size();
        else
            size = qmlEditorView.size();
        sizes[splitter.indexOf(&controlView)] = size.height();
        formView.hide();
        qmlEditorView.hide();
        controlView.show();
        toolbar.show();
        splitter.setSizes(sizes);
        parent->_currentScene = &controlScene;
    } else {
        editorModeButton.setChecked(true);
        editorModeButton.setDisabled(true);
        cGuiModeButton.setChecked(false);
        wGuiModeButton.setChecked(false);
        cGuiModeButton.setEnabled(true);
        wGuiModeButton.setEnabled(true);
        noSkinButton.setChecked(false);
        phoneLandscapeButton.setChecked(false);
        phonePortraitButton.setChecked(false);
        desktopSkinButton.setChecked(false);
        noSkinButton.setDisabled(true);
        phonePortraitButton.setDisabled(true);
        phoneLandscapeButton.setDisabled(true);
        desktopSkinButton.setDisabled(true);
        snappingButton.setChecked(false);
        snappingButton.setDisabled(true);
        showOutlineButton.setChecked(false);
        showOutlineButton.setDisabled(true);
        refreshPreviewButton.setDisabled(true);
        clearFormButton.setDisabled(true);
        fitInSceneButton.setDisabled(true);
        layItVertButton.setDisabled(true);
        layItHorzButton.setDisabled(true);
        layItGridButton.setDisabled(true);
        breakLayoutButton.setDisabled(true);
        auto sizes = splitter.sizes();
        QSize size;
        if (formView.isVisible())
            size = formView.size();
        else
            size = controlView.size();
        sizes[splitter.indexOf(&qmlEditorView)] = size.height();
        toolbar.hide();
        formView.hide();
        controlView.hide();
        qmlEditorView.show();
        splitter.setSizes(sizes);
    }
}

DesignMode DesignerWidget::_mode = FormGui;
ControlScene* DesignerWidget::_currentScene = nullptr;

DesignerWidget::DesignerWidget(QObject *parent)
    : QObject(parent)
    , _formView(&formScene)
    , _controlView(&controlScene)
    , _lastScaleOfWv(1.0)
    , _lastScaleOfCv(1.0)
{

    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Plain);

    hlayout.setContentsMargins(0, 0, 0, 0);
    hlayout.setSpacing(0);
    hlayout.addWidget(&toolbar2);
    hlayout.addLayout(&vlayout);

    vlayout.setContentsMargins(0, 0, 0, 0);
    vlayout.setSpacing(0);
    vlayout.addWidget(&splitter);

    splitter.setStyleSheet("QSplitter{background: #e0e4e7;}");
    splitter.setOrientation(Qt::Vertical);
    splitter.addWidget(&toolbar);
    splitter.addWidget(&formView);
    splitter.addWidget(&controlView);
    splitter.addWidget(&qmlEditorView);
    splitter.addWidget(&outputWidget);
    splitter.setCollapsible(0, false);
    splitter.setCollapsible(1, false);
    splitter.setCollapsible(2, false);
    splitter.setCollapsible(3, false);
    splitter.setCollapsible(4, false);
    splitter.handle(0)->setDisabled(true);
    splitter.handle(1)->setDisabled(true);
    splitter.handle(2)->setDisabled(true);
    splitter.handle(3)->setDisabled(true);
    splitter.setHandleWidth(0);

    outputWidget.setSplitter(&splitter);
    outputWidget.setSplitterHandle(splitter.handle(4));
    connect(&splitter, SIGNAL(splitterMoved(int,int)),
      &outputWidget, SLOT(updateLastHeight()));
    qmlEditorView.setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);

//    formView.setViewport(new QOpenGLWidget);
    formView.setRenderHint(QPainter::Antialiasing);
    formView.setRubberBandSelectionMode(Qt::IntersectsItemShape);
    formView.setDragMode(QGraphicsView::RubberBandDrag);
    formView.setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    formView.setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    formView.setBackgroundBrush(QColor("#e0e4e7"));
    formView.setFrameShape(QFrame::NoFrame);
    formView.setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);

//    controlView.setViewport(new QOpenGLWidget);
    controlView.setRenderHint(QPainter::Antialiasing);
    controlView.setRubberBandSelectionMode(Qt::IntersectsItemShape);
    controlView.setDragMode(QGraphicsView::RubberBandDrag);
    controlView.setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    controlView.setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    controlView.setBackgroundBrush(QColor("#e0e4e7"));
    controlView.setFrameShape(QFrame::NoFrame);
    controlView.setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);

    // Toolbar settings
    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Preferred);

    #if defined(Q_OS_WIN)
    zoomlLevelCombobox.setFixedHeight(fit::fx(18));
    #endif
    zoomlLevelCombobox.addItem("10 %");
    zoomlLevelCombobox.addItem("25 %");
    zoomlLevelCombobox.addItem("50 %");
    zoomlLevelCombobox.addItem("75 %");
    zoomlLevelCombobox.addItem("90 %");
    zoomlLevelCombobox.addItem("100 %");
    zoomlLevelCombobox.addItem("125 %");
    zoomlLevelCombobox.addItem("150 %");
    zoomlLevelCombobox.addItem("175 %");
    zoomlLevelCombobox.addItem("200 %");
    zoomlLevelCombobox.addItem("300 %");
    zoomlLevelCombobox.addItem("500 %");
    zoomlLevelCombobox.addItem("1000 %");
    zoomlLevelCombobox.setCurrentIndex(5);

    #if defined(Q_OS_WIN)
    themeCombobox.setFixedHeight(fit::fx(18));
    #endif
    for (auto theme : QQuickStyle::availableStyles())
        themeCombobox.addItem(theme);

    showOutlineButton.setCheckable(true);
    showOutlineButton.setChecked(Control::showOutline());
    snappingButton.setCheckable(true);
    snappingButton.setChecked(formScene.snapping());

    phonePortraitButton.setCheckable(true);
    phonePortraitButton.setChecked(true);
    phonePortraitButton.setDisabled(true);
    phoneLandscapeButton.setCheckable(true);
    desktopSkinButton.setCheckable(true);
    noSkinButton.setCheckable(true);

    refreshPreviewButton.setCursor(Qt::PointingHandCursor);
    clearFormButton.setCursor(Qt::PointingHandCursor);
    undoButton.setCursor(Qt::PointingHandCursor);
    redoButton.setCursor(Qt::PointingHandCursor);
    phonePortraitButton.setCursor(Qt::PointingHandCursor);
    phoneLandscapeButton.setCursor(Qt::PointingHandCursor);
    desktopSkinButton.setCursor(Qt::PointingHandCursor);
    noSkinButton.setCursor(Qt::PointingHandCursor);
    snappingButton.setCursor(Qt::PointingHandCursor);
    showOutlineButton.setCursor(Qt::PointingHandCursor);
    fitInSceneButton.setCursor(Qt::PointingHandCursor);
    zoomlLevelCombobox.setCursor(Qt::PointingHandCursor);
    themeCombobox.setCursor(Qt::PointingHandCursor);
    layItVertButton.setCursor(Qt::PointingHandCursor);
    layItHorzButton.setCursor(Qt::PointingHandCursor);
    layItGridButton.setCursor(Qt::PointingHandCursor);
    breakLayoutButton.setCursor(Qt::PointingHandCursor);

    refreshPreviewButton.setToolTip("Refresh control previews on the Dashboard.");
    clearFormButton.setToolTip("Clear controls on the Dashboard.");
    undoButton.setToolTip("Undo action.");
    redoButton.setToolTip("Redo action.");
    phonePortraitButton.setToolTip("Skin: Phone portrait.");
    phoneLandscapeButton.setToolTip("Skin: Phone landscape.");
    desktopSkinButton.setToolTip("Skin: Desktop.");
    noSkinButton.setToolTip("No skin (free).");
    snappingButton.setToolTip("Enable snapping to help aligning of controls to each others.");
    showOutlineButton.setToolTip("Show outline frame for controls.");
    fitInSceneButton.setToolTip("Fit scene into the Dashboard.");
    zoomlLevelCombobox.setToolTip("Change zoom level.");
    themeCombobox.setToolTip("Change Theme. Themes are only available for Quick Controls 2.0 and above.");
    loadingIndicator.setToolTip("Background operations indicator.");
    layItVertButton.setToolTip("Lay out selected controls vertically.");
    layItHorzButton.setToolTip("Lay out selected controls horizontally.");
    layItGridButton.setToolTip("Lay out selected controls as grid.");
    breakLayoutButton.setToolTip("Break layouts for selected controls.");

    refreshPreviewButton.setIcon(QIcon(":/resources/images/refresh.png"));
    clearFormButton.setIcon(QIcon(":/resources/images/clean.png"));
    undoButton.setIcon(QIcon(":/resources/images/undo.png"));
    redoButton.setIcon(QIcon(":/resources/images/redo.png"));
    phonePortraitButton.setIcon(QIcon(":/resources/images/portrait.png"));
    phoneLandscapeButton.setIcon(QIcon(":/resources/images/landscape.png"));
    desktopSkinButton.setIcon(QIcon(":/resources/images/desktop.png"));
    noSkinButton.setIcon(QIcon(":/resources/images/free.png"));
    snappingButton.setIcon(QIcon(":/resources/images/snap.png"));
    showOutlineButton.setIcon(QIcon(":/resources/images/outline.png"));
    fitInSceneButton.setIcon(QIcon(":/resources/images/fit.png"));
    layItVertButton.setIcon(QIcon(":/resources/images/vert.png"));
    layItHorzButton.setIcon(QIcon(":/resources/images/hort.png"));
    layItGridButton.setIcon(QIcon(":/resources/images/grid.png"));
    breakLayoutButton.setIcon(QIcon(":/resources/images/break.png"));
    loadingIndicator.setImage(QImage(":/resources/images/refresh.png"));
    loadingIndicator.setRunning(false);

    connect(&snappingButton, SIGNAL(toggled(bool)),
      SLOT(handleSnappingClick(bool)));
    connect(&showOutlineButton, SIGNAL(toggled(bool)),
      SLOT(handleShowOutlineClick(bool)));
    connect(&zoomlLevelCombobox, SIGNAL(currentTextChanged(QString)),
      SLOT(handleZoomLevelChange(QString)));
//    connect(&themeCombobox, SIGNAL(currentTextChanged(QString)),
//      SLOT(handleThemeChange(QString)));
    connect(&fitInSceneButton, SIGNAL(clicked(bool)),
      SLOT(handleFitInSceneClick()));
    connect(&refreshPreviewButton, SIGNAL(clicked(bool)),
      SLOT(handleRefreshPreviewClick()));
    connect(&clearFormButton, SIGNAL(clicked(bool)),
      SLOT(handleClearControls()));
    connect(&phonePortraitButton, SIGNAL(clicked(bool)),
      SLOT(handlePhonePortraitButtonClick()));
    connect(&phoneLandscapeButton, SIGNAL(clicked(bool)),
      SLOT(handlePhoneLandscapeButtonClick()));
    connect(&desktopSkinButton, SIGNAL(clicked(bool)),
      SLOT(handleDesktopSkinButtonClick()));
    connect(&noSkinButton, SIGNAL(clicked(bool)),
      SLOT(handleNoSkinButtonClick()));

    toolbar.setStyleSheet(CSS::DesignerToolbar);
    toolbar.setFixedHeight(fit::fx(21));
    toolbar.setIconSize(QSize(fit::fx(14), fit::fx(14)));
    toolbar.addWidget(&undoButton);
    toolbar.addWidget(&redoButton);
    toolbar.addSeparator();
    toolbar.addWidget(&refreshPreviewButton);
    toolbar.addWidget(&clearFormButton);
    toolbar.addSeparator();
    toolbar.addWidget(&phonePortraitButton);
    toolbar.addWidget(&phoneLandscapeButton);
    toolbar.addWidget(&desktopSkinButton);
    toolbar.addWidget(&noSkinButton);
    toolbar.addSeparator();
    toolbar.addWidget(&snappingButton);
    toolbar.addWidget(&showOutlineButton);
    toolbar.addWidget(&fitInSceneButton);
    toolbar.addWidget(&themeCombobox);
    toolbar.addWidget(&zoomlLevelCombobox);
    toolbar.addSeparator();
    toolbar.addWidget(&loadingIndicator);
    toolbar.addWidget(spacer);
    toolbar.addWidget(&layItVertButton);
    toolbar.addWidget(&layItHorzButton);
    toolbar.addWidget(&layItGridButton);
    toolbar.addWidget(&breakLayoutButton);

    // toolbar2 settings
    QWidget* spacer_2 = new QWidget;
    spacer_2->setSizePolicy(QSizePolicy::Preferred,
      QSizePolicy::Expanding);

    editorModeButton.setCheckable(true);
    wGuiModeButton.setCheckable(true);
    cGuiModeButton.setCheckable(true);
    wGuiModeButton.setChecked(true);
    wGuiModeButton.setDisabled(true);

    editorModeButton.setCursor(Qt::PointingHandCursor);
    wGuiModeButton.setCursor(Qt::PointingHandCursor);
    cGuiModeButton.setCursor(Qt::PointingHandCursor);
    playButton.setCursor(Qt::PointingHandCursor);
    buildButton.setCursor(Qt::PointingHandCursor);

    editorModeButton.setToolTip("Switch to Qml Editor.");
    cGuiModeButton.setToolTip("Switch to Tool Editor.");
    wGuiModeButton.setToolTip("Switch to Form Editor.");
    playButton.setToolTip("Run project.");
    buildButton.setToolTip("Get builds.");

    editorModeButton.setIcon(QIcon(":/resources/images/text.png"));
    cGuiModeButton.setIcon(QIcon(":/resources/images/gui.png"));
    wGuiModeButton.setIcon(QIcon(":/resources/images/form.png"));
    playButton.setIcon(QIcon(":/resources/images/play.png"));
    buildButton.setIcon(QIcon(":/resources/images/build.png"));

    connect(&editorModeButton, SIGNAL(clicked(bool)),
      SLOT(handleEditorModeButtonClick()));
    connect(&cGuiModeButton, SIGNAL(clicked(bool)),
      SLOT(handleCGuiModeButtonClick()));
    connect(&wGuiModeButton, SIGNAL(clicked(bool)),
      SLOT(handleWGuiModeButtonClick()));
    connect(&playButton, SIGNAL(clicked(bool)),
      SLOT(handlePlayButtonClick()));
    connect(&buildButton, SIGNAL(clicked(bool)),
      SLOT(handleBuildButtonClick()));

    toolbar2.setSizePolicy(QSizePolicy::Preferred,
      QSizePolicy::Expanding);
    toolbar2.setOrientation(Qt::Vertical);
    toolbar2.setStyleSheet(CSS::DesignerToolbarV);
    toolbar2.setFixedWidth(fit::fx(21));
    toolbar2.setIconSize(QSize(fit::fx(16), fit::fx(16)));
    toolbar2.addWidget(&wGuiModeButton);
    toolbar2.addWidget(&cGuiModeButton);
    toolbar2.addWidget(&editorModeButton);
    toolbar2.addWidget(spacer_2);
    toolbar2.addWidget(&playButton);
    toolbar2.addWidget(&buildButton);

    _errorChecker.setInterval(INTERVAL_ERROR_CHECK);
    connect(&_errorChecker, SIGNAL(timeout()),
      parent, SLOT(checkErrors()));
    _errorChecker.start();

    SaveTransaction::instance();
    connect((IssuesBox*)outputWidget.box(Issues), SIGNAL(entryDoubleClicked(Control*)),
      parent, SLOT(controlDoubleClick(Control*)));
    connect(cW, SIGNAL(doubleClicked(Control*)),
      parent, SLOT(controlDoubleClick(Control*)));
    connect(cW, SIGNAL(controlDropped(Control*,QPointF,QString)),
      parent, SLOT(controlDropped(Control*,QPointF,QString)));
    connect(cW, SIGNAL(skinChanged(Form*)), parent,
      SLOT(updateSkin()));
    connect(SaveBackend::instance(), SIGNAL(parserRunningChanged(bool)),
      SLOT(handleIndicatorChanges()));
    connect(PreviewBackend::instance(), SIGNAL(workingChanged(bool)),
      SLOT(handleIndicatorChanges()));


    connect(this, SIGNAL(modeChanged()), _d, SLOT(handleModeChange()));
    _handleModeChange();
    ((IssuesBox*)_outputWidget.box(Issues))->setCurrentMode(_mode);
}

DesignerWidget* DesignerWidget::instance()
{
    return _parent;
}

void DesignerWidget::setSettleWidget(QWidget* widget)
{
    _settleWidget = widget;
    if (_settleWidget)
        _settleWidget->setLayout(&_hlayout);
}

const DesignMode& DesignerWidget::mode() const
{
    return _mode;
}

void DesignerWidget::setMode(const DesignMode& mode)
{
    _mode = mode;
    ((IssuesBox*)_outputWidget.box(Issues))->setCurrentMode(_mode);
    emit _parent->modeChanged();
}

ControlScene* DesignerWidget::currentScene()
{
    return _currentScene;
}

ControlScene* DesignerWidget::controlScene()
{
    return &_controlScene;
}

FormScene* DesignerWidget::formScene()
{
    return &_formScene;
}

QmlEditorView* DesignerWidget::qmlEditorView()
{
    return &_qmlEditorView;
}

ControlView* DesignerWidget::controlView()
{
    return &_controlView;
}

FormView* DesignerWidget::formView()
{
    return &_formView;
}

LoadingIndicator* DesignerWidget::loadingIndicator()
{
    return &_loadingIndicator;
}

void DesignerWidget::updateSkin()
{
    auto form = formScene()->mainForm();
    if (form) {
        _noSkinButton.setChecked(false);
        _phoneLandscapeButton.setChecked(false);
        _phonePortraitButton.setChecked(false);
        _desktopSkinButton.setChecked(false);
        _noSkinButton.setEnabled(true);
        _phoneLandscapeButton.setEnabled(true);
        _phonePortraitButton.setEnabled(true);
        _desktopSkinButton.setEnabled(true);

        if (form->skin() == Skin::Desktop) {
            _desktopSkinButton.setChecked(true);
            _desktopSkinButton.setEnabled(false);
        } else if (form->skin() == Skin::NoSkin) {
            _noSkinButton.setChecked(true);
            _noSkinButton.setEnabled(false);
        } else if (form->skin() == Skin::PhonePortrait) {
            _phonePortraitButton.setChecked(true);
            _phonePortraitButton.setEnabled(false);
        } else {
            _phoneLandscapeButton.setChecked(true);
            _phoneLandscapeButton.setEnabled(false);
        }
    }
}

QSplitter* DesignerWidget::splitter()
{
    return &_splitter;
}

OutputWidget* DesignerWidget::outputWidget()
{
    return &_outputWidget;
}

void DesignerWidget::checkErrors()
{
    static_cast<IssuesBox*>(_outputWidget.box(Issues))->refresh();
//    MainWindow::instance()->inspectorPage()->refresh(); //FIXME
    _qmlEditorView.refreshErrors();
}

void DesignerWidget::handleControlClick(Control* control)
{
    currentScene()->clearSelection();
    control->setSelected(true);
}

void DesignerWidget::handleControlDoubleClick(Control* control)
{
    auto sizes = _splitter.sizes();
    QSize size;
    if (_formView.isVisible())
        size = _formView.size();
    else if (_qmlEditorView.isVisible())
        size = _qmlEditorView.size();
    else
        size = _controlView.size();
    sizes[_splitter.indexOf(&_qmlEditorView)] = size.height();

    _qmlEditorView.addControl(control);
    if (_qmlEditorView.pinned())
        DesignerWidget::setMode(CodeEdit);
    _qmlEditorView.setMode(QmlEditorView::CodeEditor);
    _qmlEditorView.openControl(control);
    _qmlEditorView.raiseContainer();

    _splitter.setSizes(sizes);
}

void DesignerWidget::handleControlDrop(Control* control, const QPointF& pos, const QString& url)
{
    auto scene = (ControlScene*)control->scene();
    scene->clearSelection();
    auto newControl = new Control(url, control->mode());
    SaveBackend::addControl(newControl, control,
      scene->mainControl()->uid(), scene->mainControl()->dir());
    newControl->setParentItem(control);
    newControl->setPos(pos);
    newControl->setSelected(true);
    newControl->refresh();
}

#include "designerwidget.moc"
