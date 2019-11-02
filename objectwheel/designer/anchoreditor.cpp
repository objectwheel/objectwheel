#include <anchoreditor.h>
#include <anchorrow.h>
#include <designerscene.h>
#include <control.h>

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
    for (int i = controls.size() - 1; i >= 0; --i) {
        if (!DesignerScene::isAnchorViable(sourceControl, controls.at(i)))
            controls.removeAt(i);
    }
    return controls;
}

AnchorEditor::AnchorEditor(DesignerScene* scene, QWidget* parent) : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint)
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
    m_sourceControlComboBox->setFixedSize(QSize(140, 22));
    m_sourceControlComboBox->setToolTip(tr("Source control"));

    marginsLayout->setContentsMargins(0, 0, 0, 0);
    marginsLayout->setSpacing(2);
    marginsLayout->addWidget(marginsLabel);
    marginsLayout->addSpacing(46);
    marginsLayout->addWidget(m_marginsSpinBox);
    marginsLayout->addStretch();

    m_marginsSpinBox->setToolTip(tr("Generic margins for left, right, top and bottom sides. It "
                                    "can be overridden via setting individual margins from below."));
    m_marginsSpinBox->setCursor(Qt::PointingHandCursor);
    m_marginsSpinBox->setFixedSize(QSize(80, 22));
    m_marginsSpinBox->setRange(-999.99, 999.99);
    m_marginsSpinBox->setDecimals(2);

    alignmentLayout->setContentsMargins(0, 0, 0, 0);
    alignmentLayout->setSpacing(2);
    alignmentLayout->addWidget(alignmentLabel);
    alignmentLayout->addSpacing(5);
    alignmentLayout->addWidget(m_alignWhenCenteredCheckBox);
    alignmentLayout->addStretch();

    m_alignWhenCenteredCheckBox->setCursor(Qt::PointingHandCursor);
    m_alignWhenCenteredCheckBox->setFixedHeight(22);
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
    categoriesLayout->addSpacing(68);
    categoriesLayout->addWidget(targetLineLabel);
    categoriesLayout->addSpacing(23);
    categoriesLayout->addWidget(marginLabel);
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
    categoriesLayout2->addSpacing(68);
    categoriesLayout2->addWidget(targetLineLabel2);
    categoriesLayout2->addSpacing(23);
    categoriesLayout2->addWidget(marginLabel2);
    categoriesLayout2->addStretch();

    m_dialogButtonBox->setFixedHeight(22);

    closeButton->setToolTip(tr("Close the Anchor Editor"));
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setFocusPolicy(Qt::NoFocus);

    clearButton->setText(tr("Clear"));
    clearButton->setToolTip(tr("Clear anchors"));
    clearButton->setCursor(Qt::PointingHandCursor);
    clearButton->setFocusPolicy(Qt::NoFocus);

    connect(m_sourceControlComboBox, qOverload<int>(&QComboBox::activated), this, [=] (int index) {
        setSourceControl(m_sourceControlComboBox->itemData(index).value<Control*>());
        refresh();
    });
    connect(m_sourceControlComboBox, qOverload<int>(&QComboBox::activated),
            this, &AnchorEditor::sourceControlActivated);
    connect(m_alignWhenCenteredCheckBox, &QCheckBox::clicked,
            this, &AnchorEditor::alignmentActivated);
    connect(m_marginsSpinBox, &QDoubleSpinBox::editingFinished,
            this, [=] { emit marginsEdited(m_marginsSpinBox->value()); });
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
        emit filled(checked ? m_fillRow->targetControl() : nullptr);
    });
    connect(m_fillRow, &AnchorRow::targetControlActivated, this, [=] {
        m_leftRow->setTargetControl(m_fillRow->targetControl());
        m_rightRow->setTargetControl(m_fillRow->targetControl());
        m_topRow->setTargetControl(m_fillRow->targetControl());
        m_bottomRow->setTargetControl(m_fillRow->targetControl());
        emit filled(m_fillRow->targetControl());
    });
    connect(m_centerInRow, &AnchorRow::sourceButtonClicked, this, [=] (bool checked) {
        m_horizontalCenterRow->setFillCenterModeEnabled(checked, m_centerInRow->targetControl());
        m_verticalCenterRow->setFillCenterModeEnabled(checked, m_centerInRow->targetControl());
        if (checked) {
            emit centered(m_centerInRow->targetControl(),
                          m_centerInRow->popupModeEnabled() && m_centerInRow->targetControl() == 0);
        } else {
            emit centered(nullptr, false);
        }
    });
    connect(m_centerInRow, &AnchorRow::targetControlActivated, this, [=] {
        m_horizontalCenterRow->setTargetControl(m_centerInRow->targetControl());
        m_verticalCenterRow->setTargetControl(m_centerInRow->targetControl());
        emit centered(m_centerInRow->targetControl(),
                      m_centerInRow->popupModeEnabled() && m_centerInRow->targetControl() == 0);
    });
    connect(clearButton, &QPushButton::clicked,
            this, &AnchorEditor::clear);
    connect(clearButton, &QPushButton::clicked,
            this, &AnchorEditor::cleared);
    connect(closeButton, &QPushButton::clicked,
            this, &AnchorEditor::close);
}

Control* AnchorEditor::sourceControl() const
{
    return m_sourceControl;
}

void AnchorEditor::setSourceControl(Control* sourceControl)
{
    if (m_sourceControl != sourceControl)
        m_sourceControl = sourceControl;
}

Control* AnchorEditor::primaryTargetControl() const
{
    return m_primaryTargetControl;
}

void AnchorEditor::setPrimaryTargetControl(Control* primaryTargetControl)
{
    if (m_primaryTargetControl != primaryTargetControl)
        m_primaryTargetControl = primaryTargetControl;
}

void AnchorEditor::onMarginOffsetEditingFinish(AnchorRow* row)
{
    emit marginOffsetEdited(row->sourceLineType(), row->marginOffset());
}

void AnchorEditor::onTargetControlActivate(AnchorRow* row)
{
    emit anchored(row->sourceLineType(), AnchorLine(row->targetLineType(), row->targetControl()));
}

void AnchorEditor::onTargetLineTypeActivate(AnchorRow* row)
{
    if (row->targetLineType() == AnchorLine::Invalid)
        emit marginOffsetEdited(row->sourceLineType(), row->marginOffset());
    emit anchored(row->sourceLineType(), AnchorLine(row->targetLineType(), row->targetControl()));
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

void AnchorEditor::clear()
{
    m_marginsSpinBox->setValue(0);
    m_alignWhenCenteredCheckBox->setChecked(true);

    m_leftRow->clear();
    m_rightRow->clear();
    m_topRow->clear();
    m_bottomRow->clear();
    m_fillRow->clear();
    m_horizontalCenterRow->clear();
    m_verticalCenterRow->clear();
    m_centerInRow->clear();

    const QList<Control*>& anchorTargets = availableAnchorTargets(m_sourceControl);
    m_leftRow->setTargetControlList(anchorTargets);
    m_rightRow->setTargetControlList(anchorTargets);
    m_topRow->setTargetControlList(anchorTargets);
    m_bottomRow->setTargetControlList(anchorTargets);
    m_fillRow->setTargetControlList(anchorTargets);
    m_horizontalCenterRow->setTargetControlList(anchorTargets);
    m_verticalCenterRow->setTargetControlList(anchorTargets);
    m_centerInRow->setTargetControlList(anchorTargets);

    Control* primaryTargetControl = m_primaryTargetControl;
    if (!anchorTargets.contains(primaryTargetControl))
        primaryTargetControl = m_sourceControl->parentControl();

    m_leftRow->setTargetControl(primaryTargetControl);
    m_rightRow->setTargetControl(primaryTargetControl);
    m_topRow->setTargetControl(primaryTargetControl);
    m_bottomRow->setTargetControl(primaryTargetControl);
    m_fillRow->setTargetControl(primaryTargetControl);
    m_horizontalCenterRow->setTargetControl(primaryTargetControl);
    m_verticalCenterRow->setTargetControl(primaryTargetControl);
    m_centerInRow->setPopupModeEnabled(m_sourceControl);
    m_centerInRow->setTargetControl(primaryTargetControl, false);
}

void AnchorEditor::refreshNow()
{
    m_sourceControlComboBox->clear();
    for (Control* control : m_scene->items<Control>()) {
        if (!DesignerScene::isInappropriateAnchorSource(control))
            m_sourceControlComboBox->addItem(control->id(), QVariant::fromValue(control));
    }
    m_sourceControlComboBox->setCurrentText(m_sourceControl->id());
    m_marginsSpinBox->setValue(m_sourceControl->anchors()->margins());
    m_alignWhenCenteredCheckBox->setChecked(m_sourceControl->anchors()->alignWhenCentered());

    m_leftRow->clear();
    m_rightRow->clear();
    m_topRow->clear();
    m_bottomRow->clear();
    m_fillRow->clear();
    m_horizontalCenterRow->clear();
    m_verticalCenterRow->clear();
    m_centerInRow->clear();

    const QList<Control*>& anchorTargets = availableAnchorTargets(m_sourceControl);
    m_leftRow->setTargetControlList(anchorTargets);
    m_rightRow->setTargetControlList(anchorTargets);
    m_topRow->setTargetControlList(anchorTargets);
    m_bottomRow->setTargetControlList(anchorTargets);
    m_fillRow->setTargetControlList(anchorTargets);
    m_horizontalCenterRow->setTargetControlList(anchorTargets);
    m_verticalCenterRow->setTargetControlList(anchorTargets);
    m_centerInRow->setTargetControlList(anchorTargets);

    Control* primaryTargetControl = m_primaryTargetControl;
    if (!anchorTargets.contains(primaryTargetControl))
        primaryTargetControl = m_sourceControl->parentControl();

    m_leftRow->setTargetControl(primaryTargetControl);
    m_rightRow->setTargetControl(primaryTargetControl);
    m_topRow->setTargetControl(primaryTargetControl);
    m_bottomRow->setTargetControl(primaryTargetControl);
    m_fillRow->setTargetControl(primaryTargetControl);
    m_horizontalCenterRow->setTargetControl(primaryTargetControl);
    m_verticalCenterRow->setTargetControl(primaryTargetControl);
    m_centerInRow->setTargetControl(primaryTargetControl);

    m_leftRow->setMarginOffset(m_sourceControl->anchors()->leftMargin());
    m_rightRow->setMarginOffset(m_sourceControl->anchors()->rightMargin());
    m_topRow->setMarginOffset(m_sourceControl->anchors()->topMargin());
    m_bottomRow->setMarginOffset(m_sourceControl->anchors()->bottomMargin());
    m_horizontalCenterRow->setMarginOffset(m_sourceControl->anchors()->horizontalCenterOffset());
    m_verticalCenterRow->setMarginOffset(m_sourceControl->anchors()->verticalCenterOffset());

    if (m_sourceControl->anchors()->left().isValid()) {
        m_leftRow->setTargetControl(m_sourceControl->anchors()->left().control());
        m_leftRow->setTargetLineType(m_sourceControl->anchors()->left().type());
    }

    if (m_sourceControl->anchors()->right().isValid()) {
        m_rightRow->setTargetControl(m_sourceControl->anchors()->right().control());
        m_rightRow->setTargetLineType(m_sourceControl->anchors()->right().type());
    }

    if (m_sourceControl->anchors()->top().isValid()) {
        m_topRow->setTargetControl(m_sourceControl->anchors()->top().control());
        m_topRow->setTargetLineType(m_sourceControl->anchors()->top().type());
    }

    if (m_sourceControl->anchors()->bottom().isValid()) {
        m_bottomRow->setTargetControl(m_sourceControl->anchors()->bottom().control());
        m_bottomRow->setTargetLineType(m_sourceControl->anchors()->bottom().type());
    }

    if (m_sourceControl->anchors()->fill()) {
        m_fillRow->setSourceButtonChecked(true);
        m_fillRow->setTargetControl(m_sourceControl->anchors()->fill());
        m_leftRow->setFillCenterModeEnabled(true, m_sourceControl->anchors()->fill());
        m_rightRow->setFillCenterModeEnabled(true, m_sourceControl->anchors()->fill());
        m_topRow->setFillCenterModeEnabled(true, m_sourceControl->anchors()->fill());
        m_bottomRow->setFillCenterModeEnabled(true, m_sourceControl->anchors()->fill());
    }

    if (m_sourceControl->anchors()->horizontalCenter().isValid()) {
        m_horizontalCenterRow->setTargetControl(m_sourceControl->anchors()->horizontalCenter().control());
        m_horizontalCenterRow->setTargetLineType(m_sourceControl->anchors()->horizontalCenter().type());
    }

    if (m_sourceControl->anchors()->verticalCenter().isValid()) {
        m_verticalCenterRow->setTargetControl(m_sourceControl->anchors()->verticalCenter().control());
        m_verticalCenterRow->setTargetLineType(m_sourceControl->anchors()->verticalCenter().type());
    }

    m_centerInRow->setPopupModeEnabled(m_sourceControl);
    if (m_sourceControl->anchors()->centerIn()) {
        m_centerInRow->setSourceButtonChecked(true);
        m_centerInRow->setTargetControl(m_sourceControl->anchors()->centerIn(), m_sourceControl->overlayPopup());
        m_horizontalCenterRow->setFillCenterModeEnabled(true, m_sourceControl->anchors()->centerIn());
        m_verticalCenterRow->setFillCenterModeEnabled(true, m_sourceControl->anchors()->centerIn());
    }

    m_leftRow->setEnabled(!m_centerInRow->popupModeEnabled());
    m_rightRow->setEnabled(!m_centerInRow->popupModeEnabled());
    m_topRow->setEnabled(!m_centerInRow->popupModeEnabled());
    m_bottomRow->setEnabled(!m_centerInRow->popupModeEnabled());
    m_fillRow->setEnabled(!m_centerInRow->popupModeEnabled());
    m_horizontalCenterRow->setEnabled(!m_centerInRow->popupModeEnabled());
    m_verticalCenterRow->setEnabled(!m_centerInRow->popupModeEnabled());
    m_marginsSpinBox->setEnabled(!m_centerInRow->popupModeEnabled());
    m_alignWhenCenteredCheckBox->setEnabled(!m_centerInRow->popupModeEnabled());
}
