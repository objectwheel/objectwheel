#include <designerwidget.h>
#include <fit.h>
#include <css.h>
#include <designerview.h>
#include <designerscene.h>
#include <qmlcodeeditorwidget.h>
#include <savebackend.h>
#include <controlwatcher.h>
#include <exposerbackend.h>
#include <filemanager.h>

#include <QToolBar>
#include <QToolButton>
#include <QComboBox>
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

    m_refreshButton->setIcon(QIcon(":/resources/images/refresh.png"));
    m_clearButton->setIcon(QIcon(":/resources/images/clean.png"));
    m_undoButton->setIcon(QIcon(":/resources/images/undo.png"));
    m_redoButton->setIcon(QIcon(":/resources/images/redo.png"));
    m_snappingButton->setIcon(QIcon(":/resources/images/snap.png"));
    m_outlineButton->setIcon(QIcon(":/resources/images/outline.png"));
    m_fitButton->setIcon(QIcon(":/resources/images/fit.png"));

    connect(m_snappingButton, SIGNAL(toggled(bool)), SLOT(onSnappingButtonClick(bool)));
    connect(m_outlineButton, SIGNAL(toggled(bool)), SLOT(onOutlineButtonClick(bool)));
    connect(m_zoomlLevelCombobox, SIGNAL(currentTextChanged(QString)), SLOT(onZoomLevelChange(QString)));
    connect(m_fitButton, SIGNAL(clicked(bool)), SLOT(onFitButtonClick()));
    connect(m_refreshButton, SIGNAL(clicked(bool)), SLOT(onRefreshButtonClick()));
    connect(m_clearButton, SIGNAL(clicked(bool)), SLOT(onClearButtonClick()));

    m_toolbar->setStyleSheet(CSS::DesignerToolbar);
    m_toolbar->setFixedHeight(fit::fx(21));
    m_toolbar->setIconSize(QSize(fit::fx(14), fit::fx(14)));
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

    connect(cW, SIGNAL(doubleClicked(Control*)), SLOT(onControlDoubleClick(Control*)));
    connect(cW, SIGNAL(controlDropped(Control*,QPointF,QString)), SLOT(onControlDrop(Control*,QPointF,QString)));
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
            m_designerScene->removeChildControlsOnly(m_designerScene->mainForm());
            SaveBackend::instance()->removeChildControlsOnly(m_designerScene->mainForm());
            break;
        } default: {
            // Do nothing
            break;
        }
    }
}

void DesignerWidget::reset()
{
    m_designerScene->reset();
    m_designerView->reset();
    m_outlineButton->setChecked(m_designerScene->showOutlines());
    m_snappingButton->setChecked(m_designerScene->snapping());
    onZoomLevelChange("100 %");
}

QSize DesignerWidget::sizeHint() const
{
    return fit::fx(QSizeF(680, 680)).toSize();
}

void DesignerWidget::onControlClick(Control* control)
{
    m_designerScene->clearSelection();
    control->setSelected(true);
}

void DesignerWidget::onControlDoubleClick(Control* control)
{
    m_qmlCodeEditorWidget->addControl(control);
    m_qmlCodeEditorWidget->setMode(QmlCodeEditorWidget::CodeEditor);
    m_qmlCodeEditorWidget->openControl(control);
}

void DesignerWidget::onControlDrop(Control* control, const QPointF& pos, const QString& url)
{
    m_designerScene->clearSelection();
    auto newControl = ExposerBackend::instance()->exposeControl(dname(dname(url)), pos, "NULL", control, m_designerScene->mainForm()->dir(), m_designerScene->mainForm()->uid());
    newControl->setSelected(true);
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