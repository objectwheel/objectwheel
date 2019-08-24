#include <anchoreditor.h>
#include <anchorrow.h>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>

AnchorEditor::AnchorEditor(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_leftRow(new AnchorRow(this))
  , m_rightRow(new AnchorRow(this))
  , m_topRow(new AnchorRow(this))
  , m_bottomRow(new AnchorRow(this))
  , m_fillRow(new AnchorRow(this))
  , m_horizontalCenterRow(new AnchorRow(this))
  , m_verticalCentrRow(new AnchorRow(this))
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
    m_layout->addWidget(m_verticalCentrRow);
    m_layout->addWidget(m_centerRow);
    m_layout->addWidget(m_closeButton, 0, Qt::AlignVCenter | Qt::AlignRight);
    m_layout->addStretch();
    m_layout->setSizeConstraint(QLayout::SetFixedSize);

    m_leftRow->setSourceLineType(AnchorLine::Left);
    m_topRow->setSourceLineType(AnchorLine::Top);
    m_rightRow->setSourceLineType(AnchorLine::Right);
    m_bottomRow->setSourceLineType(AnchorLine::Bottom);
    m_horizontalCenterRow->setSourceLineType(AnchorLine::HorizontalCenter);
    m_verticalCentrRow->setSourceLineType(AnchorLine::VerticalCenter);
    m_fillRow->setSourceLineType(AnchorLine::Fill);
    m_centerRow->setSourceLineType(AnchorLine::Center);

    m_closeButton->setFixedHeight(24);
    m_closeButton->setText(tr("Close"));
    m_closeButton->setToolTip(tr("Close the Anchor Editor"));
    m_closeButton->setCursor(Qt::PointingHandCursor);
    connect(m_closeButton, &QPushButton::clicked, this, &AnchorEditor::close);
}
