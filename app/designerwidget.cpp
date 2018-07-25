#include <designerwidget.h>
#include <designerview.h>
#include <designerscene.h>
#include <qmlcodeeditorwidget.h>
#include <controlcreationmanager.h>
#include <transparentcombobox.h>
#include <utilsicons.h>
#include <toolbar.h>
#include <toolbutton.h>
#include <controlpreviewingmanager.h>
#include <saveutils.h>

#include <toolbar.h>
#include <QPainter>
#include <QVBoxLayout>
#include <QMessageBox>

namespace {
    QString findText(qreal ratio);
    qreal roundRatio(qreal ratio);
    qreal findRatio(const QString& text);
}

DesignerWidget::DesignerWidget(QmlCodeEditorWidget* qmlCodeEditorWidget, QWidget *parent) : QWidget(parent)
  , m_lastScale(1.0)
  , m_qmlCodeEditorWidget(qmlCodeEditorWidget)
  , m_layout(new QVBoxLayout(this))
  , m_designerScene(new DesignerScene(this))
  , m_designerView(new DesignerView(m_designerScene))
  , m_toolbar(new ToolBar)
  , m_undoButton(new ToolButton)
  , m_redoButton(new ToolButton)
  , m_clearButton(new ToolButton)
  , m_refreshButton(new ToolButton)
  , m_snappingButton(new ToolButton)
  , m_fitButton(new ToolButton)
  , m_outlineButton(new ToolButton)
  , m_zoomlLevelCombobox(new TransparentComboBox)
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
    m_snappingButton->setCheckable(true);
    m_snappingButton->setChecked(m_designerScene->snapping());

    m_refreshButton->setCursor(Qt::PointingHandCursor);
    m_clearButton->setCursor(Qt::PointingHandCursor);
    m_undoButton->setCursor(Qt::PointingHandCursor);
    m_redoButton->setCursor(Qt::PointingHandCursor);
    m_snappingButton->setCursor(Qt::PointingHandCursor);
    m_outlineButton->setCursor(Qt::PointingHandCursor);
    m_fitButton->setCursor(Qt::PointingHandCursor);
    m_zoomlLevelCombobox->setCursor(Qt::PointingHandCursor);

    m_refreshButton->setToolTip("Refresh control previews on the Dashboard.");
    m_clearButton->setToolTip("Clear controls on the Dashboard.");
    m_undoButton->setToolTip("Undo action.");
    m_redoButton->setToolTip("Redo action.");
    m_snappingButton->setToolTip("Enable snapping to help aligning of controls to each others.");
    m_outlineButton->setToolTip("Show outline frame for controls.");
    m_fitButton->setToolTip("Fit scene into the Dashboard.");
    m_zoomlLevelCombobox->setToolTip("Change zoom level.");

    m_refreshButton->setIcon(Utils::Icons::RELOAD.icon());
    m_clearButton->setIcon(Utils::Icons::CLEAN_TOOLBAR.icon());
    m_undoButton->setIcon(Utils::Icons::UNDO_TOOLBAR.icon());
    m_redoButton->setIcon(Utils::Icons::REDO_TOOLBAR.icon());
    m_snappingButton->setIcon(Utils::Icons::SNAPPING_TOOLBAR.icon());
    m_outlineButton->setIcon(Utils::Icons::BOUNDING_RECT.icon());
    m_fitButton->setIcon(Utils::Icons::FITTOVIEW_TOOLBAR.icon());

    connect(m_snappingButton, SIGNAL(toggled(bool)), SLOT(onSnappingButtonClick(bool)));
    connect(m_outlineButton, SIGNAL(toggled(bool)), SLOT(onOutlineButtonClick(bool)));
    connect(m_zoomlLevelCombobox, SIGNAL(currentTextChanged(QString)), SLOT(onZoomLevelChange(QString)));
    connect(m_fitButton, SIGNAL(clicked(bool)), SLOT(onFitButtonClick()));
    connect(m_refreshButton, SIGNAL(clicked(bool)), SLOT(onRefreshButtonClick()));
    connect(m_clearButton, SIGNAL(clicked(bool)), SLOT(onClearButtonClick()));

    m_toolbar->setFixedHeight(24);
    m_toolbar->addWidget(m_undoButton);
    m_toolbar->addWidget(m_redoButton);
    m_toolbar->addSeparator();
    m_toolbar->addWidget(m_refreshButton);
    m_toolbar->addWidget(m_clearButton);
    m_toolbar->addSeparator();
    m_toolbar->addWidget(m_snappingButton);
    m_toolbar->addWidget(m_outlineButton);
    m_toolbar->addWidget(m_fitButton);
    m_toolbar->addWidget(m_zoomlLevelCombobox);
    m_toolbar->addStretch();
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
    for (auto ratio : ratios)
        if (roundRatio(diff) == ratio)
            m_zoomlLevelCombobox->setCurrentText(findText(ratio));
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

void DesignerWidget::sweep()
{
    m_designerScene->sweep();
    m_designerView->sweep();
    m_outlineButton->setChecked(m_designerScene->showOutlines());
    m_snappingButton->setChecked(m_designerScene->snapping());
    onZoomLevelChange("100 %");
}

QSize DesignerWidget::sizeHint() const
{
    return QSize(680, 680);
}

void DesignerWidget::handleControlDoubleClick(Control* control)
{
    m_qmlCodeEditorWidget->addControl(control);
    m_qmlCodeEditorWidget->setMode(QmlCodeEditorWidget::CodeEditor);
    m_qmlCodeEditorWidget->openControl(control);
}

void DesignerWidget::handleControlDrop(Control* control, const QPointF& pos, const QString& url)
{
    m_designerScene->clearSelection();
    auto newControl = ControlCreationManager::createControl(SaveUtils::toParentDir(url), pos,
            "NULL", control, m_designerScene->currentForm()->dir(), m_designerScene->currentForm()->uid());
    newControl->setSelected(true);
}

void DesignerWidget::onControlSelectionChange(const QList<Control*>& selectedControls)
{
    m_designerScene->clearSelection();
    for (Control* control : selectedControls)
        control->setSelected(true);
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