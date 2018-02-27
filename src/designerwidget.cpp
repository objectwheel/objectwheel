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
#include <saveutils.h>
#include <savebackend.h>
#include <outputpane.h>
#include <controlwatcher.h>
#include <savetransaction.h>
#include <mainwindow.h>
#include <issuesbox.h>
#include <projectbackend.h>
#include <previewerbackend.h>

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

void DesignerWidget::scaleScene(qreal ratio)
{
    if (_mode == FormGui) {
        _formView->scale((1.0 / _lastScaleOfWv) * ratio, (1.0 / _lastScaleOfWv) * ratio);
        _lastScaleOfWv = ratio;
    } else {
        _controlView->scale((1.0 / _lastScaleOfCv) * ratio, (1.0 / _lastScaleOfCv) * ratio);
        _lastScaleOfCv = ratio;
    }
}

void DesignerWidget::handleIndicatorChanges()
{
    _loadingIndicator->setRunning(SaveBackend::instance()->parserWorking() || PreviewerBackend::instance()->isWorking());
}

void DesignerWidget::handleSnappingClick(bool value)
{
    if (_mode == FormGui)
        _formScene->setSnapping(value);
    else if (_mode == ControlGui)
        _controlScene->setSnapping(value);
}

void DesignerWidget::handleShowOutlineClick(bool value)
{
    if (_mode == FormGui ||
        _mode == ControlGui) {
        _formScene->setShowOutlines(value);
        _controlScene->setShowOutlines(value);
    }
}

void DesignerWidget::handleFitInSceneClick()
{
    auto ratios = { 0.1, 0.25, 0.5, 0.75, 0.9, 1.0, 1.25, 1.50, 1.75, 2.0, 3.0, 5.0, 10.0 };
    auto diff = _mode == FormGui ?
                    qMin(_formView->width() / _formScene->width(),
                         _formView->height() / _formScene->height()) :
                    qMin(_controlView->width() / _controlScene->width(),
                         _controlView->height() / _controlScene->height());;
    for (auto ratio : ratios)
        if (roundRatio(diff) == ratio)
            _zoomlLevelCombobox->setCurrentText(findText(ratio));
}

void DesignerWidget::handleThemeChange(const QString& text)
{
    SaveUtils::setProjectProperty(ProjectBackend::instance()->dir(), PTAG_THEME, text);
    handleRefreshPreviewClick();
}

void DesignerWidget::handleZoomLevelChange(const QString& text)
{
    qreal ratio = findRatio(text);
    scaleScene(ratio);
}

void DesignerWidget::handlePhonePortraitButtonClick()
{
    auto form = formScene()->mainForm();
//    form->setSkin(SaveUtils::PhonePortrait);
    SaveBackend::instance()->setProperty(form, PTAG_SKIN, SaveUtils::PhonePortrait);
    _phonePortraitButton->setDisabled(true);
    _phoneLandscapeButton->setChecked(false);
    _desktopSkinButton->setChecked(false);
    _noSkinButton->setChecked(false);
    _phoneLandscapeButton->setEnabled(true);
    _desktopSkinButton->setEnabled(true);
    _noSkinButton->setEnabled(true);
    if (_formScene->mainControl())
        _formScene->mainControl()->centralize();
}

void DesignerWidget::handlePhoneLandscapeButtonClick()
{
    auto form = formScene()->mainForm();
//    form->setSkin(SaveUtils::PhoneLandscape);
    SaveBackend::instance()->setProperty(form, PTAG_SKIN, SaveUtils::PhoneLandscape);
    _phoneLandscapeButton->setDisabled(true);
    _phonePortraitButton->setChecked(false);
    _desktopSkinButton->setChecked(false);
    _noSkinButton->setChecked(false);
    _phonePortraitButton->setEnabled(true);
    _desktopSkinButton->setEnabled(true);
    _noSkinButton->setEnabled(true);
    if (_formScene->mainControl())
        _formScene->mainControl()->centralize();
}

void DesignerWidget::handleDesktopSkinButtonClick()
{
    auto form = formScene()->mainForm();
//    form->setSkin(SaveUtils::Desktop);
    SaveBackend::instance()->setProperty(form, PTAG_SKIN, SaveUtils::Desktop);
    _desktopSkinButton->setDisabled(true);
    _phoneLandscapeButton->setChecked(false);
    _phonePortraitButton->setChecked(false);
    _noSkinButton->setChecked(false);
    _phonePortraitButton->setEnabled(true);
    _phoneLandscapeButton->setEnabled(true);
    _noSkinButton->setEnabled(true);
    if (_formScene->mainControl())
        _formScene->mainControl()->centralize();
}

void DesignerWidget::handleNoSkinButtonClick()
{
    auto form = formScene()->mainForm();
//    form->setSkin(SaveUtils::NoSkin);
    SaveBackend::instance()->setProperty(form, PTAG_SKIN, SaveUtils::NoSkin);
    _noSkinButton->setDisabled(true);
    _phoneLandscapeButton->setChecked(false);
    _desktopSkinButton->setChecked(false);
    _phonePortraitButton->setChecked(false);
    _phonePortraitButton->setEnabled(true);
    _phoneLandscapeButton->setEnabled(true);
    _desktopSkinButton->setEnabled(true);
    if (_formScene->mainControl())
        _formScene->mainControl()->centralize();
}

void DesignerWidget::handleRefreshPreviewClick()
{
    if (_mode == FormGui) {
        _formScene->mainForm()->refresh();
        for (auto control : _formScene->mainForm()->childControls())
            control->refresh();
    } else {
        _controlScene->mainControl()->refresh();
        for (auto control : _controlScene->mainControl()->childControls())
            control->refresh();
    }
}

void DesignerWidget::handleClearControls()
{
    auto scene = currentScene();
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
            SaveBackend::instance()->removeChildControlsOnly(scene->mainControl());
            break;
        } default: {
            // Do nothing
            break;
        }
    }
}

void DesignerWidget::handleEditorModeButtonClick()
{
    setMode(CodeEdit);
}

void DesignerWidget::handleCGuiModeButtonClick()
{
    setMode(ControlGui);
}

void DesignerWidget::handleWGuiModeButtonClick()
{
    setMode(FormGui);
}

void DesignerWidget::handleModeChange()
{
    if (_mode == FormGui) {
        _wGuiModeButton->setChecked(true);
        _wGuiModeButton->setDisabled(true);
        _cGuiModeButton->setChecked(false);
        _editorModeButton->setChecked(false);
        _cGuiModeButton->setEnabled(true);
        _editorModeButton->setEnabled(true);

        auto form = formScene()->mainForm();
//        if (form) {
//            if (form->skin() == SaveUtils::Desktop) {
//                _desktopSkinButton->setChecked(true);
//                handleDesktopSkinButtonClick();
//            } else if (form->skin() == SaveUtils::NoSkin) {
//                _noSkinButton->setChecked(true);
//                handleNoSkinButtonClick();
//            } else if (form->skin() == SaveUtils::PhonePortrait) {
//                _phonePortraitButton->setChecked(true);
//                handlePhonePortraitButtonClick();
//            } else {
//                _phoneLandscapeButton->setChecked(true);
//                handlePhoneLandscapeButtonClick();
//            }
//        }

        _snappingButton->setChecked(_formScene->snapping());
        _snappingButton->setEnabled(true);
        _refreshPreviewButton->setEnabled(true);
        _clearFormButton->setEnabled(true);
        _showOutlineButton->setChecked(_formScene->showOutlines());
        _showOutlineButton->setEnabled(true);
        _fitInSceneButton->setEnabled(true);
        _layItVertButton->setEnabled(true);
        _layItHorzButton->setEnabled(true);
        _layItGridButton->setEnabled(true);
        _breakLayoutButton->setEnabled(true);
        _zoomlLevelCombobox->setCurrentText(findText(_lastScaleOfWv));
        auto sizes = _splitter->sizes();
        QSize size;
        if (_controlView->isVisible())
            size = _controlView->size();
        else
            size = _qmlEditorView->size();
        sizes[_splitter->indexOf(_formView)] = size.height();
        _controlView->hide();
        _qmlEditorView->hide();
        _formView->show();
        _toolbar->show();
        _splitter->setSizes(sizes);
        _currentScene = _formScene;
    } else if (_mode == ControlGui) {
        _cGuiModeButton->setChecked(true);
        _cGuiModeButton->setDisabled(true);
        _editorModeButton->setChecked(false);
        _wGuiModeButton->setChecked(false);
        _editorModeButton->setEnabled(true);
        _wGuiModeButton->setEnabled(true);
        _noSkinButton->setChecked(true);
        _phoneLandscapeButton->setChecked(false);
        _phonePortraitButton->setChecked(false);
        _desktopSkinButton->setChecked(false);
        _noSkinButton->setDisabled(true);
        _phonePortraitButton->setDisabled(true);
        _phoneLandscapeButton->setDisabled(true);
        _desktopSkinButton->setDisabled(true);
        if (_controlScene->mainControl())
            _controlScene->mainControl()->centralize();
        _snappingButton->setChecked(_controlScene->snapping());
        _snappingButton->setEnabled(true);
        _refreshPreviewButton->setEnabled(true);
        _clearFormButton->setEnabled(true);
        _showOutlineButton->setChecked(_controlScene->showOutlines());
        _showOutlineButton->setEnabled(true);
        _fitInSceneButton->setEnabled(true);
        _layItVertButton->setEnabled(true);
        _layItHorzButton->setEnabled(true);
        _layItGridButton->setEnabled(true);
        _breakLayoutButton->setEnabled(true);
        _zoomlLevelCombobox->setCurrentText(findText(_lastScaleOfCv));
        auto sizes = _splitter->sizes();
        QSize size;
        if (_formView->isVisible())
            size = _formView->size();
        else
            size = _qmlEditorView->size();
        sizes[_splitter->indexOf(_controlView)] = size.height();
        _formView->hide();
        _qmlEditorView->hide();
        _controlView->show();
        _toolbar->show();
        _splitter->setSizes(sizes);
        _currentScene = _controlScene;
    } else {
        _editorModeButton->setChecked(true);
        _editorModeButton->setDisabled(true);
        _cGuiModeButton->setChecked(false);
        _wGuiModeButton->setChecked(false);
        _cGuiModeButton->setEnabled(true);
        _wGuiModeButton->setEnabled(true);
        _noSkinButton->setChecked(false);
        _phoneLandscapeButton->setChecked(false);
        _phonePortraitButton->setChecked(false);
        _desktopSkinButton->setChecked(false);
        _noSkinButton->setDisabled(true);
        _phonePortraitButton->setDisabled(true);
        _phoneLandscapeButton->setDisabled(true);
        _desktopSkinButton->setDisabled(true);
        _snappingButton->setChecked(false);
        _snappingButton->setDisabled(true);
        _showOutlineButton->setChecked(false);
        _showOutlineButton->setDisabled(true);
        _refreshPreviewButton->setDisabled(true);
        _clearFormButton->setDisabled(true);
        _fitInSceneButton->setDisabled(true);
        _layItVertButton->setDisabled(true);
        _layItHorzButton->setDisabled(true);
        _layItGridButton->setDisabled(true);
        _breakLayoutButton->setDisabled(true);
        auto sizes = _splitter->sizes();
        QSize size;
        if (_formView->isVisible())
            size = _formView->size();
        else
            size = _controlView->size();
        sizes[_splitter->indexOf(_qmlEditorView)] = size.height();
        _toolbar->hide();
        _formView->hide();
        _controlView->hide();
        _qmlEditorView->show();
        _splitter->setSizes(sizes);
    }
}

DesignerWidget::DesignerWidget(QWidget *parent) : QFrame(parent)
  , _mode(FormGui)
  , _lastScaleOfWv(1.0)
  , _lastScaleOfCv(1.0)
  , _currentScene(nullptr)
{
    _hlayout = new QHBoxLayout(this);
    _toolbar2 = new QToolBar;
    _editorModeButton = new QToolButton;
    _wGuiModeButton = new QToolButton;
    _cGuiModeButton = new QToolButton;
    _vlayout = new QVBoxLayout;
    _splitter = new QSplitter;
    _formScene = new FormScene;
    _controlScene = new ControlScene;
    _formView = new FormView(_formScene);
    _controlView = new ControlView(_controlScene);
    _qmlEditorView = new QmlEditorView;
    _outputPane = new OutputPane;
    _toolbar = new QToolBar;
    _refreshPreviewButton = new QToolButton;
    _clearFormButton = new QToolButton;
    _undoButton = new QToolButton;
    _redoButton = new QToolButton;
    _phonePortraitButton = new QToolButton;
    _phoneLandscapeButton = new QToolButton;
    _desktopSkinButton = new QToolButton;
    _noSkinButton = new QToolButton;
    _snappingButton = new QToolButton;
    _showOutlineButton = new QToolButton;
    _fitInSceneButton = new QToolButton;
    _zoomlLevelCombobox = new QComboBox;
    _themeCombobox = new QComboBox;
    _loadingIndicator = new LoadingIndicator;
    _layItVertButton = new QToolButton;
    _layItHorzButton = new QToolButton;
    _layItGridButton = new QToolButton;
    _breakLayoutButton = new QToolButton;
    _errorChecker = new QTimer(this);

    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Plain);

    _hlayout->setContentsMargins(0, 0, 0, 0);
    _hlayout->setSpacing(0);
    _hlayout->addWidget(_toolbar2);
    _hlayout->addLayout(_vlayout);

    _vlayout->setContentsMargins(0, 0, 0, 0);
    _vlayout->setSpacing(0);
    _vlayout->addWidget(_splitter);

    _outputPane->setSplitter(_splitter);
    _outputPane->setSplitterHandle(_splitter->handle(4));
    connect(_splitter, SIGNAL(splitterMoved(int,int)),
            _outputPane, SLOT(updateLastHeight()));
    _qmlEditorView->setSizePolicy(QSizePolicy::Expanding,
                                  QSizePolicy::Expanding);

    //    _formView->setViewport(new QOpenGLWidget);
    _formView->setRenderHint(QPainter::Antialiasing);
    _formView->setRubberBandSelectionMode(Qt::IntersectsItemShape);
    _formView->setDragMode(QGraphicsView::RubberBandDrag);
    _formView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    _formView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    _formView->setBackgroundBrush(QColor("#e0e4e7"));
    _formView->setFrameShape(QFrame::NoFrame);
    _formView->setSizePolicy(QSizePolicy::Expanding,
                             QSizePolicy::Expanding);

    //    _controlView->setViewport(new QOpenGLWidget);
    _controlView->setRenderHint(QPainter::Antialiasing);
    _controlView->setRubberBandSelectionMode(Qt::IntersectsItemShape);
    _controlView->setDragMode(QGraphicsView::RubberBandDrag);
    _controlView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    _controlView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    _controlView->setBackgroundBrush(QColor("#e0e4e7"));
    _controlView->setFrameShape(QFrame::NoFrame);
    _controlView->setSizePolicy(QSizePolicy::Expanding,
                                QSizePolicy::Expanding);

    // Toolbar settings
    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding,
                          QSizePolicy::Preferred);

#if defined(Q_OS_WIN)
    _zoomlLevelCombobox->setFixedHeight(fit::fx(18));
#endif
    _zoomlLevelCombobox->addItem("10 %");
    _zoomlLevelCombobox->addItem("25 %");
    _zoomlLevelCombobox->addItem("50 %");
    _zoomlLevelCombobox->addItem("75 %");
    _zoomlLevelCombobox->addItem("90 %");
    _zoomlLevelCombobox->addItem("100 %");
    _zoomlLevelCombobox->addItem("125 %");
    _zoomlLevelCombobox->addItem("150 %");
    _zoomlLevelCombobox->addItem("175 %");
    _zoomlLevelCombobox->addItem("200 %");
    _zoomlLevelCombobox->addItem("300 %");
    _zoomlLevelCombobox->addItem("500 %");
    _zoomlLevelCombobox->addItem("1000 %");
    _zoomlLevelCombobox->setCurrentIndex(5);

#if defined(Q_OS_WIN)
    _themeCombobox->setFixedHeight(fit::fx(18));
#endif
    for (auto theme : QQuickStyle::availableStyles())
        _themeCombobox->addItem(theme);

    _showOutlineButton->setCheckable(true);
    _showOutlineButton->setChecked(Control::showOutline());
    _snappingButton->setCheckable(true);
    _snappingButton->setChecked(_formScene->snapping());

    _phonePortraitButton->setCheckable(true);
    _phonePortraitButton->setChecked(true);
    _phonePortraitButton->setDisabled(true);
    _phoneLandscapeButton->setCheckable(true);
    _desktopSkinButton->setCheckable(true);
    _noSkinButton->setCheckable(true);

    _refreshPreviewButton->setCursor(Qt::PointingHandCursor);
    _clearFormButton->setCursor(Qt::PointingHandCursor);
    _undoButton->setCursor(Qt::PointingHandCursor);
    _redoButton->setCursor(Qt::PointingHandCursor);
    _phonePortraitButton->setCursor(Qt::PointingHandCursor);
    _phoneLandscapeButton->setCursor(Qt::PointingHandCursor);
    _desktopSkinButton->setCursor(Qt::PointingHandCursor);
    _noSkinButton->setCursor(Qt::PointingHandCursor);
    _snappingButton->setCursor(Qt::PointingHandCursor);
    _showOutlineButton->setCursor(Qt::PointingHandCursor);
    _fitInSceneButton->setCursor(Qt::PointingHandCursor);
    _zoomlLevelCombobox->setCursor(Qt::PointingHandCursor);
    _themeCombobox->setCursor(Qt::PointingHandCursor);
    _layItVertButton->setCursor(Qt::PointingHandCursor);
    _layItHorzButton->setCursor(Qt::PointingHandCursor);
    _layItGridButton->setCursor(Qt::PointingHandCursor);
    _breakLayoutButton->setCursor(Qt::PointingHandCursor);

    _refreshPreviewButton->setToolTip("Refresh control previews on the Dashboard.");
    _clearFormButton->setToolTip("Clear controls on the Dashboard.");
    _undoButton->setToolTip("Undo action.");
    _redoButton->setToolTip("Redo action.");
    _phonePortraitButton->setToolTip("Skin: Phone portrait.");
    _phoneLandscapeButton->setToolTip("Skin: Phone landscape.");
    _desktopSkinButton->setToolTip("Skin: Desktop.");
    _noSkinButton->setToolTip("No skin (free).");
    _snappingButton->setToolTip("Enable snapping to help aligning of controls to each others.");
    _showOutlineButton->setToolTip("Show outline frame for controls.");
    _fitInSceneButton->setToolTip("Fit scene into the Dashboard.");
    _zoomlLevelCombobox->setToolTip("Change zoom level.");
    _themeCombobox->setToolTip("Change Theme. Themes are only available for Quick Controls 2.0 and above.");
    _loadingIndicator->setToolTip("Background operations indicator.");
    _layItVertButton->setToolTip("Lay out selected controls vertically.");
    _layItHorzButton->setToolTip("Lay out selected controls horizontally.");
    _layItGridButton->setToolTip("Lay out selected controls as grid.");
    _breakLayoutButton->setToolTip("Break layouts for selected controls.");

    _refreshPreviewButton->setIcon(QIcon(":/resources/images/refresh.png"));
    _clearFormButton->setIcon(QIcon(":/resources/images/clean.png"));
    _undoButton->setIcon(QIcon(":/resources/images/undo.png"));
    _redoButton->setIcon(QIcon(":/resources/images/redo.png"));
    _phonePortraitButton->setIcon(QIcon(":/resources/images/portrait.png"));
    _phoneLandscapeButton->setIcon(QIcon(":/resources/images/landscape.png"));
    _desktopSkinButton->setIcon(QIcon(":/resources/images/desktop.png"));
    _noSkinButton->setIcon(QIcon(":/resources/images/free.png"));
    _snappingButton->setIcon(QIcon(":/resources/images/snap.png"));
    _showOutlineButton->setIcon(QIcon(":/resources/images/outline.png"));
    _fitInSceneButton->setIcon(QIcon(":/resources/images/fit.png"));
    _layItVertButton->setIcon(QIcon(":/resources/images/vert.png"));
    _layItHorzButton->setIcon(QIcon(":/resources/images/hort.png"));
    _layItGridButton->setIcon(QIcon(":/resources/images/grid.png"));
    _breakLayoutButton->setIcon(QIcon(":/resources/images/break.png"));
    _loadingIndicator->setImage(QImage(":/resources/images/loading.png"));
    _loadingIndicator->setRunning(false);

    connect(_snappingButton, SIGNAL(toggled(bool)),
            SLOT(handleSnappingClick(bool)));
    connect(_showOutlineButton, SIGNAL(toggled(bool)),
            SLOT(handleShowOutlineClick(bool)));
    connect(_zoomlLevelCombobox, SIGNAL(currentTextChanged(QString)),
            SLOT(handleZoomLevelChange(QString)));
    connect(_themeCombobox, SIGNAL(currentTextChanged(QString)),
            SLOT(handleThemeChange(QString)));
    connect(_fitInSceneButton, SIGNAL(clicked(bool)),
            SLOT(handleFitInSceneClick()));
    connect(_refreshPreviewButton, SIGNAL(clicked(bool)),
            SLOT(handleRefreshPreviewClick()));
    connect(_clearFormButton, SIGNAL(clicked(bool)),
            SLOT(handleClearControls()));
    connect(_phonePortraitButton, SIGNAL(clicked(bool)),
            SLOT(handlePhonePortraitButtonClick()));
    connect(_phoneLandscapeButton, SIGNAL(clicked(bool)),
            SLOT(handlePhoneLandscapeButtonClick()));
    connect(_desktopSkinButton, SIGNAL(clicked(bool)),
            SLOT(handleDesktopSkinButtonClick()));
    connect(_noSkinButton, SIGNAL(clicked(bool)),
            SLOT(handleNoSkinButtonClick()));

    _toolbar->setStyleSheet(CSS::DesignerToolbar);
    _toolbar->setFixedHeight(fit::fx(21));
    _toolbar->setIconSize(QSize(fit::fx(14), fit::fx(14)));
    _toolbar->addWidget(_undoButton);
    _toolbar->addWidget(_redoButton);
    _toolbar->addSeparator();
    _toolbar->addWidget(_refreshPreviewButton);
    _toolbar->addWidget(_clearFormButton);
    _toolbar->addSeparator();
    _toolbar->addWidget(_phonePortraitButton);
    _toolbar->addWidget(_phoneLandscapeButton);
    _toolbar->addWidget(_desktopSkinButton);
    _toolbar->addWidget(_noSkinButton);
    _toolbar->addSeparator();
    _toolbar->addWidget(_snappingButton);
    _toolbar->addWidget(_showOutlineButton);
    _toolbar->addWidget(_fitInSceneButton);
    _toolbar->addWidget(_themeCombobox);
    _toolbar->addWidget(_zoomlLevelCombobox);
    _toolbar->addSeparator();
    _toolbar->addWidget(_loadingIndicator);
    _toolbar->addWidget(spacer);
    _toolbar->addWidget(_layItVertButton);
    _toolbar->addWidget(_layItHorzButton);
    _toolbar->addWidget(_layItGridButton);
    _toolbar->addWidget(_breakLayoutButton);

    // _toolbar2 settings
    _editorModeButton->setCheckable(true);
    _wGuiModeButton->setCheckable(true);
    _cGuiModeButton->setCheckable(true);
    _wGuiModeButton->setChecked(true);
    _wGuiModeButton->setDisabled(true);

    _editorModeButton->setCursor(Qt::PointingHandCursor);
    _wGuiModeButton->setCursor(Qt::PointingHandCursor);
    _cGuiModeButton->setCursor(Qt::PointingHandCursor);

    _editorModeButton->setToolTip("Switch to Qml Editor.");
    _cGuiModeButton->setToolTip("Switch to Tool Editor.");
    _wGuiModeButton->setToolTip("Switch to Form Editor.");

    _editorModeButton->setIcon(QIcon(":/resources/images/text.png"));
    _cGuiModeButton->setIcon(QIcon(":/resources/images/gui.png"));
    _wGuiModeButton->setIcon(QIcon(":/resources/images/form.png"));

    connect(_editorModeButton, SIGNAL(clicked(bool)),
            SLOT(handleEditorModeButtonClick()));
    connect(_cGuiModeButton, SIGNAL(clicked(bool)),
            SLOT(handleCGuiModeButtonClick()));
    connect(_wGuiModeButton, SIGNAL(clicked(bool)),
            SLOT(handleWGuiModeButtonClick()));

    _toolbar2->setSizePolicy(QSizePolicy::Preferred,
                             QSizePolicy::Expanding);
    _toolbar2->setOrientation(Qt::Vertical);
    _toolbar2->setStyleSheet(CSS::DesignerToolbarV);
    _toolbar2->setFixedWidth(fit::fx(21));
    _toolbar2->setIconSize(QSize(fit::fx(16), fit::fx(16)));
    _toolbar2->addWidget(_wGuiModeButton);
    _toolbar2->addWidget(_cGuiModeButton);
    _toolbar2->addWidget(_editorModeButton);

    _errorChecker->setInterval(INTERVAL_ERROR_CHECK);
    connect(_errorChecker, SIGNAL(timeout()),
            this, SLOT(checkErrors()));
    _errorChecker->start();

    _splitter->setStyleSheet("QSplitter{background: #e0e4e7;}");
    _splitter->setOrientation(Qt::Vertical);
    _splitter->addWidget(_toolbar);
    _splitter->addWidget(_formView);
    _splitter->addWidget(_controlView);
    _splitter->addWidget(_qmlEditorView);
    _splitter->addWidget(_outputPane);
    _splitter->setCollapsible(0, false);
    _splitter->setCollapsible(1, false);
    _splitter->setCollapsible(2, false);
    _splitter->setCollapsible(3, false);
    _splitter->setCollapsible(4, false);
    _splitter->handle(0)->setDisabled(true);
    _splitter->handle(1)->setDisabled(true);
    _splitter->handle(2)->setDisabled(true);
    _splitter->handle(3)->setDisabled(true);
    _splitter->setHandleWidth(0);

    SaveTransaction::instance();
    connect(_outputPane->issuesBox(), SIGNAL(entryDoubleClicked(Control*)),
            this, SLOT(handleControlDoubleClick(Control*)));
    connect(cW, SIGNAL(doubleClicked(Control*)),
            this, SLOT(handleControlDoubleClick(Control*)));
    connect(cW, SIGNAL(controlDropped(Control*,QPointF,QString)),
            this, SLOT(handleControlDrop(Control*,QPointF,QString)));
    connect(SaveBackend::instance(), SIGNAL(parserRunningChanged(bool)),
            SLOT(handleIndicatorChanges()));
    connect(PreviewerBackend::instance(), SIGNAL(stateChanged()),
            SLOT(handleIndicatorChanges()));

    connect(this, SIGNAL(modeChanged()), this, SLOT(handleModeChange()));
    handleModeChange();
    _outputPane->issuesBox()->setCurrentMode(_mode);
}

const DesignMode& DesignerWidget::mode() const
{
    return _mode;
}

void DesignerWidget::setMode(const DesignMode& mode)
{
    _mode = mode;
    _outputPane->issuesBox()->setCurrentMode(_mode);
    emit modeChanged();
}

ControlScene* DesignerWidget::currentScene()
{
    return _currentScene;
}

ControlScene* DesignerWidget::controlScene()
{
    return _controlScene;
}

FormScene* DesignerWidget::formScene()
{
    return _formScene;
}

QmlEditorView* DesignerWidget::qmlEditorView()
{
    return _qmlEditorView;
}

ControlView* DesignerWidget::controlView()
{
    return _controlView;
}

FormView* DesignerWidget::formView()
{
    return _formView;
}

LoadingIndicator* DesignerWidget::loadingIndicator()
{
    return _loadingIndicator;
}

void DesignerWidget::updateSkin()
{
    auto form = formScene()->mainForm();
    if (form) {
        _noSkinButton->setChecked(false);
        _phoneLandscapeButton->setChecked(false);
        _phonePortraitButton->setChecked(false);
        _desktopSkinButton->setChecked(false);
        _noSkinButton->setEnabled(true);
        _phoneLandscapeButton->setEnabled(true);
        _phonePortraitButton->setEnabled(true);
        _desktopSkinButton->setEnabled(true);

//        if (form->skin() == SaveUtils::Desktop) {
//            _desktopSkinButton->setChecked(true);
//            _desktopSkinButton->setEnabled(false);
//        } else if (form->skin() == SaveUtils::NoSkin) {
//            _noSkinButton->setChecked(true);
//            _noSkinButton->setEnabled(false);
//        } else if (form->skin() == SaveUtils::PhonePortrait) {
//            _phonePortraitButton->setChecked(true);
//            _phonePortraitButton->setEnabled(false);
//        } else {
//            _phoneLandscapeButton->setChecked(true);
//            _phoneLandscapeButton->setEnabled(false);
//        }
    }
}

QSplitter* DesignerWidget::splitter()
{
    return _splitter;
}

OutputPane* DesignerWidget::outputPane()
{
    return _outputPane;
}

void DesignerWidget::clear()
{
    _lastScaleOfWv = 1.0;
    _lastScaleOfCv = 1.0;
    setMode(FormGui);
}

void DesignerWidget::checkErrors()
{
    _outputPane->issuesBox()->refresh();
    //    MainWindow::instance()->inspectorPage()->refresh(); //FIXME
    _qmlEditorView->refreshErrors();
}

void DesignerWidget::handleControlClick(Control* control)
{
    currentScene()->clearSelection();
    control->setSelected(true);
}

void DesignerWidget::handleControlDoubleClick(Control* control)
{
    auto sizes = _splitter->sizes();
    QSize size;
    if (_formView->isVisible())
        size = _formView->size();
    else if (_qmlEditorView->isVisible())
        size = _qmlEditorView->size();
    else
        size = _controlView->size();
    sizes[_splitter->indexOf(_qmlEditorView)] = size.height();

    _qmlEditorView->addControl(control);
    if (_qmlEditorView->pinned())
        setMode(CodeEdit);
    _qmlEditorView->setMode(QmlEditorView::CodeEditor);
    _qmlEditorView->openControl(control);
    _qmlEditorView->raiseContainer();

    _splitter->setSizes(sizes);
}

void DesignerWidget::handleControlDrop(Control* control, const QPointF& pos, const QString& url)
{
    auto scene = (ControlScene*)control->scene();
    scene->clearSelection();
    auto newControl = new Control(url, control->mode());
    SaveBackend::instance()->addControl(newControl, control,
                            scene->mainControl()->uid(), scene->mainControl()->dir());
    newControl->setParentItem(control);
    newControl->setPos(pos);
    newControl->setSelected(true);
    newControl->refresh();
}
