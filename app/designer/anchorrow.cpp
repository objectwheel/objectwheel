#include <anchorrow.h>
#include <paintutils.h>
#include <buttongroup.h>
#include <control.h>

#include <QBoxLayout>
#include <QToolButton>
#include <QComboBox>
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

static QIcon anchorLineIcon(AnchorLine::Type type, QWidget* widget)
{
    using namespace PaintUtils;
    const QColor& color = widget->palette().buttonText().color();
    switch (type) {
    case AnchorLine::Left:
        return QIcon(renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/left.svg")), color));
    case AnchorLine::Right:
        return QIcon(renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/right.svg")), color));
    case AnchorLine::Top:
        return QIcon(renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/top.svg")), color));
    case AnchorLine::Bottom:
        return QIcon(renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/bottom.svg")), color));
    case AnchorLine::HorizontalCenter:
        return QIcon(renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/horizontal.svg")), color));
    case AnchorLine::VerticalCenter:
        return QIcon(renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/vertical.svg")), color));
    case AnchorLine::Fill:
        return QIcon(renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/fill.svg")), color));
    case AnchorLine::Center:
        return QIcon(renderOverlaidPixmap(QPixmap(QStringLiteral(":/images/anchors/center.svg")), color));
    default:
        return QIcon();
    }
}

AnchorRow::AnchorRow(QWidget* parent) : QWidget(parent)
  , m_sourceLineType(AnchorLine::Invalid)
  , m_targetLineType(AnchorLine::Invalid)
  , m_layout(new QHBoxLayout(this))
  , m_sourceLineButton(new QToolButton(this))
  , m_targetButtonGroup(new ButtonGroup(this))
  , m_targetLineButton1(new QToolButton(this))
  , m_targetLineButton2(new QToolButton(this))
  , m_targetLineButton3(new QToolButton(this))
  , m_marginOffsetSpinBox(new QDoubleSpinBox(this))
  , m_targetControlComboBox(new QComboBox(this))
{
    auto hbox = new QHBoxLayout;
    hbox->setSpacing(0);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->addWidget(m_targetLineButton1);
    hbox->addWidget(m_targetLineButton2);
    hbox->addWidget(m_targetLineButton3);

    m_layout->setSpacing(4);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_sourceLineButton);
    m_layout->addSpacing(12);
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
    m_marginOffsetSpinBox->setFixedSize(QSize(72, 24));
    m_marginOffsetSpinBox->setRange(-999.99, 999.99);
    m_marginOffsetSpinBox->setDecimals(2);

    m_targetControlComboBox->setCursor(Qt::PointingHandCursor);
    m_targetControlComboBox->setFixedSize(QSize(120, 24));
    m_targetControlComboBox->setToolTip(tr("Target control"));

    m_targetButtonGroup->addButton(m_targetLineButton1);
    m_targetButtonGroup->addButton(m_targetLineButton2);
    m_targetButtonGroup->addButton(m_targetLineButton3);

    m_targetLineButton1->setEnabled(false);
    m_targetLineButton2->setEnabled(false);
    m_targetLineButton3->setEnabled(false);
    m_marginOffsetSpinBox->setEnabled(false);
    m_targetControlComboBox->setEnabled(false);

    connect(this, &AnchorRow::sourceLineTypeChanged,
            this, &AnchorRow::onSourceLineTypeChange);
    connect(this, &AnchorRow::targetLineTypeChanged,
            this, &AnchorRow::onTargetLineTypeChange);
    connect(m_sourceLineButton, &QToolButton::clicked,
            this, &AnchorRow::onSourceButtonClick);
    connect(m_targetButtonGroup, &ButtonGroup::buttonClicked,
            this, &AnchorRow::onTargetButtonClick);
    connect(m_targetButtonGroup, &ButtonGroup::buttonClicked,
            this, &AnchorRow::targetLineTypeActivated);
    connect(m_marginOffsetSpinBox, &QDoubleSpinBox::editingFinished,
            this, &AnchorRow::marginOffsetEditingFinished);
    connect(m_targetControlComboBox, qOverload<int>(&QComboBox::activated),
            this, &AnchorRow::targetControlActivated);
}

bool AnchorRow::isActive() const
{
    return m_sourceLineButton->isChecked();
}

void AnchorRow::setActive(bool active)
{
    if (m_sourceLineButton->isChecked() != active)
        m_sourceLineButton->click();
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

Control* AnchorRow::currentTargetControl() const
{
    return m_targetControlComboBox->currentData().value<Control*>();
}

void AnchorRow::setCurrentTargetControl(const Control* control)
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
    m_sourceLineButton->setChecked(false);
    setTargetLineType(AnchorLine::Invalid);
    m_targetLineButton1->setEnabled(false);
    m_targetLineButton2->setEnabled(false);
    m_targetLineButton3->setEnabled(false);
    m_marginOffsetSpinBox->setValue(0);
    m_targetControlComboBox->clear();
}

QSize AnchorRow::minimumSizeHint() const
{
    return QWidget::minimumSizeHint() + QSize(4, -4);
}

QSize AnchorRow::sizeHint() const
{
    return minimumSizeHint();
}

void AnchorRow::onSourceLineTypeChange()
{
    m_sourceLineButton->setIcon(anchorLineIcon(m_sourceLineType, this));
    m_sourceLineButton->setToolTip(anchorLineText(m_sourceLineType));

    bool gadgetsVisible = m_sourceLineType != AnchorLine::Fill && m_sourceLineType != AnchorLine::Center;
    m_targetLineButton1->setVisible(gadgetsVisible);
    m_targetLineButton2->setVisible(gadgetsVisible);
    m_targetLineButton3->setVisible(gadgetsVisible);
    m_marginOffsetSpinBox->setVisible(gadgetsVisible);
    m_targetControlComboBox->setVisible(gadgetsVisible);

    if (gadgetsVisible) {
        bool offset = m_sourceLineButton->toolTip().contains(QRegularExpression("Vertical|Horizontal"));
        m_marginOffsetSpinBox->setToolTip(anchorLineText(m_sourceLineType) + (offset ? " offset" : " margin"));
        if (AnchorLine::isVertical(m_sourceLineType)) {
            m_targetLineButton1->setIcon(anchorLineIcon(AnchorLine::Left, this));
            m_targetLineButton2->setIcon(anchorLineIcon(AnchorLine::HorizontalCenter, this));
            m_targetLineButton3->setIcon(anchorLineIcon(AnchorLine::Right, this));
            m_targetLineButton1->setToolTip(anchorLineText(AnchorLine::Left));
            m_targetLineButton2->setToolTip(anchorLineText(AnchorLine::HorizontalCenter));
            m_targetLineButton3->setToolTip(anchorLineText(AnchorLine::Right));
        } else {
            m_targetLineButton1->setIcon(anchorLineIcon(AnchorLine::Top, this));
            m_targetLineButton2->setIcon(anchorLineIcon(AnchorLine::VerticalCenter, this));
            m_targetLineButton3->setIcon(anchorLineIcon(AnchorLine::Bottom, this));
            m_targetLineButton1->setToolTip(anchorLineText(AnchorLine::Top));
            m_targetLineButton2->setToolTip(anchorLineText(AnchorLine::VerticalCenter));
            m_targetLineButton3->setToolTip(anchorLineText(AnchorLine::Bottom));
        }
        onTargetLineTypeChange();
    }
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

void AnchorRow::onSourceButtonClick()
{
    bool enableTargetButtons = m_sourceLineButton->isChecked();
    bool targetLineValid = m_targetButtonGroup->checkedButton();
    m_targetLineButton1->setEnabled(enableTargetButtons);
    m_targetLineButton2->setEnabled(enableTargetButtons);
    m_targetLineButton3->setEnabled(enableTargetButtons);
    m_marginOffsetSpinBox->setEnabled(enableTargetButtons && targetLineValid);
    m_targetControlComboBox->setEnabled(enableTargetButtons && targetLineValid);
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
