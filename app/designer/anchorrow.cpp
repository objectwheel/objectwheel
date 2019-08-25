#include <anchorrow.h>
#include <paintutils.h>
#include <buttongroup.h>
#include <control.h>

#include <QBoxLayout>
#include <QComboBox>
#include <QToolButton>
#include <QLabel>
#include <QDoubleSpinBox>

static QString anchorLineText(AnchorLine::Type type)
{
    switch (type) {
    case AnchorLine::Left:
        return QObject::tr("Left");
    case AnchorLine::Right:
        return QObject::tr("Right");
    case AnchorLine::Top:
        return QObject::tr("Top");
    case AnchorLine::Bottom:
        return QObject::tr("Bottom");
    case AnchorLine::HorizontalCenter:
        return QObject::tr("Horizontal center");
    case AnchorLine::VerticalCenter:
        return QObject::tr("Vertical center");
    case AnchorLine::Fill:
        return QObject::tr("Fill");
    case AnchorLine::Center:
        return QObject::tr("Center in");
    default:
        return QString();
    }
}

static QPixmap anchorLinePixmap(AnchorLine::Type type, QWidget* widget)
{
    using namespace PaintUtils;
    const QColor& color = widget->palette().buttonText().color();
    switch (type) {
    case AnchorLine::Left:
        return renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/left.svg")), color);
    case AnchorLine::Right:
        return renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/right.svg")), color);
    case AnchorLine::Top:
        return renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/top.svg")), color);
    case AnchorLine::Bottom:
        return renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/bottom.svg")), color);
    case AnchorLine::HorizontalCenter:
        return renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/horizontal.svg")), color);
    case AnchorLine::VerticalCenter:
        return renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/vertical.svg")), color);
    case AnchorLine::Fill:
        return renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/fill.svg")), color);
    case AnchorLine::Center:
        return renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/center.svg")), color);
    default:
        return QPixmap();
    }
}

AnchorRow::AnchorRow(AnchorLine::Type sourceLineType, QWidget* parent) : QWidget(parent)
  , m_sourceLineType(sourceLineType)
  , m_targetLineType(AnchorLine::Invalid)
  , m_layout(new QHBoxLayout(this))
  , m_fillCenterButton(new QToolButton(this))
  , m_sourceIcon(new QLabel(this))
  , m_arrowIcon(new QLabel(this))
  , m_targetControlComboBox(new QComboBox(this))
  , m_marginOffsetSpinBox(new QDoubleSpinBox(this))
  , m_targetButtonGroup(new ButtonGroup(this))
  , m_targetLineButton1(new QToolButton(this))
  , m_targetLineButton2(new QToolButton(this))
  , m_targetLineButton3(new QToolButton(this))
{    
    bool vertical = AnchorLine::isVertical(m_sourceLineType);
    bool fillCenter = m_sourceLineType == AnchorLine::Fill || m_sourceLineType == AnchorLine::Center;
    bool offset = m_sourceLineType == AnchorLine::HorizontalCenter || m_sourceLineType == AnchorLine::VerticalCenter;
    auto hbox = new QHBoxLayout;
    hbox->setSpacing(0);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->addWidget(m_targetLineButton1);
    hbox->addWidget(m_targetLineButton2);
    hbox->addWidget(m_targetLineButton3);

    m_layout->setSpacing(6);
    m_layout->setContentsMargins(0, 0, 0, 0);
    if (!fillCenter)
        m_layout->addSpacing(5);
    m_layout->addWidget(fillCenter ? (QWidget*)m_fillCenterButton : (QWidget*)m_sourceIcon, 0, Qt::AlignCenter);
    m_layout->addSpacing(fillCenter ? 1 : 8);
    m_layout->addWidget(m_arrowIcon, 0, Qt::AlignCenter);
    m_layout->addSpacing(6);
    m_layout->addWidget(m_targetControlComboBox);
    m_layout->addWidget(m_marginOffsetSpinBox);
    m_layout->addLayout(hbox);
    m_layout->addStretch();

    m_sourceIcon->setFixedSize(12, 12);
    m_sourceIcon->setScaledContents(true);
    m_sourceIcon->setPixmap(anchorLinePixmap(m_sourceLineType, this));
    m_sourceIcon->setToolTip(anchorLineText(m_sourceLineType));

    m_fillCenterButton->setCursor(Qt::PointingHandCursor);
    m_fillCenterButton->setFixedSize(QSize(24, 24));
    m_fillCenterButton->setIconSize({12, 12});
    m_fillCenterButton->setCheckable(true);
    m_fillCenterButton->setIcon(anchorLinePixmap(m_sourceLineType, this));
    m_fillCenterButton->setToolTip(anchorLineText(m_sourceLineType));

    m_arrowIcon->setFixedSize(8, 8);
    m_arrowIcon->setScaledContents(true);
    m_arrowIcon->setPixmap(QPixmap(":/images/extension.svg"));

    m_marginOffsetSpinBox->setToolTip(anchorLineText(m_sourceLineType) + (offset ? " offset" : " margin"));
    m_marginOffsetSpinBox->setCursor(Qt::PointingHandCursor);
    m_marginOffsetSpinBox->setFixedSize(QSize(80, 24));
    m_marginOffsetSpinBox->setRange(-999.99, 999.99);
    m_marginOffsetSpinBox->setDecimals(2);
    m_marginOffsetSpinBox->setEnabled(false);

    m_targetControlComboBox->setCursor(Qt::PointingHandCursor);
    m_targetControlComboBox->setFixedSize(QSize(140, 24));
    m_targetControlComboBox->setToolTip(tr("Target control"));
    m_targetControlComboBox->setEnabled(false);

    m_targetLineButton1->setCursor(Qt::PointingHandCursor);
    m_targetLineButton1->setFixedSize(QSize(24, 24));
    m_targetLineButton1->setIconSize({12, 12});
    m_targetLineButton1->setCheckable(true);
    m_targetLineButton1->setIcon(anchorLinePixmap(vertical ? AnchorLine::Left : AnchorLine::Top, this));
    m_targetLineButton1->setToolTip(anchorLineText(vertical ? AnchorLine::Left : AnchorLine::Top));

    m_targetLineButton2->setCursor(Qt::PointingHandCursor);
    m_targetLineButton2->setFixedSize(QSize(24, 24));
    m_targetLineButton2->setIconSize({12, 12});
    m_targetLineButton2->setCheckable(true);
    m_targetLineButton2->setIcon(anchorLinePixmap(vertical ? AnchorLine::HorizontalCenter : AnchorLine::VerticalCenter, this));
    m_targetLineButton2->setToolTip(anchorLineText(vertical ? AnchorLine::HorizontalCenter : AnchorLine::VerticalCenter));

    m_targetLineButton3->setCursor(Qt::PointingHandCursor);
    m_targetLineButton3->setFixedSize(QSize(24, 24));
    m_targetLineButton3->setIconSize({12, 12});
    m_targetLineButton3->setCheckable(true);
    m_targetLineButton3->setIcon(anchorLinePixmap(vertical ? AnchorLine::Right : AnchorLine::Bottom, this));
    m_targetLineButton3->setToolTip(anchorLineText(vertical ? AnchorLine::Right : AnchorLine::Bottom));

    m_targetButtonGroup->addButton(m_targetLineButton1);
    m_targetButtonGroup->addButton(m_targetLineButton2);
    m_targetButtonGroup->addButton(m_targetLineButton3);

    if (fillCenter) {
        m_sourceIcon->setVisible(false);
        m_marginOffsetSpinBox->setVisible(false);
        m_targetLineButton1->setVisible(false);
        m_targetLineButton2->setVisible(false);
        m_targetLineButton3->setVisible(false);
    } else {
        m_fillCenterButton->setVisible(false);
    }

    connect(this, &AnchorRow::targetLineTypeChanged,
            this, &AnchorRow::onTargetLineTypeChange);
    connect(m_targetButtonGroup, &ButtonGroup::buttonClicked,
            this, &AnchorRow::onTargetButtonClick);
    connect(m_targetButtonGroup, &ButtonGroup::buttonClicked,
            this, &AnchorRow::targetLineTypeActivated);
    connect(m_marginOffsetSpinBox, &QDoubleSpinBox::editingFinished,
            this, &AnchorRow::marginOffsetEditingFinished);
    connect(m_targetControlComboBox, qOverload<int>(&QComboBox::activated),
            this, &AnchorRow::targetControlActivated);
}

AnchorLine::Type AnchorRow::sourceLineType() const
{
    return m_sourceLineType;
}

AnchorLine::Type AnchorRow::targetLineType() const
{
    return m_targetLineType;
}

void AnchorRow::setTargetLineType(AnchorLine::Type targetLineType)
{
    if (m_targetLineType != targetLineType) {
        m_targetLineType = targetLineType;
        emit targetLineTypeChanged();
    }
}

qreal AnchorRow::marginOffset() const
{
    return m_marginOffsetSpinBox->value();
}

void AnchorRow::setMarginOffset(qreal marginOffset)
{
    m_marginOffsetSpinBox->setValue(marginOffset);
}

QList<Control*> AnchorRow::targetControlList() const
{
    QList<Control*> controls;
    for (int i = 0; i < m_targetControlComboBox->count(); ++i)
        controls.append(m_targetControlComboBox->itemData(i).value<Control*>());
    return controls;
}

void AnchorRow::setTargetControlList(const QList<Control*>& targetControlList)
{
    for (Control* control : targetControlList)
        m_targetControlComboBox->addItem(control->id(), QVariant::fromValue(control));
}

Control* AnchorRow::targetControl() const
{
    return m_targetControlComboBox->currentData().value<Control*>();
}

void AnchorRow::setTargetControl(const Control* control)
{
    for (int i = 0; i < m_targetControlComboBox->count(); ++i) {
        if (m_targetControlComboBox->itemData(i).value<Control*>() == control) {
            m_targetControlComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void AnchorRow::clear()
{
    setTargetLineType(AnchorLine::Invalid);
    m_marginOffsetSpinBox->setValue(0);
    m_targetControlComboBox->clear();
}

void AnchorRow::onTargetLineTypeChange()
{
    if (AnchorLine::isVertical(m_sourceLineType) == AnchorLine::isVertical(m_targetLineType)) {
        switch (m_targetLineType) {
        case AnchorLine::Left:
        case AnchorLine::Top:
            m_targetLineButton1->setChecked(true);
            m_marginOffsetSpinBox->setEnabled(true);
            m_targetControlComboBox->setEnabled(true);
            return;
        case AnchorLine::HorizontalCenter:
        case AnchorLine::VerticalCenter:
            m_targetLineButton2->setChecked(true);
            m_marginOffsetSpinBox->setEnabled(true);
            m_targetControlComboBox->setEnabled(true);
            return;
        case AnchorLine::Right:
        case AnchorLine::Bottom:
            m_targetLineButton3->setChecked(true);
            m_marginOffsetSpinBox->setEnabled(true);
            m_targetControlComboBox->setEnabled(true);
            return;
        default:
            break;
        }
    }
    m_targetButtonGroup->uncheckAll();
    m_marginOffsetSpinBox->setEnabled(false);
    m_targetControlComboBox->setEnabled(false);
}

void AnchorRow::onTargetButtonClick(QAbstractButton* button, bool checked)
{
    if (checked) {
        bool vertical = AnchorLine::isVertical(m_sourceLineType);
        if (button == m_targetLineButton1)
            setTargetLineType(vertical ? AnchorLine::Left : AnchorLine::Top);
        else if (button == m_targetLineButton2)
            setTargetLineType(vertical ? AnchorLine::HorizontalCenter : AnchorLine::VerticalCenter);
        else
            setTargetLineType(vertical ? AnchorLine::Right : AnchorLine::Bottom);
    } else {
        setTargetLineType(AnchorLine::Invalid);
    }
}
