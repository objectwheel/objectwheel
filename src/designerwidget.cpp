#include <designerwidget.h>
#include <designerscene.h>
#include <designerview.h>
#include <fit.h>
#include <qmleditorview.h>

#include <QToolBar>
#include <QToolButton>
#include <QComboBox>
#include <QPainter>
#include <QVBoxLayout>

namespace {
    QString findText(qreal ratio);
    qreal roundRatio(qreal ratio);
    qreal findRatio(const QString& text);
}

DesignerWidget::DesignerWidget(QmlEditorView* qmlEditorView, QWidget *parent) : QWidget(parent)
  , m_lastScale(1.0)
  , m_qmlEditorView(qmlEditorView)
  , m_layout(new QVBoxLayout(this))
  , m_designerScene(new DesignerScene(this))
  , m_designerView(new DesignerView(m_designerScene))
  , m_toolbar(new QToolBar)
  , m_undoButton(new QToolButton)
  , m_redoButton(new QToolButton)
  , m_clearButton(new QToolButton)
  , m_refreshButton(new QToolButton)
  , m_snappingButton(new QToolButton)
  , m_fitButton(new QToolButton)
  , m_outlineButton(new QToolButton)
  , m_zoomlLevelCombobox(new QComboBox)
{
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_toolbar);
    m_layout->addWidget(m_designerView);

    m_designerView->setRenderHint(QPainter::Antialiasing);
    m_designerView->setRubberBandSelectionMode(Qt::IntersectsItemShape);
    m_designerView->setDragMode(QGraphicsView::RubberBandDrag);
    m_designerView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_designerView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    m_designerView->setBackgroundBrush(QColor("#edf2f5"));
    m_designerView->setFrameShape(QFrame::NoFrame);
    m_designerView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    #if defined(Q_OS_WIN)
    m_zoomlLevelCombobox->setFixedHeight(fit::fx(18));
    #endif
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

    m_outlineButton->setCheckable(true);
    m_outlineButton->setChecked(Control::showOutline());
    m_snappingButton->setCheckable(true);
    m_snappingButton->setChecked(m_designerScene->snapping());

    m_phonePortraitButton->setCheckable(true);
    m_phonePortraitButton->setChecked(true);
    m_phonePortraitButton->setDisabled(true);
    m_phoneLandscapeButton->setCheckable(true);
    m_desktopSkinButton->setCheckable(true);
    m_noSkinButton->setCheckable(true);

    m_refreshPreviewButton->setCursor(Qt::PointingHandCursor);
    m_clearFormButton->setCursor(Qt::PointingHandCursor);
    m_undoButton->setCursor(Qt::PointingHandCursor);
    m_redoButton->setCursor(Qt::PointingHandCursor);
    m_phonePortraitButton->setCursor(Qt::PointingHandCursor);
    m_phoneLandscapeButton->setCursor(Qt::PointingHandCursor);
    m_desktopSkinButton->setCursor(Qt::PointingHandCursor);
    m_noSkinButton->setCursor(Qt::PointingHandCursor);
    m_snappingButton->setCursor(Qt::PointingHandCursor);
    m_showOutlineButton->setCursor(Qt::PointingHandCursor);
    m_fitInSceneButton->setCursor(Qt::PointingHandCursor);
    m_zoomlLevelCombobox->setCursor(Qt::PointingHandCursor);
    m_themeCombobox->setCursor(Qt::PointingHandCursor);
    m_layItVertButton->setCursor(Qt::PointingHandCursor);
    m_layItHorzButton->setCursor(Qt::PointingHandCursor);
    m_layItGridButton->setCursor(Qt::PointingHandCursor);
    m_breakLayoutButton->setCursor(Qt::PointingHandCursor);

    m_refreshPreviewButton->setToolTip("Refresh control previews on the Dashboard.");
    m_clearFormButton->setToolTip("Clear controls on the Dashboard.");
    m_undoButton->setToolTip("Undo action.");
    m_redoButton->setToolTip("Redo action.");
    m_phonePortraitButton->setToolTip("Skin: Phone portrait.");
    m_phoneLandscapeButton->setToolTip("Skin: Phone landscape.");
    m_desktopSkinButton->setToolTip("Skin: Desktop.");
    m_noSkinButton->setToolTip("No skin (free).");
    m_snappingButton->setToolTip("Enable snapping to help aligning of controls to each others.");
    m_showOutlineButton->setToolTip("Show outline frame for controls.");
    m_fitInSceneButton->setToolTip("Fit scene into the Dashboard.");
    m_zoomlLevelCombobox->setToolTip("Change zoom level.");
    m_themeCombobox->setToolTip("Change Theme. Themes are only available for Quick Controls 2.0 and above.");
    m_loadingIndicator->setToolTip("Background operations indicator.");
    m_layItVertButton->setToolTip("Lay out selected controls vertically.");
    m_layItHorzButton->setToolTip("Lay out selected controls horizontally.");
    m_layItGridButton->setToolTip("Lay out selected controls as grid.");
    m_breakLayoutButton->setToolTip("Break layouts for selected controls.");

    m_refreshPreviewButton->setIcon(QIcon(":/resources/images/refresh.png"));
    m_clearFormButton->setIcon(QIcon(":/resources/images/clean.png"));
    m_undoButton->setIcon(QIcon(":/resources/images/undo.png"));
    m_redoButton->setIcon(QIcon(":/resources/images/redo.png"));
    m_phonePortraitButton->setIcon(QIcon(":/resources/images/portrait.png"));
    m_phoneLandscapeButton->setIcon(QIcon(":/resources/images/landscape.png"));
    m_desktopSkinButton->setIcon(QIcon(":/resources/images/desktop.png"));
    m_noSkinButton->setIcon(QIcon(":/resources/images/free.png"));
    m_snappingButton->setIcon(QIcon(":/resources/images/snap.png"));
    m_showOutlineButton->setIcon(QIcon(":/resources/images/outline.png"));
    m_fitInSceneButton->setIcon(QIcon(":/resources/images/fit.png"));
    m_layItVertButton->setIcon(QIcon(":/resources/images/vert.png"));
    m_layItHorzButton->setIcon(QIcon(":/resources/images/hort.png"));
    m_layItGridButton->setIcon(QIcon(":/resources/images/grid.png"));
    m_breakLayoutButton->setIcon(QIcon(":/resources/images/break.png"));

    connect(_snappingButton, SIGNAL(toggled(bool)),
            SLOT(onSnappingClick(bool)));
    connect(_showOutlineButton, SIGNAL(toggled(bool)),
            SLOT(onShowOutlineClick(bool)));
    connect(_zoomlLevelCombobox, SIGNAL(currentTextChanged(QString)),
            SLOT(onZoomLevelChange(QString)));
    connect(_themeCombobox, SIGNAL(currentTextChanged(QString)),
            SLOT(onThemeChange(QString)));
    connect(_fitInSceneButton, SIGNAL(clicked(bool)),
            SLOT(onFitInSceneClick()));
    connect(_refreshPreviewButton, SIGNAL(clicked(bool)),
            SLOT(onRefreshPreviewClick()));
    connect(_clearFormButton, SIGNAL(clicked(bool)),
            SLOT(onClearControls()));
    connect(_phonePortraitButton, SIGNAL(clicked(bool)),
            SLOT(onPhonePortraitButtonClick()));
    connect(_phoneLandscapeButton, SIGNAL(clicked(bool)),
            SLOT(onPhoneLandscapeButtonClick()));
    connect(_desktopSkinButton, SIGNAL(clicked(bool)),
            SLOT(onDesktopSkinButtonClick()));
    connect(_noSkinButton, SIGNAL(clicked(bool)),
            SLOT(onNoSkinButtonClick()));

    m_toolbar->setStyleSheet(CSS::DesignerToolbar);
    m_toolbar->setFixedHeight(fit::fx(21));
    m_toolbar->setIconSize(QSize(fit::fx(14), fit::fx(14)));
    m_toolbar->addWidget(_undoButton);
    m_toolbar->addWidget(_redoButton);
    m_toolbar->addSeparator();
    m_toolbar->addWidget(_refreshPreviewButton);
    m_toolbar->addWidget(_clearFormButton);
    m_toolbar->addSeparator();
    m_toolbar->addWidget(_phonePortraitButton);
    m_toolbar->addWidget(_phoneLandscapeButton);
    m_toolbar->addWidget(_desktopSkinButton);
    m_toolbar->addWidget(_noSkinButton);
    m_toolbar->addSeparator();
    m_toolbar->addWidget(_snappingButton);
    m_toolbar->addWidget(_showOutlineButton);
    m_toolbar->addWidget(_fitInSceneButton);
    m_toolbar->addWidget(_themeCombobox);
    m_toolbar->addWidget(_zoomlLevelCombobox);
    m_toolbar->addSeparator();
    m_toolbar->addWidget(_loadingIndicator);
    m_toolbar->addWidget(spacer);
    m_toolbar->addWidget(_layItVertButton);
    m_toolbar->addWidget(_layItHorzButton);
    m_toolbar->addWidget(_layItGridButton);
    m_toolbar->addWidget(_breakLayoutButton);


    SaveTransaction::instance();
    connect(_outputPane->issuesBox(), SIGNAL(entryDoubleClicked(Control*)),
            this, SLOT(onControlDoubleClick(Control*)));
    connect(cW, SIGNAL(doubleClicked(Control*)),
            this, SLOT(onControlDoubleClick(Control*)));
    connect(cW, SIGNAL(controlDropped(Control*,QPointF,QString)),
            this, SLOT(onControlDrop(Control*,QPointF,QString)));
    connect(PreviewerBackend::instance(), SIGNAL(busyChanged()),
            SLOT(onIndicatorChanges()));

    connect(this, SIGNAL(modeChanged()), this, SLOT(onModeChange()));
    onModeChange();
    _outputPane->issuesBox()->setCurrentMode(_mode);
}

void DesignerWidget::scaleScene(qreal ratio)
{
    m_designerView->scale((1.0 / m_lastScale) * ratio, (1.0 / m_lastScale) * ratio);
    m_lastScale = ratio;
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
    for (auto ratio : ratios)
        if (roundRatio(diff) == ratio)
            m_zoomlLevelCombobox->setCurrentText(findText(ratio));
}

void DesignerWidget::onZoomLevelChange(const QString& text)
{
    qreal ratio = findRatio(text);
    scaleScene(ratio);
}

void DesignerWidget::onRefreshButtonClick()
{
    m_designerScene->mainForm()->refresh();
    for (auto control : m_designerScene->mainForm()->childControls())
        control->refresh();
}

void DesignerWidget::onClearButtonClick()
{
    if (!m_designerScene->mainForm())
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

void DesignerWidget::reset()
{
    m_lastScale = 1.0;
}

void DesignerWidget::onControlClick(Control* control)
{
    m_designerScene->clearSelection();
    control->setSelected(true);
}

void DesignerWidget::onControlDoubleClick(Control* control)
{
    _qmlEditorView->addControl(control);
    if (_qmlEditorView->pinned())
        setMode(CodeEdit);
    _qmlEditorView->setMode(QmlEditorView::CodeEditor);
    _qmlEditorView->openControl(control);
    _qmlEditorView->raiseContainer();

    _splitter->setSizes(sizes);
}

void DesignerWidget::onControlDrop(Control* control, const QPointF& pos, const QString& url)
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

namespace {
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
}