#include <anchoreditor.h>
#include <anchorrow.h>
#include <QBoxLayout>
#include <QLabel>

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
{
    setWindowTitle(tr("Anchor Editor"));
    setWindowFlags(Qt::Dialog
                   | Qt::CustomizeWindowHint
                   | Qt::WindowTitleHint
                   | Qt::WindowCloseButtonHint);

    auto sourceLabel = new QLabel(tr("Source"), this);
    auto targetLabel = new QLabel(tr("Target"), this);
    sourceLabel->setSizePolicy(QSizePolicy::Fixed, sourceLabel->sizePolicy().verticalPolicy());
    auto hbox = new QHBoxLayout;
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(0);
    hbox->addWidget(sourceLabel);
    hbox->addSpacing(20);
    hbox->addWidget(targetLabel);

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
    m_layout->addStretch();

    m_leftRow->setSourceLineType(AnchorLine::Left);
    m_topRow->setSourceLineType(AnchorLine::Top);
    m_rightRow->setSourceLineType(AnchorLine::Right);
    m_bottomRow->setSourceLineType(AnchorLine::Bottom);
    m_horizontalCenterRow->setSourceLineType(AnchorLine::HorizontalCenter);
    m_verticalCentrRow->setSourceLineType(AnchorLine::VerticalCenter);
    m_fillRow->setSourceLineType(AnchorLine::Fill);
    m_centerRow->setSourceLineType(AnchorLine::Center);
}
