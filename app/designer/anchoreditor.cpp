#include <anchoreditor.h>
#include <anchorrow.h>
#include <control.h>

#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QDoubleSpinBox>

static const char g_sourceProperty[] = "_q_AnchorEditor_sourceProperty";

static QList<Control*> availableAnchorTargets(Control* source)
{
    QList<Control*> controls(source->siblings());
    if (source->parentControl())
        controls.append(source->parentControl());
    return controls;
}

AnchorEditor::AnchorEditor(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_sourceControlLabel(new QLabel(this))
  , m_marginOffsetLabel(new QLabel(this))
  , m_sourceControlComboBox(new QComboBox(this))
  , m_marginSpinBox(new QDoubleSpinBox(this))
  , m_leftRow(new AnchorRow(AnchorLine::Left, this))
  , m_rightRow(new AnchorRow(AnchorLine::Right, this))
  , m_topRow(new AnchorRow(AnchorLine::Top, this))
  , m_bottomRow(new AnchorRow(AnchorLine::Bottom, this))
  , m_fillRow(new AnchorRow(AnchorLine::Fill, this))
  , m_horizontalCenterRow(new AnchorRow(AnchorLine::HorizontalCenter, this))
  , m_verticalCenterRow(new AnchorRow(AnchorLine::VerticalCenter, this))
  , m_centerInRow(new AnchorRow(AnchorLine::Center, this))
  , m_dialogButtonBox(new QDialogButtonBox(this))
{
    setFocusPolicy(Qt::NoFocus);
    setWindowTitle(tr("Anchor Editor"));
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);

    auto sourceLabel = new QLabel(tr("Source"), this);
    auto targetLabel = new QLabel(tr("Target"), this);
    auto marginLabel = new QLabel(tr("Margin/Offset"), this);
    auto targetControlLabel = new QLabel(tr("Target control"), this);

    QFont font(this->font());
    font.setPixelSize(font.pixelSize() - 1);
    sourceLabel->setFont(font);
    targetLabel->setFont(font);
    marginLabel->setFont(font);
    targetControlLabel->setFont(font);
    sourceLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto hbox = new QHBoxLayout;
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(0);
    hbox->addWidget(sourceLabel);
    hbox->addSpacing(5);
    hbox->addWidget(targetLabel);
    hbox->addSpacing(44);
    hbox->addWidget(marginLabel);
    hbox->addSpacing(5);
    hbox->addWidget(targetControlLabel);
    hbox->addStretch();

    m_layout->setContentsMargins(6, 6, 6, 6);
    m_layout->setSpacing(1);
    m_layout->addLayout(hbox);
    m_layout->addWidget(m_leftRow);
    m_layout->addWidget(m_rightRow);
    m_layout->addWidget(m_topRow);
    m_layout->addWidget(m_bottomRow);
    m_layout->addWidget(m_fillRow);
    m_layout->addWidget(m_horizontalCenterRow);
    m_layout->addWidget(m_verticalCenterRow);
    m_layout->addWidget(m_centerInRow);
    m_layout->addWidget(m_dialogButtonBox, 0, Qt::AlignVCenter | Qt::AlignRight);
    m_layout->addStretch();
    m_layout->setSizeConstraint(QLayout::SetFixedSize);

    m_dialogButtonBox->setFixedHeight(24);
    auto closeButton = m_dialogButtonBox->addButton(QDialogButtonBox::Close);
    closeButton->setToolTip(tr("Close the Anchor Editor"));
    closeButton->setCursor(Qt::PointingHandCursor);
    connect(closeButton, &QPushButton::clicked, this, &AnchorEditor::close);
    auto resetButton = m_dialogButtonBox->addButton(QDialogButtonBox::Reset);
    resetButton->setToolTip(tr("Reset anchors"));
    resetButton->setCursor(Qt::PointingHandCursor);
//    connect(resetButton, &QPushButton::clicked, this, &AnchorEditor::reset);

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
}

void AnchorEditor::activate(Control* source, Control* target)
{
    setProperty(g_sourceProperty, QVariant::fromValue(source));

    m_leftRow->clear();
    m_rightRow->clear();
    m_topRow->clear();
    m_bottomRow->clear();
    m_horizontalCenterRow->clear();
    m_verticalCenterRow->clear();

    m_leftRow->setTargetControlList(availableAnchorTargets(source));
    m_rightRow->setTargetControlList(availableAnchorTargets(source));
    m_topRow->setTargetControlList(availableAnchorTargets(source));
    m_bottomRow->setTargetControlList(availableAnchorTargets(source));
    m_horizontalCenterRow->setTargetControlList(availableAnchorTargets(source));
    m_verticalCenterRow->setTargetControlList(availableAnchorTargets(source));

    m_leftRow->setTargetControl(target);
    m_rightRow->setTargetControl(target);
    m_topRow->setTargetControl(target);
    m_bottomRow->setTargetControl(target);
    m_horizontalCenterRow->setTargetControl(target);
    m_verticalCenterRow->setTargetControl(target);

    if (source->anchors()->left().isValid()) {
        m_leftRow->setTargetLineType(source->anchors()->left().type());
        m_leftRow->setMarginOffset(source->anchors()->leftMargin());
        m_leftRow->setTargetControl(source->anchors()->left().control());
    }

    if (source->anchors()->right().isValid()) {
        m_rightRow->setTargetLineType(source->anchors()->right().type());
        m_rightRow->setMarginOffset(source->anchors()->rightMargin());
        m_rightRow->setTargetControl(source->anchors()->right().control());
    }

    if (source->anchors()->top().isValid()) {
        m_topRow->setTargetLineType(source->anchors()->top().type());
        m_topRow->setMarginOffset(source->anchors()->topMargin());
        m_topRow->setTargetControl(source->anchors()->top().control());
    }

    if (source->anchors()->bottom().isValid()) {
        m_bottomRow->setTargetLineType(source->anchors()->bottom().type());
        m_bottomRow->setMarginOffset(source->anchors()->bottomMargin());
        m_bottomRow->setTargetControl(source->anchors()->bottom().control());
    }

    if (source->anchors()->horizontalCenter().isValid()) {
        m_horizontalCenterRow->setTargetLineType(source->anchors()->horizontalCenter().type());
        m_horizontalCenterRow->setMarginOffset(source->anchors()->horizontalCenterOffset());
        m_horizontalCenterRow->setTargetControl(source->anchors()->horizontalCenter().control());
    }

    if (source->anchors()->verticalCenter().isValid()) {
        m_verticalCenterRow->setTargetLineType(source->anchors()->verticalCenter().type());
        m_verticalCenterRow->setMarginOffset(source->anchors()->verticalCenterOffset());
        m_verticalCenterRow->setTargetControl(source->anchors()->verticalCenter().control());
    }

    show();
}

void AnchorEditor::onMarginOffsetEditingFinish(AnchorRow* row)
{
    emit marginOffsetEdited(AnchorLine(row->sourceLineType(), property(g_sourceProperty).value<Control*>()), row->marginOffset());
}

void AnchorEditor::onTargetControlActivate(AnchorRow* row)
{
    emit anchored(AnchorLine(row->sourceLineType(), property(g_sourceProperty).value<Control*>()),
                  AnchorLine(row->targetLineType(), row->targetControl()));
}

void AnchorEditor::onTargetLineTypeActivate(AnchorRow* row)
{
    emit anchored(AnchorLine(row->sourceLineType(), property(g_sourceProperty).value<Control*>()),
                  AnchorLine(row->targetLineType(), row->targetControl()));
}
