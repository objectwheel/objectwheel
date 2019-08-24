#include <anchorrow.h>
#include <QBoxLayout>
#include <QToolButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QButtonGroup>

static QString anchorLineText(AnchorLine::Type type)
{
    switch (type) {
    case AnchorLine::Left:
        return QStringLiteral("Left");
    case AnchorLine::Right:
        return QStringLiteral("Right");
    case AnchorLine::Top:
        return QStringLiteral("Top");
    case AnchorLine::Bottom:
        return QStringLiteral("Bottom");
    case AnchorLine::HorizontalCenter:
        return QStringLiteral("Horizontal center");
    case AnchorLine::VerticalCenter:
        return QStringLiteral("Vertical center");
    case AnchorLine::Fill:
        return QStringLiteral("Fill");
    case AnchorLine::Center:
        return QStringLiteral("Center in");
    default:
        return QString();
    }
}

static QIcon anchorLineIcon(AnchorLine::Type type)
{
    switch (type) {
    case AnchorLine::Left:
        return QIcon(":/images/anchors/left.svg");
    case AnchorLine::Right:
        return QIcon(":/images/anchors/right.svg");
    case AnchorLine::Top:
        return QIcon(":/images/anchors/top.svg");
    case AnchorLine::Bottom:
        return QIcon(":/images/anchors/bottom.svg");
    case AnchorLine::HorizontalCenter:
        return QIcon(":/images/anchors/horizontalCenter.svg");
    case AnchorLine::VerticalCenter:
        return QIcon(":/images/anchors/verticalCenter.svg");
    case AnchorLine::Fill:
        return QIcon(":/images/anchors/fill.svg");
    case AnchorLine::Center:
        return QIcon(":/images/anchors/center.svg");
    default:
        return QIcon();
    }
}

AnchorRow::AnchorRow(QWidget *parent) : QWidget(parent)
  , m_sourceLineType(AnchorLine::Invalid)
  , m_layout(new QHBoxLayout(this))
  , m_sourceLineButton(new QToolButton(this))
  , m_targetLineButton1(new QToolButton(this))
  , m_targetLineButton2(new QToolButton(this))
  , m_targetLineButton3(new QToolButton(this))
  , m_marginOffsetSpinBox(new QDoubleSpinBox(this))
  , m_targetControlComboBox(new QComboBox(this))
{
    auto bgroup = new QButtonGroup(this);
    bgroup->addButton(m_targetLineButton1);
    bgroup->addButton(m_targetLineButton2);
    bgroup->addButton(m_targetLineButton3);

    auto hbox = new QHBoxLayout;
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(0);
    hbox->addWidget(m_targetLineButton1);
    hbox->addWidget(m_targetLineButton2);
    hbox->addWidget(m_targetLineButton3);

    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(6);
    m_layout->addWidget(m_sourceLineButton);
    m_layout->addSpacing(30);
    m_layout->addLayout(hbox);
    m_layout->addWidget(m_marginOffsetSpinBox);
    m_layout->addWidget(m_targetControlComboBox);
    m_layout->addStretch();

    m_sourceLineButton->setCursor(Qt::PointingHandCursor);
    m_sourceLineButton->setFixedSize(QSize(24, 24));
    m_sourceLineButton->setIconSize({12, 12});
    m_sourceLineButton->setCheckable(true);

    m_targetLineButton1->setCursor(Qt::PointingHandCursor);
    m_targetLineButton1->setFixedSize(QSize(24, 24));
    m_targetLineButton1->setIconSize({12, 12});
    m_targetLineButton1->setCheckable(true);

    m_targetLineButton2->setCursor(Qt::PointingHandCursor);
    m_targetLineButton2->setFixedSize(QSize(24, 24));
    m_targetLineButton2->setIconSize({12, 12});
    m_targetLineButton2->setCheckable(true);

    m_targetLineButton3->setCursor(Qt::PointingHandCursor);
    m_targetLineButton3->setFixedSize(QSize(24, 24));
    m_targetLineButton3->setIconSize({12, 12});
    m_targetLineButton3->setCheckable(true);

    m_marginOffsetSpinBox->setCursor(Qt::PointingHandCursor);
    m_marginOffsetSpinBox->setFixedSize(QSize(64, 24));
    m_marginOffsetSpinBox->setRange(-999.99, 999.99);
    m_marginOffsetSpinBox->setDecimals(2);

    m_targetControlComboBox->setCursor(Qt::PointingHandCursor);
    m_targetControlComboBox->setFixedSize(QSize(120, 24));

    connect(this, &AnchorRow::sourceLineTypeChanged,
            this, &AnchorRow::onSourceLineTypeChange);
    connect(m_sourceLineButton, &QToolButton::clicked,
            this, &AnchorRow::onSourceButtonCheckedChange);

    onSourceButtonCheckedChange();
}

AnchorLine::Type AnchorRow::sourceLineType() const
{
    return m_sourceLineType;
}

void AnchorRow::setSourceLineType(AnchorLine::Type sourceLineType)
{
    if (m_sourceLineType != sourceLineType) {
        m_sourceLineType = sourceLineType;
        emit sourceLineTypeChanged();
    }
}

void AnchorRow::onSourceLineTypeChange()
{
    m_sourceLineButton->setToolTip(anchorLineText(m_sourceLineType));
    m_sourceLineButton->setIcon(anchorLineIcon(m_sourceLineType));

    if (AnchorLine::isVertical(m_sourceLineType)) {
        m_targetLineButton1->setIcon(anchorLineIcon(AnchorLine::Left));
        m_targetLineButton2->setIcon(anchorLineIcon(AnchorLine::HorizontalCenter));
        m_targetLineButton3->setIcon(anchorLineIcon(AnchorLine::Right));
    } else {
        m_targetLineButton1->setIcon(anchorLineIcon(AnchorLine::Top));
        m_targetLineButton2->setIcon(anchorLineIcon(AnchorLine::VerticalCenter));
        m_targetLineButton3->setIcon(anchorLineIcon(AnchorLine::Bottom));
    }

    bool showTargetGadgets = m_sourceLineType != AnchorLine::Fill && m_sourceLineType != AnchorLine::Center;
    m_targetLineButton1->setVisible(showTargetGadgets);
    m_targetLineButton2->setVisible(showTargetGadgets);
    m_targetLineButton3->setVisible(showTargetGadgets);
    m_marginOffsetSpinBox->setVisible(showTargetGadgets);
    m_targetControlComboBox->setVisible(showTargetGadgets);
}

void AnchorRow::onSourceButtonCheckedChange()
{
    bool enableTargetGadgets = m_sourceLineButton->isChecked();
    m_targetLineButton1->setEnabled(enableTargetGadgets);
    m_targetLineButton2->setEnabled(enableTargetGadgets);
    m_targetLineButton3->setEnabled(enableTargetGadgets);
    m_marginOffsetSpinBox->setEnabled(enableTargetGadgets);
    m_targetControlComboBox->setEnabled(enableTargetGadgets);
}
