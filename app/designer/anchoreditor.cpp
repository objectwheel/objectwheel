#include <anchoreditor.h>
#include <anchorrow.h>
#include <control.h>
#include <designerscene.h>

#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QCheckBox>

static QList<Control*> availableAnchorTargets(Control* sourceControl)
{
    QList<Control*> controls(sourceControl->siblings());
    if (sourceControl->parentControl())
        controls.append(sourceControl->parentControl());
    return controls;
}

AnchorEditor::AnchorEditor(DesignerScene* scene, QWidget* parent) : QWidget(parent)
  , m_scene(scene)
  , m_refreshScheduled(false)
  , m_layout(new QVBoxLayout(this))
  , m_sourceControlComboBox(new QComboBox(this))
  , m_marginsSpinBox(new QDoubleSpinBox(this))
  , m_leftRow(new AnchorRow(AnchorLine::Left, this))
  , m_rightRow(new AnchorRow(AnchorLine::Right, this))
  , m_topRow(new AnchorRow(AnchorLine::Top, this))
  , m_bottomRow(new AnchorRow(AnchorLine::Bottom, this))
  , m_fillRow(new AnchorRow(AnchorLine::Fill, this))
  , m_horizontalCenterRow(new AnchorRow(AnchorLine::HorizontalCenter, this))
  , m_verticalCenterRow(new AnchorRow(AnchorLine::VerticalCenter, this))
  , m_centerInRow(new AnchorRow(AnchorLine::Center, this))
  , m_alignWhenCenteredCheckBox(new QCheckBox(this))
  , m_dialogButtonBox(new QDialogButtonBox(this))
{
    setFocusPolicy(Qt::NoFocus);
    setWindowTitle(tr("Anchor Editor"));
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);

    auto sourceGroup = new QGroupBox(tr("Source Control"), this);
    auto sourceGroupLayout = new QVBoxLayout(sourceGroup);
    auto sourceControlLayout = new QHBoxLayout;
    auto marginsLayout = new QHBoxLayout;
    auto alignmentLayout = new QHBoxLayout;
    auto sideGroup = new QGroupBox(tr("Side Anchors"), this);
    auto sideGroupLayout = new QVBoxLayout(sideGroup);
    auto categoriesLayout = new QHBoxLayout;
    auto centerGroup = new QGroupBox(tr("Center Anchors"), this);
    auto centerGroupLayout = new QVBoxLayout(centerGroup);
    auto categoriesLayout2 = new QHBoxLayout;
    auto sourceControlLabel = new QLabel(tr("Source control: "), this);
    auto marginsLabel = new QLabel(tr("Margins:"), this);
    auto alignmentLabel = new QLabel(tr("Pixel alignment: "), this);
    auto sourceLineLabel = new QLabel(tr("Line"), this);
    auto targetControlLabel = new QLabel(tr("Target control"), this);
    auto marginLabel = new QLabel(tr("Margin/Offset"), this);
    auto targetLineLabel = new QLabel(tr("Target line"), this);
    auto sourceLineLabel2 = new QLabel(tr("Line"), this);
    auto targetControlLabel2 = new QLabel(tr("Target control"), this);
    auto marginLabel2 = new QLabel(tr("Margin/Offset"), this);
    auto targetLineLabel2 = new QLabel(tr("Target line"), this);
    auto closeButton = m_dialogButtonBox->addButton(QDialogButtonBox::Close);
    auto clearButton = m_dialogButtonBox->addButton(QDialogButtonBox::Reset);

    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->setSpacing(8);
    m_layout->addWidget(sourceGroup);
    m_layout->addWidget(sideGroup);
    m_layout->addWidget(centerGroup);
    m_layout->addWidget(m_dialogButtonBox, 0, Qt::AlignVCenter | Qt::AlignRight);
    m_layout->addStretch();

    sourceGroupLayout->setContentsMargins(4, 4, 4, 4);
    sourceGroupLayout->setSpacing(1);
    sourceGroupLayout->addLayout(sourceControlLayout);
    sourceGroupLayout->addLayout(marginsLayout);
    sourceGroupLayout->addLayout(alignmentLayout);

    sourceControlLayout->setContentsMargins(0, 0, 0, 0);
    sourceControlLayout->setSpacing(2);
    sourceControlLayout->addWidget(sourceControlLabel);
    sourceControlLayout->addSpacing(10);
    sourceControlLayout->addWidget(m_sourceControlComboBox);
    sourceControlLayout->addStretch();

    m_sourceControlComboBox->setCursor(Qt::PointingHandCursor);
    m_sourceControlComboBox->setFixedSize(QSize(140, 24));
    m_sourceControlComboBox->setToolTip(tr("Source control"));

    marginsLayout->setContentsMargins(0, 0, 0, 0);
    marginsLayout->setSpacing(2);
    marginsLayout->addWidget(marginsLabel);
    marginsLayout->addSpacing(46);
    marginsLayout->addWidget(m_marginsSpinBox);
    marginsLayout->addStretch();

    m_marginsSpinBox->setToolTip(tr("Generic margins"));
    m_marginsSpinBox->setCursor(Qt::PointingHandCursor);
    m_marginsSpinBox->setFixedSize(QSize(80, 24));
    m_marginsSpinBox->setRange(-999.99, 999.99);
    m_marginsSpinBox->setDecimals(2);

    alignmentLayout->setContentsMargins(0, 0, 0, 0);
    alignmentLayout->setSpacing(2);
    alignmentLayout->addWidget(alignmentLabel);
    alignmentLayout->addSpacing(5);
    alignmentLayout->addWidget(m_alignWhenCenteredCheckBox);
    alignmentLayout->addStretch();

    m_alignWhenCenteredCheckBox->setCursor(Qt::PointingHandCursor);
    m_alignWhenCenteredCheckBox->setFixedHeight(24);
    m_alignWhenCenteredCheckBox->setChecked(true);
    m_alignWhenCenteredCheckBox->setText(tr("Align when centered"));
    m_alignWhenCenteredCheckBox->setToolTip(tr("This forces centered anchors to align to a whole "
                                               "pixel; (enabled by default) if the item being "
                                               "centered has an odd width or height, the item will "
                                               "be positioned on a whole pixel rather than being "
                                               "placed on a half-pixel. This ensures the item is "
                                               "painted crisply. There are cases where this is not "
                                               "desirable, for example when rotating the item "
                                               "jitters may be apparent as the center is rounded."));

    sideGroupLayout->setContentsMargins(4, 4, 4, 4);
    sideGroupLayout->setSpacing(0);
    sideGroupLayout->addLayout(categoriesLayout);
    sideGroupLayout->addWidget(m_leftRow);
    sideGroupLayout->addWidget(m_rightRow);
    sideGroupLayout->addWidget(m_topRow);
    sideGroupLayout->addWidget(m_bottomRow);
    sideGroupLayout->addWidget(m_fillRow);

    categoriesLayout->setContentsMargins(0, 0, 0, 0);
    categoriesLayout->setSpacing(0);
    categoriesLayout->addWidget(sourceLineLabel);
    categoriesLayout->addSpacing(26);
    categoriesLayout->addWidget(targetControlLabel);
    categoriesLayout->addSpacing(70);
    categoriesLayout->addWidget(marginLabel);
    categoriesLayout->addSpacing(7);
    categoriesLayout->addWidget(targetLineLabel);
    categoriesLayout->addStretch();

    centerGroupLayout->setContentsMargins(4, 4, 4, 4);
    centerGroupLayout->setSpacing(0);
    centerGroupLayout->addLayout(categoriesLayout2);
    centerGroupLayout->addWidget(m_horizontalCenterRow);
    centerGroupLayout->addWidget(m_verticalCenterRow);
    centerGroupLayout->addWidget(m_centerInRow);

    categoriesLayout2->setContentsMargins(0, 0, 0, 0);
    categoriesLayout2->setSpacing(0);
    categoriesLayout2->addWidget(sourceLineLabel2);
    categoriesLayout2->addSpacing(26);
    categoriesLayout2->addWidget(targetControlLabel2);
    categoriesLayout2->addSpacing(70);
    categoriesLayout2->addWidget(marginLabel2);
    categoriesLayout2->addSpacing(7);
    categoriesLayout2->addWidget(targetLineLabel2);
    categoriesLayout2->addStretch();

    m_dialogButtonBox->setFixedHeight(24);

    closeButton->setToolTip(tr("Close the Anchor Editor"));
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setFocus();

    clearButton->setText(tr("Clear"));
    clearButton->setToolTip(tr("Clear anchors"));
    clearButton->setCursor(Qt::PointingHandCursor);

    connect(closeButton, &QPushButton::clicked,
            this, &AnchorEditor::close);
    connect(clearButton, &QPushButton::clicked,
            this, &AnchorEditor::cleared);
    connect(m_leftRow, &AnchorRow::marginOffsetEditingFinished,
            this, [=] { onMarginOffsetEditingFinish(m_leftRow); });
    connect(m_rightRow, &AnchorRow::marginOffsetEditingFinished,
            this, [=] { onMarginOffsetEditingFinish(m_rightRow); });
    connect(m_topRow, &AnchorRow::marginOffsetEditingFinished,
            this, [=] { onMarginOffsetEditingFinish(m_topRow); });
    connect(m_bottomRow, &AnchorRow::marginOffsetEditingFinished,
            this, [=] { onMarginOffsetEditingFinish(m_bottomRow); });
    connect(m_horizontalCenterRow, &AnchorRow::marginOffsetEditingFinished,
            this, [=] { onMarginOffsetEditingFinish(m_horizontalCenterRow); });
    connect(m_verticalCenterRow, &AnchorRow::marginOffsetEditingFinished,
            this, [=] { onMarginOffsetEditingFinish(m_verticalCenterRow); });
    connect(m_leftRow, &AnchorRow::targetControlActivated,
            this, [=] { onTargetControlActivate(m_leftRow); });
    connect(m_rightRow, &AnchorRow::targetControlActivated,
            this, [=] { onTargetControlActivate(m_rightRow); });
    connect(m_topRow, &AnchorRow::targetControlActivated,
            this, [=] { onTargetControlActivate(m_topRow); });
    connect(m_bottomRow, &AnchorRow::targetControlActivated,
            this, [=] { onTargetControlActivate(m_bottomRow); });
    connect(m_horizontalCenterRow, &AnchorRow::targetControlActivated,
            this, [=] { onTargetControlActivate(m_horizontalCenterRow); });
    connect(m_verticalCenterRow, &AnchorRow::targetControlActivated,
            this, [=] { onTargetControlActivate(m_verticalCenterRow); });
    connect(m_leftRow, &AnchorRow::targetLineTypeActivated,
            this, [=] { onTargetLineTypeActivate(m_leftRow); });
    connect(m_rightRow, &AnchorRow::targetLineTypeActivated,
            this, [=] { onTargetLineTypeActivate(m_rightRow); });
    connect(m_topRow, &AnchorRow::targetLineTypeActivated,
            this, [=] { onTargetLineTypeActivate(m_topRow); });
    connect(m_bottomRow, &AnchorRow::targetLineTypeActivated,
            this, [=] { onTargetLineTypeActivate(m_bottomRow); });
    connect(m_horizontalCenterRow, &AnchorRow::targetLineTypeActivated,
            this, [=] { onTargetLineTypeActivate(m_horizontalCenterRow); });
    connect(m_verticalCenterRow, &AnchorRow::targetLineTypeActivated,
            this, [=] { onTargetLineTypeActivate(m_verticalCenterRow); });

    connect(m_fillRow, &AnchorRow::sourceButtonClicked, this, [=] (bool checked) {
        m_leftRow->setFillCenterModeEnabled(checked, m_fillRow->targetControl());
        m_rightRow->setFillCenterModeEnabled(checked, m_fillRow->targetControl());
        m_topRow->setFillCenterModeEnabled(checked, m_fillRow->targetControl());
        m_bottomRow->setFillCenterModeEnabled(checked, m_fillRow->targetControl());
    });
}

Control* AnchorEditor::sourceControl() const
{
    return m_sourceControl;
}

void AnchorEditor::setSourceControl(Control* sourceControl)
{
    if (m_sourceControl != sourceControl) {
        m_sourceControl = sourceControl;
        refresh();
    }
}

Control* AnchorEditor::primaryTargetControl() const
{
    return m_primaryTargetControl;
}

void AnchorEditor::setPrimaryTargetControl(Control* primaryTargetControl)
{
    if (m_primaryTargetControl != primaryTargetControl) {
        m_primaryTargetControl = primaryTargetControl;
        refresh();
    }
}

void AnchorEditor::onMarginOffsetEditingFinish(AnchorRow* row)
{
    emit marginOffsetEdited(AnchorLine(row->sourceLineType(), m_sourceControl), row->marginOffset());
}

void AnchorEditor::onTargetControlActivate(AnchorRow* row)
{
    emit anchored(AnchorLine(row->sourceLineType(), m_sourceControl),
                  AnchorLine(row->targetLineType(), row->targetControl()));
}

void AnchorEditor::onTargetLineTypeActivate(AnchorRow* row)
{
    emit anchored(AnchorLine(row->sourceLineType(), m_sourceControl),
                  AnchorLine(row->targetLineType(), row->targetControl()));
}

void AnchorEditor::refresh(bool delayed)
{
    if (delayed) {
        if (!m_refreshScheduled) {
            m_refreshScheduled = true;
            QMetaObject::invokeMethod(this, [=] {
                refreshNow();
                m_refreshScheduled = false;
            }, Qt::QueuedConnection);
        }
    } else {
        refreshNow();
    }
}

void AnchorEditor::refreshNow()
{
    m_sourceControlComboBox->clear();
    for (Control* control : m_scene->items<Control>()) {
        if (control->gui())
            m_sourceControlComboBox->addItem(control->id());
    }

    m_leftRow->clear();
    m_rightRow->clear();
    m_topRow->clear();
    m_bottomRow->clear();
    m_horizontalCenterRow->clear();
    m_verticalCenterRow->clear();
    m_fillRow->clear();

    m_leftRow->setTargetControlList(availableAnchorTargets(sourceControl));
    m_rightRow->setTargetControlList(availableAnchorTargets(sourceControl));
    m_topRow->setTargetControlList(availableAnchorTargets(sourceControl));
    m_bottomRow->setTargetControlList(availableAnchorTargets(sourceControl));
    m_horizontalCenterRow->setTargetControlList(availableAnchorTargets(sourceControl));
    m_verticalCenterRow->setTargetControlList(availableAnchorTargets(sourceControl));
    m_fillRow->setTargetControlList(availableAnchorTargets(sourceControl));

    m_leftRow->setTargetControl(m_primaryTargetControl);
    m_rightRow->setTargetControl(m_primaryTargetControl);
    m_topRow->setTargetControl(m_primaryTargetControl);
    m_bottomRow->setTargetControl(m_primaryTargetControl);
    m_horizontalCenterRow->setTargetControl(m_primaryTargetControl);
    m_verticalCenterRow->setTargetControl(m_primaryTargetControl);
    m_fillRow->setTargetControl(m_primaryTargetControl);

    if (sourceControl->anchors()->left().isValid()) {
        m_leftRow->setTargetLineType(sourceControl->anchors()->left().type());
        m_leftRow->setMarginOffset(sourceControl->anchors()->leftMargin());
        m_leftRow->setTargetControl(sourceControl->anchors()->left().control());
    }

    if (sourceControl->anchors()->right().isValid()) {
        m_rightRow->setTargetLineType(sourceControl->anchors()->right().type());
        m_rightRow->setMarginOffset(sourceControl->anchors()->rightMargin());
        m_rightRow->setTargetControl(sourceControl->anchors()->right().control());
    }

    if (sourceControl->anchors()->top().isValid()) {
        m_topRow->setTargetLineType(sourceControl->anchors()->top().type());
        m_topRow->setMarginOffset(sourceControl->anchors()->topMargin());
        m_topRow->setTargetControl(sourceControl->anchors()->top().control());
    }

    if (sourceControl->anchors()->bottom().isValid()) {
        m_bottomRow->setTargetLineType(sourceControl->anchors()->bottom().type());
        m_bottomRow->setMarginOffset(sourceControl->anchors()->bottomMargin());
        m_bottomRow->setTargetControl(sourceControl->anchors()->bottom().control());
    }

    if (sourceControl->anchors()->horizontalCenter().isValid()) {
        m_horizontalCenterRow->setTargetLineType(sourceControl->anchors()->horizontalCenter().type());
        m_horizontalCenterRow->setMarginOffset(sourceControl->anchors()->horizontalCenterOffset());
        m_horizontalCenterRow->setTargetControl(sourceControl->anchors()->horizontalCenter().control());
    }

    if (sourceControl->anchors()->verticalCenter().isValid()) {
        m_verticalCenterRow->setTargetLineType(sourceControl->anchors()->verticalCenter().type());
        m_verticalCenterRow->setMarginOffset(sourceControl->anchors()->verticalCenterOffset());
        m_verticalCenterRow->setTargetControl(sourceControl->anchors()->verticalCenter().control());
    }

    if (sourceControl->anchors()->fill()) {
        m_fillRow->setSourceButtonChecked(true);
        m_fillRow->setTargetControl(sourceControl->anchors()->fill());
        m_leftRow->setFillCenterModeEnabled(true, sourceControl->anchors()->fill());
        m_rightRow->setFillCenterModeEnabled(true, sourceControl->anchors()->fill());
        m_topRow->setFillCenterModeEnabled(true, sourceControl->anchors()->fill());
        m_bottomRow->setFillCenterModeEnabled(true, sourceControl->anchors()->fill());
    }
}
