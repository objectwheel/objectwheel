#include <designmanager.h>
#include <formscene.h>
#include <formview.h>
#include <controlscene.h>
#include <controlview.h>
#include <qmleditorview.h>
#include <control.h>
#include <fit.h>
#include <css.h>
#include <loadingindicator.h>
#include <savemanager.h>
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

using namespace Fit;

class DesignManagerPrivate : public QObject
{
        Q_OBJECT

    public:
        DesignManagerPrivate(DesignManager* parent);
        ~DesignManagerPrivate();

        qreal roundRatio(qreal ratio);
        qreal findRatio(const QString& text);
        QString findText(qreal ratio);
        void scaleScene(qreal ratio);

    public slots:
        void handleSnappingClicked(bool value);
        void handleShowOutlineClicked(bool value);
        void handleFitInSceneClicked();
        void handleZoomLevelChange(const QString& text);
        void handlePhonePortraitButtonClicked();
        void handlePhoneLandscapeButtonClicked();
        void handleDesktopSkinButtonClicked();
        void handleNoSkinButtonClicked();
        void handleRefreshPreviewClicked();
        void handleClearControls();
        void handleEditorModeButtonClicked();
        void handleCGuiModeButtonClicked();
        void handleWGuiModeButtonClicked();
        void handlePlayButtonClicked();
        void handleBuildButtonClicked();

    public:
        DesignManager* parent;
        QWidget dummyWidget;
        QWidget* settleWidget = nullptr;
        QHBoxLayout hlayout;

        QToolBar toolbar_2;
        QToolButton editorModeButton;
        QToolButton wGuiModeButton;
        QToolButton cGuiModeButton;
        QToolButton playButton;
        QToolButton buildButton;

        QVBoxLayout vlayout;
        FormScene formScene;
        ControlScene controlScene;
        FormView formView;
        ControlView controlView;
        QmlEditorView qmlEditorView;
        qreal lastScaleOfWv;
        qreal lastScaleOfCv;
        QToolBar toolbar;
        QToolButton refreshPreviewButton;
        QToolButton clearFormButton;
        QToolButton undoButton;
        QToolButton redoButton;
        QToolButton phonePortraitButton;
        QToolButton phoneLandscapeButton;
        QToolButton desktopSkinButton;
        QToolButton noSkinButton;
        QToolButton snappingButton;
        QToolButton showOutlineButton;
        QToolButton fitInSceneButton;
        QComboBox zoomlLevelCombobox;
        LoadingIndicator loadingIndicator;
        QToolButton layItVertButton;
        QToolButton layItHorzButton;
        QToolButton layItGridButton;
        QToolButton breakLayoutButton;
};

DesignManagerPrivate::DesignManagerPrivate(DesignManager* parent)
    : QObject(parent)
    , parent(parent)
    , formView(&formScene)
    , controlView(&controlScene)
    , lastScaleOfWv(1.0)
    , lastScaleOfCv(1.0)
{
    dummyWidget.setHidden(true);

    hlayout.setContentsMargins(0, 0, 0, 0);
    hlayout.setSpacing(0);
    hlayout.addWidget(&toolbar_2);
    hlayout.addLayout(&vlayout);

    vlayout.setContentsMargins(0, 0, 0, 0);
    vlayout.setSpacing(0);
    vlayout.addWidget(&toolbar);
    vlayout.addWidget(&formView);
    vlayout.addWidget(&controlView);
    vlayout.addWidget(&qmlEditorView);

    formView.setRenderHint(QPainter::Antialiasing);
    formView.setRubberBandSelectionMode(Qt::IntersectsItemShape);
    formView.setDragMode(QGraphicsView::RubberBandDrag);
    formView.setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    formView.setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    formView.setBackgroundBrush(QColor("#e0e4e7"));
    formView.setFrameShape(QFrame::NoFrame);

    controlView.setRenderHint(QPainter::Antialiasing);
    controlView.setRubberBandSelectionMode(Qt::IntersectsItemShape);
    controlView.setDragMode(QGraphicsView::RubberBandDrag);
    controlView.setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    controlView.setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    controlView.setBackgroundBrush(QColor("#e0e4e7"));
    controlView.setFrameShape(QFrame::NoFrame);

    // Toolbar settings
    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

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

    connect(&snappingButton, SIGNAL(toggled(bool)), SLOT(handleSnappingClicked(bool)));
    connect(&showOutlineButton, SIGNAL(toggled(bool)), SLOT(handleShowOutlineClicked(bool)));
    connect(&zoomlLevelCombobox, SIGNAL(currentTextChanged(QString)), SLOT(handleZoomLevelChange(QString)));
    connect(&fitInSceneButton, SIGNAL(clicked(bool)), SLOT(handleFitInSceneClicked()));
    connect(&refreshPreviewButton, SIGNAL(clicked(bool)), SLOT(handleRefreshPreviewClicked()));
    connect(&clearFormButton, SIGNAL(clicked(bool)), SLOT(handleClearControls()));
    connect(&phonePortraitButton, SIGNAL(clicked(bool)), SLOT(handlePhonePortraitButtonClicked()));
    connect(&phoneLandscapeButton, SIGNAL(clicked(bool)), SLOT(handlePhoneLandscapeButtonClicked()));
    connect(&desktopSkinButton, SIGNAL(clicked(bool)), SLOT(handleDesktopSkinButtonClicked()));
    connect(&noSkinButton, SIGNAL(clicked(bool)), SLOT(handleNoSkinButtonClicked()));

    toolbar.setStyleSheet(CSS::DesignerToolbar);
    toolbar.setFixedHeight(fit(21));
    toolbar.setIconSize(QSize(fit(14), fit(14)));
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
    toolbar.addWidget(&zoomlLevelCombobox);
    toolbar.addSeparator();
    toolbar.addWidget(&loadingIndicator);
    toolbar.addWidget(spacer);
    toolbar.addWidget(&layItVertButton);
    toolbar.addWidget(&layItHorzButton);
    toolbar.addWidget(&layItGridButton);
    toolbar.addWidget(&breakLayoutButton);

    // Toolbar_2 settings
    QWidget* spacer_2 = new QWidget;
    spacer_2->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

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
    cGuiModeButton.setToolTip("Switch to Control Editor.");
    wGuiModeButton.setToolTip("Switch to Form Editor.");
    playButton.setToolTip("Run project.");
    buildButton.setToolTip("Get builds.");

    editorModeButton.setIcon(QIcon(":/resources/images/text.png"));
    cGuiModeButton.setIcon(QIcon(":/resources/images/gui.png"));
    wGuiModeButton.setIcon(QIcon(":/resources/images/form.png"));
    playButton.setIcon(QIcon(":/resources/images/play.png"));
    buildButton.setIcon(QIcon(":/resources/images/build.png"));

    connect(&editorModeButton, SIGNAL(clicked(bool)), SLOT(handleEditorModeButtonClicked()));
    connect(&cGuiModeButton, SIGNAL(clicked(bool)), SLOT(handleCGuiModeButtonClicked()));
    connect(&wGuiModeButton, SIGNAL(clicked(bool)), SLOT(handleWGuiModeButtonClicked()));
    connect(&playButton, SIGNAL(clicked(bool)), SLOT(handlePlayButtonClicked()));
    connect(&buildButton, SIGNAL(clicked(bool)), SLOT(handleBuildButtonClicked()));

    toolbar_2.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    toolbar_2.setOrientation(Qt::Vertical);
    toolbar_2.setStyleSheet(CSS::DesignerToolbarV);
    toolbar_2.setFixedWidth(fit(21));
    toolbar_2.setIconSize(QSize(fit(16), fit(16)));
    toolbar_2.addWidget(&wGuiModeButton);
    toolbar_2.addWidget(&cGuiModeButton);
    toolbar_2.addWidget(&editorModeButton);
    toolbar_2.addWidget(spacer_2);
    toolbar_2.addWidget(&playButton);
    toolbar_2.addWidget(&buildButton);
}

DesignManagerPrivate::~DesignManagerPrivate()
{
    dummyWidget.setLayout(&hlayout);
}

qreal DesignManagerPrivate::roundRatio(qreal ratio)
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

qreal DesignManagerPrivate::findRatio(const QString& text)
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

QString DesignManagerPrivate::findText(qreal ratio)
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

void DesignManagerPrivate::scaleScene(qreal ratio)
{
    if (DesignManager::_mode == DesignManager::FormGUI) {
        formView.scale((1.0 / lastScaleOfWv) * ratio, (1.0 / lastScaleOfWv) * ratio);
        lastScaleOfWv = ratio;
    } else {
        controlView.scale((1.0 / lastScaleOfCv) * ratio, (1.0 / lastScaleOfCv) * ratio);
        lastScaleOfCv = ratio;
    }
}

void DesignManagerPrivate::handleSnappingClicked(bool value)
{
    if (DesignManager::_mode == DesignManager::FormGUI)
        formScene.setSnapping(value);
    else if (DesignManager::_mode == DesignManager::ControlGUI)
        controlScene.setSnapping(value);
}

void DesignManagerPrivate::handleShowOutlineClicked(bool value)
{
    if (DesignManager::_mode == DesignManager::FormGUI ||
        DesignManager::_mode == DesignManager::ControlGUI) {
        formScene.setShowOutlines(value);
        controlScene.setShowOutlines(value);
    }
}

void DesignManagerPrivate::handleFitInSceneClicked()
{
    auto ratios = { 0.1, 0.25, 0.5, 0.75, 0.9, 1.0, 1.25, 1.50, 1.75, 2.0, 3.0, 5.0, 10.0 };
    auto diff = DesignManager::_mode == DesignManager::FormGUI ?
                    qMin(formView.width() / formScene.width(),
                         formView.height() / formScene.height()) :
                    qMin(controlView.width() / controlScene.width(),
                         controlView.height() / controlScene.height());;
    for (auto ratio : ratios)
        if (roundRatio(diff) == ratio)
            zoomlLevelCombobox.setCurrentText(findText(ratio));
}

void DesignManagerPrivate::handleZoomLevelChange(const QString& text)
{
    qreal ratio = findRatio(text);
    scaleScene(ratio);
}

void DesignManagerPrivate::handlePhonePortraitButtonClicked()
{
    auto form = DesignManager::formScene()->mainForm();
    form->setSkin(Skin::PhonePortrait);
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

void DesignManagerPrivate::handlePhoneLandscapeButtonClicked()
{
    auto form = DesignManager::formScene()->mainForm();
    form->setSkin(Skin::PhoneLandscape);
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

void DesignManagerPrivate::handleDesktopSkinButtonClicked()
{
    auto form = DesignManager::formScene()->mainForm();
    form->setSkin(Skin::Desktop);
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

void DesignManagerPrivate::handleNoSkinButtonClicked()
{
    auto form = DesignManager::formScene()->mainForm();
    form->setSkin(Skin::NoSkin);
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

void DesignManagerPrivate::handleRefreshPreviewClicked()
{
    if (DesignManager::_mode == DesignManager::FormGUI) {
        for (auto control : formScene.controls())
            control->refresh();
    } else {
        for (auto control : controlScene.controls())
            control->refresh();
    }
}

void DesignManagerPrivate::handleClearControls()
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
            SaveManager::removeChildControlsOnly(scene->mainControl());
            break;
        } default: {
            // Do nothing
            break;
        }
    }
}

void DesignManagerPrivate::handleEditorModeButtonClicked()
{
    DesignManager::setMode(DesignManager::CodeEdit);
}

void DesignManagerPrivate::handleCGuiModeButtonClicked()
{
    DesignManager::setMode(DesignManager::ControlGUI);
}

void DesignManagerPrivate::handleWGuiModeButtonClicked()
{
    DesignManager::setMode(DesignManager::FormGUI);
}
void DesignManagerPrivate::handlePlayButtonClicked()
{
    SaveManager::execProject();
}

void DesignManagerPrivate::handleBuildButtonClicked()
{
    //TODO
}

void DesignManager::handleModeChange()
{
    if (DesignManager::_mode == DesignManager::FormGUI) {
        if (MainWindow::instance())
            MainWindow::instance()->showDockWidgets();
        _d->wGuiModeButton.setChecked(true);
        _d->wGuiModeButton.setDisabled(true);
        _d->cGuiModeButton.setChecked(false);
        _d->editorModeButton.setChecked(false);
        _d->cGuiModeButton.setEnabled(true);
        _d->editorModeButton.setEnabled(true);

        auto form = DesignManager::formScene()->mainForm();
        if (form) {
            if (form->skin() == Skin::Desktop) {
                _d->desktopSkinButton.setChecked(true);
                _d->handleDesktopSkinButtonClicked();
            } else if (form->skin() == Skin::NoSkin) {
                _d->noSkinButton.setChecked(true);
                _d->handleNoSkinButtonClicked();
            } else if (form->skin() == Skin::PhonePortrait) {
                _d->phonePortraitButton.setChecked(true);
                _d->handlePhonePortraitButtonClicked();
            } else {
                _d->phoneLandscapeButton.setChecked(true);
                _d->handlePhoneLandscapeButtonClicked();
            }
        }

        _d->snappingButton.setChecked(_d->formScene.snapping());
        _d->snappingButton.setEnabled(true);
        _d->refreshPreviewButton.setEnabled(true);
        _d->clearFormButton.setEnabled(true);
        _d->showOutlineButton.setChecked(_d->formScene.showOutlines());
        _d->showOutlineButton.setEnabled(true);
        _d->fitInSceneButton.setEnabled(true);
        _d->layItVertButton.setEnabled(true);
        _d->layItHorzButton.setEnabled(true);
        _d->layItGridButton.setEnabled(true);
        _d->breakLayoutButton.setEnabled(true);
        _d->zoomlLevelCombobox.setCurrentText(_d->findText(_d->lastScaleOfWv));
        _d->controlView.hide();
        _d->qmlEditorView.hide();
        _d->formView.show();
        _d->toolbar.show();
        _currentScene = &_d->formScene;
    } else if (DesignManager::_mode == DesignManager::ControlGUI) {
        if (MainWindow::instance())
            MainWindow::instance()->showDockWidgets();
        _d->cGuiModeButton.setChecked(true);
        _d->cGuiModeButton.setDisabled(true);
        _d->editorModeButton.setChecked(false);
        _d->wGuiModeButton.setChecked(false);
        _d->editorModeButton.setEnabled(true);
        _d->wGuiModeButton.setEnabled(true);
        _d->noSkinButton.setChecked(true);
        _d->phoneLandscapeButton.setChecked(false);
        _d->phonePortraitButton.setChecked(false);
        _d->desktopSkinButton.setChecked(false);
        _d->noSkinButton.setDisabled(true);
        _d->phonePortraitButton.setDisabled(true);
        _d->phoneLandscapeButton.setDisabled(true);
        _d->desktopSkinButton.setDisabled(true);
        if (_d->controlScene.mainControl())
            _d->controlScene.mainControl()->centralize();
        _d->snappingButton.setChecked(_d->controlScene.snapping());
        _d->snappingButton.setEnabled(true);
        _d->refreshPreviewButton.setEnabled(true);
        _d->clearFormButton.setEnabled(true);
        _d->showOutlineButton.setChecked(_d->controlScene.showOutlines());
        _d->showOutlineButton.setEnabled(true);
        _d->fitInSceneButton.setEnabled(true);
        _d->layItVertButton.setEnabled(true);
        _d->layItHorzButton.setEnabled(true);
        _d->layItGridButton.setEnabled(true);
        _d->breakLayoutButton.setEnabled(true);
        _d->zoomlLevelCombobox.setCurrentText(_d->findText(_d->lastScaleOfCv));
        _d->formView.hide();
        _d->qmlEditorView.hide();
        _d->controlView.show();
        _d->toolbar.show();
        _currentScene = &_d->controlScene;
    } else {
        if (MainWindow::instance())
            MainWindow::instance()->hideDockWidgets();
        _d->editorModeButton.setChecked(true);
        _d->editorModeButton.setDisabled(true);
        _d->cGuiModeButton.setChecked(false);
        _d->wGuiModeButton.setChecked(false);
        _d->cGuiModeButton.setEnabled(true);
        _d->wGuiModeButton.setEnabled(true);
        _d->noSkinButton.setChecked(false);
        _d->phoneLandscapeButton.setChecked(false);
        _d->phonePortraitButton.setChecked(false);
        _d->desktopSkinButton.setChecked(false);
        _d->noSkinButton.setDisabled(true);
        _d->phonePortraitButton.setDisabled(true);
        _d->phoneLandscapeButton.setDisabled(true);
        _d->desktopSkinButton.setDisabled(true);
        _d->snappingButton.setChecked(false);
        _d->snappingButton.setDisabled(true);
        _d->showOutlineButton.setChecked(false);
        _d->showOutlineButton.setDisabled(true);
        _d->refreshPreviewButton.setDisabled(true);
        _d->clearFormButton.setDisabled(true);
        _d->fitInSceneButton.setDisabled(true);
        _d->layItVertButton.setDisabled(true);
        _d->layItHorzButton.setDisabled(true);
        _d->layItGridButton.setDisabled(true);
        _d->breakLayoutButton.setDisabled(true);
        _d->toolbar.hide();
        _d->formView.hide();
        _d->controlView.hide();
        _d->qmlEditorView.show();
    }
}

DesignManagerPrivate* DesignManager::_d = nullptr;
DesignManager::Mode DesignManager::_mode = DesignManager::FormGUI;
ControlScene* DesignManager::_currentScene = nullptr;

DesignManager::DesignManager(QObject *parent)
    : QObject(parent)
{
    if (_d) return;
    _d = new DesignManagerPrivate(this);
    connect(this, SIGNAL(modeChanged()), SLOT(handleModeChange()));
}

DesignManager* DesignManager::instance()
{
    return _d->parent;
}

void DesignManager::setSettleWidget(QWidget* widget)
{
    _d->settleWidget = widget;
    if (_d->settleWidget)
        _d->settleWidget->setLayout(&_d->hlayout);
}

const DesignManager::Mode& DesignManager::mode()
{
    return _mode;
}

void DesignManager::setMode(const Mode& mode)
{
    _mode = mode;
    emit _d->parent->modeChanged();
}

ControlScene* DesignManager::currentScene()
{
    return _currentScene;
}

ControlScene* DesignManager::controlScene()
{
    return &_d->controlScene;
}

FormScene* DesignManager::formScene()
{
    return &_d->formScene;
}

QmlEditorView* DesignManager::qmlEditorView()
{
    return &_d->qmlEditorView;
}

LoadingIndicator* DesignManager::loadingIndicator()
{
    return &_d->loadingIndicator;
}

#include "designmanager.moc"
