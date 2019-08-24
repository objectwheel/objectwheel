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
  , m_marginOffset(0)
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

    m_targetButtonGroup->addButton(m_targetLineButton1);
    m_targetButtonGroup->addButton(m_targetLineButton2);
    m_targetButtonGroup->addButton(m_targetLineButton3);

    connect(this, &AnchorRow::sourceLineTypeChanged,
            this, &AnchorRow::onSourceLineTypeChange);
    connect(this, &AnchorRow::targetLineTypeChanged,
            this, &AnchorRow::onTargetLineTypeChange);
    connect(m_sourceLineButton, &QToolButton::clicked,
            this, &AnchorRow::onSourceButtonCheckedChange);

    onSourceButtonCheckedChange();
}

qreal AnchorRow::marginOffset() const
{
    return m_marginOffset;
}

void AnchorRow::setMarginOffset(qreal marginOffset)
{
    if (m_marginOffset != marginOffset) {
        m_marginOffset = marginOffset;
        emit marginOffsetChanged();
    }
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
    bool offset = m_sourceLineButton->toolTip().contains(QRegExp("vertical|horizontal", Qt::CaseInsensitive));
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

    bool showTargetGadgets = m_sourceLineType != AnchorLine::Fill && m_sourceLineType != AnchorLine::Center;
    m_targetLineButton1->setVisible(showTargetGadgets);
    m_targetLineButton2->setVisible(showTargetGadgets);
    m_targetLineButton3->setVisible(showTargetGadgets);
    m_marginOffsetSpinBox->setVisible(showTargetGadgets);
    m_targetControlComboBox->setVisible(showTargetGadgets);
    onTargetLineTypeChange();
}

void AnchorRow::onTargetLineTypeChange()
{
    m_targetButtonGroup->uncheckAll();

    if (AnchorLine::isVertical(m_sourceLineType) == AnchorLine::isVertical(m_targetLineType)) {
        switch (m_targetLineType) {
        case AnchorLine::Left:
        case AnchorLine::Top:
            m_targetLineButton1->setChecked(true);
            break;
        case AnchorLine::HorizontalCenter:
        case AnchorLine::VerticalCenter:
            m_targetLineButton2->setChecked(true);
            break;
        case AnchorLine::Right:
        case AnchorLine::Bottom:
            m_targetLineButton3->setChecked(true);
            break;
        default:
            break;
        }
    }
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
