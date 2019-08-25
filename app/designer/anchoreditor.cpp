#include <anchoreditor.h>
#include <anchorrow.h>
#include <control.h>

#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>

static QList<Control*> availableAnchorTargets(Control* source)
{
    QList<Control*> controls(source->siblings());
    if (source->parentControl())
        controls.append(source->parentControl());
    return controls;
}

AnchorEditor::AnchorEditor(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_leftRow(new AnchorRow(this))
  , m_rightRow(new AnchorRow(this))
  , m_topRow(new AnchorRow(this))
  , m_bottomRow(new AnchorRow(this))
  , m_fillRow(new AnchorRow(this))
  , m_horizontalCenterRow(new AnchorRow(this))
  , m_verticalCenterRow(new AnchorRow(this))
  , m_centerRow(new AnchorRow(this))
  , m_closeButton(new QPushButton(this))
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
    m_layout->addWidget(m_centerRow);
    m_layout->addWidget(m_closeButton, 0, Qt::AlignVCenter | Qt::AlignRight);
    m_layout->addStretch();
    m_layout->setSizeConstraint(QLayout::SetFixedSize);

    m_leftRow->setSourceLineType(AnchorLine::Left);
    m_rightRow->setSourceLineType(AnchorLine::Right);
    m_topRow->setSourceLineType(AnchorLine::Top);
    m_bottomRow->setSourceLineType(AnchorLine::Bottom);
    m_fillRow->setSourceLineType(AnchorLine::Fill);
    m_horizontalCenterRow->setSourceLineType(AnchorLine::HorizontalCenter);
    m_verticalCenterRow->setSourceLineType(AnchorLine::VerticalCenter);
    m_centerRow->setSourceLineType(AnchorLine::Center);

    m_closeButton->setFixedHeight(24);
    m_closeButton->setText(tr("Close"));
    m_closeButton->setToolTip(tr("Close the Anchor Editor"));
    m_closeButton->setCursor(Qt::PointingHandCursor);
    connect(m_closeButton, &QPushButton::clicked, this, &AnchorEditor::close);
}

void AnchorEditor::activate(Control* source, Control* target)
{
    m_leftRow->clear();
    m_rightRow->clear();
    m_topRow->clear();
    m_bottomRow->clear();
    m_fillRow->clear();
    m_horizontalCenterRow->clear();
    m_verticalCenterRow->clear();
    m_centerRow->clear();

    m_leftRow->setTargetControlList(availableAnchorTargets(source));
    m_rightRow->setTargetControlList(availableAnchorTargets(source));
    m_topRow->setTargetControlList(availableAnchorTargets(source));
    m_bottomRow->setTargetControlList(availableAnchorTargets(source));
    m_horizontalCenterRow->setTargetControlList(availableAnchorTargets(source));
    m_verticalCenterRow->setTargetControlList(availableAnchorTargets(source));

    m_leftRow->setCurrentTargetControl(target);
    m_rightRow->setCurrentTargetControl(target);
    m_topRow->setCurrentTargetControl(target);
    m_bottomRow->setCurrentTargetControl(target);
    m_horizontalCenterRow->setCurrentTargetControl(target);
    m_verticalCenterRow->setCurrentTargetControl(target);

    if (source->anchors()->left().isValid()) {
        m_leftRow->setActive(true);
        m_leftRow->setTargetLineType(source->anchors()->left().type());
        m_leftRow->setMarginOffset(source->anchors()->leftMargin());
        m_leftRow->setCurrentTargetControl(source->anchors()->left().control());
    }

    if (source->anchors()->right().isValid()) {
        m_rightRow->setActive(true);
        m_rightRow->setTargetLineType(source->anchors()->right().type());
        m_rightRow->setMarginOffset(source->anchors()->rightMargin());
        m_rightRow->setCurrentTargetControl(source->anchors()->right().control());
    }

    if (source->anchors()->top().isValid()) {
        m_topRow->setActive(true);
        m_topRow->setTargetLineType(source->anchors()->top().type());
        m_topRow->setMarginOffset(source->anchors()->topMargin());
        m_topRow->setCurrentTargetControl(source->anchors()->top().control());
    }

    if (source->anchors()->bottom().isValid()) {
        m_bottomRow->setActive(true);
        m_bottomRow->setTargetLineType(source->anchors()->bottom().type());
        m_bottomRow->setMarginOffset(source->anchors()->bottomMargin());
        m_bottomRow->setCurrentTargetControl(source->anchors()->bottom().control());
    }

    if (source->anchors()->horizontalCenter().isValid()) {
        m_horizontalCenterRow->setActive(true);
        m_horizontalCenterRow->setTargetLineType(source->anchors()->horizontalCenter().type());
        m_horizontalCenterRow->setMarginOffset(source->anchors()->horizontalCenterOffset());
        m_horizontalCenterRow->setCurrentTargetControl(source->anchors()->horizontalCenter().control());
    }

    if (source->anchors()->left().isValid()) {
        m_verticalCenterRow->setActive(true);
        m_verticalCenterRow->setTargetLineType(source->anchors()->verticalCenter().type());
        m_verticalCenterRow->setMarginOffset(source->anchors()->verticalCenterOffset());
        m_verticalCenterRow->setCurrentTargetControl(source->anchors()->verticalCenter().control());
    }

    show();
}
