#include <anchorrow.h>
#include <paintutils.h>
#include <buttongroup.h>
#include <control.h>

#include <QBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QEvent>

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
        return renderOverlaidPixmap(pixmap(QStringLiteral(":/images/anchors/left.svg"), {15, 15}, widget), color);
    case AnchorLine::Right:
        return renderOverlaidPixmap(pixmap(QStringLiteral(":/images/anchors/right.svg"), {15, 15}, widget), color);
    case AnchorLine::Top:
        return renderOverlaidPixmap(pixmap(QStringLiteral(":/images/anchors/top.svg"), {15, 15}, widget), color);
    case AnchorLine::Bottom:
        return renderOverlaidPixmap(pixmap(QStringLiteral(":/images/anchors/bottom.svg"), {15, 15}, widget), color);
    case AnchorLine::HorizontalCenter:
        return renderOverlaidPixmap(pixmap(QStringLiteral(":/images/anchors/horizontal.svg"), {15, 15}, widget), color);
    case AnchorLine::VerticalCenter:
        return renderOverlaidPixmap(pixmap(QStringLiteral(":/images/anchors/vertical.svg"), {15, 15}, widget), color);
    case AnchorLine::Fill:
        return renderOverlaidPixmap(pixmap(QStringLiteral(":/images/anchors/fill.svg"), {15, 15}, widget), color);
    case AnchorLine::Center:
        return renderOverlaidPixmap(pixmap(QStringLiteral(":/images/anchors/center.svg"), {15, 15}, widget), color);
    default:
        return QPixmap();
    }
}

AnchorRow::AnchorRow(AnchorLine::Type sourceLineType, QWidget* parent) : QWidget(parent)
  , m_sourceLineType(sourceLineType)
  , m_targetLineType(AnchorLine::Invalid)
  , m_fillCenterModeEnabled(false)
  , m_popupModeEnabled(false)
  , m_layout(new QHBoxLayout(this))
  , m_sourceButton(new QPushButton(this))
  , m_targetControlComboBox(new QComboBox(this))
  , m_marginOffsetSpinBox(new QDoubleSpinBox(this))
  , m_targetButtonGroup(new ButtonGroup(this))
  , m_targetLineButton1(new QPushButton(this))
  , m_targetLineButton2(new QPushButton(this))
  , m_targetLineButton3(new QPushButton(this))
{
    auto arrowIcon = new QLabel(this);
    auto targetButtonLayout = new QHBoxLayout;

    m_sourceButton->setFocusPolicy(Qt::NoFocus);
    m_targetControlComboBox->setFocusPolicy(Qt::NoFocus);
    m_targetLineButton1->setFocusPolicy(Qt::NoFocus);
    m_targetLineButton2->setFocusPolicy(Qt::NoFocus);
    m_targetLineButton3->setFocusPolicy(Qt::NoFocus);

    m_layout->setSpacing(6);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_sourceButton);
    m_layout->addSpacing(1);
    m_layout->addWidget(arrowIcon, 0, Qt::AlignCenter);
    m_layout->addSpacing(6);
    m_layout->addWidget(m_targetControlComboBox);
    m_layout->addLayout(targetButtonLayout);
    m_layout->addWidget(m_marginOffsetSpinBox);
    m_layout->addStretch();

    m_sourceButton->setFixedSize({23, 23});
    m_sourceButton->setIconSize({15, 15});
    m_sourceButton->setToolTip(anchorLineText(m_sourceLineType));

    arrowIcon->setFixedSize(16, 16);
    arrowIcon->setPixmap(PaintUtils::renderOverlaidPixmap(":/images/extension.svg", Qt::darkGray, QSize(16, 16), this));

    m_targetControlComboBox->setCursor(Qt::PointingHandCursor);
    m_targetControlComboBox->setFixedSize(QSize(140, 23));
    m_targetControlComboBox->setToolTip(tr("Target control"));
    m_targetControlComboBox->setEnabled(false);

    m_marginOffsetSpinBox->setToolTip(anchorLineText(m_sourceLineType) +
                                      (AnchorLine::isOffset(m_sourceLineType) ? " offset" : " margin"));
    m_marginOffsetSpinBox->setCursor(Qt::PointingHandCursor);
    m_marginOffsetSpinBox->setFixedSize(QSize(80, 23));
    m_marginOffsetSpinBox->setRange(-999.99, 999.99);
    m_marginOffsetSpinBox->setDecimals(2);

    targetButtonLayout->setSpacing(0);
    targetButtonLayout->setContentsMargins(0, 0, 0, 0);
    targetButtonLayout->addWidget(m_targetLineButton1);
    targetButtonLayout->addWidget(m_targetLineButton2);
    targetButtonLayout->addWidget(m_targetLineButton3);

    m_targetLineButton1->setCursor(Qt::PointingHandCursor);
    m_targetLineButton1->setFixedSize({23, 23});
    m_targetLineButton1->setIconSize({15, 15});
    m_targetLineButton1->setCheckable(true);
    m_targetLineButton1->setToolTip(anchorLineText(AnchorLine::isVertical(m_sourceLineType)
                                                   ? AnchorLine::Left : AnchorLine::Top));

    m_targetLineButton2->setCursor(Qt::PointingHandCursor);
    m_targetLineButton2->setFixedSize({23, 23});
    m_targetLineButton2->setIconSize({15, 15});
    m_targetLineButton2->setCheckable(true);
    m_targetLineButton2->setToolTip(anchorLineText(AnchorLine::isVertical(m_sourceLineType)
                                                   ? AnchorLine::HorizontalCenter
                                                   : AnchorLine::VerticalCenter));

    m_targetLineButton3->setCursor(Qt::PointingHandCursor);
    m_targetLineButton3->setFixedSize({23, 23});
    m_targetLineButton3->setIconSize({15, 15});
    m_targetLineButton3->setCheckable(true);
    m_targetLineButton3->setToolTip(anchorLineText(AnchorLine::isVertical(m_sourceLineType)
                                                   ? AnchorLine::Right : AnchorLine::Bottom));

    m_targetButtonGroup->addButton(m_targetLineButton1);
    m_targetButtonGroup->addButton(m_targetLineButton2);
    m_targetButtonGroup->addButton(m_targetLineButton3);

    if (AnchorLine::isFillCenter(m_sourceLineType)) {
        m_sourceButton->setCheckable(true);
        m_sourceButton->setCursor(Qt::PointingHandCursor);
        m_marginOffsetSpinBox->setVisible(false);
        m_targetLineButton1->setVisible(false);
        m_targetLineButton2->setVisible(false);
        m_targetLineButton3->setVisible(false);
    } else {
        m_sourceButton->setStyleSheet("border: none");
    }

    connect(m_sourceButton, &QPushButton::toggled,
            this, &AnchorRow::onSourceButtonCheckedChange);
    connect(m_sourceButton, &QPushButton::clicked,
            this, &AnchorRow::sourceButtonClicked);
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

    updateIcons();
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
    Q_ASSERT(!AnchorLine::isFillCenter(m_sourceLineType));
    Q_ASSERT(!m_fillCenterModeEnabled);
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
    Q_ASSERT(!AnchorLine::isFillCenter(m_sourceLineType));
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

void AnchorRow::setTargetControl(const Control* control, bool overlaid)
{
    if (overlaid) {
        Q_ASSERT(popupModeEnabled() && m_sourceLineType == AnchorLine::Center);
        m_targetControlComboBox->setCurrentText(QStringLiteral("Overlay.overlay"));
    } else {
        for (int i = 0; i < m_targetControlComboBox->count(); ++i) {
            if (m_targetControlComboBox->itemData(i).value<Control*>() == control) {
                m_targetControlComboBox->setCurrentIndex(i);
                break;
            }
        }
    }
}

bool AnchorRow::fillCenterModeEnabled() const
{
    return m_fillCenterModeEnabled;
}

void AnchorRow::setFillCenterModeEnabled(bool fillCenterModeEnabled, Control* targetControl)
{
    Q_ASSERT(!AnchorLine::isFillCenter(m_sourceLineType));

    if (fillCenterModeEnabled && targetControl == 0)
        return;

    if (m_fillCenterModeEnabled != fillCenterModeEnabled) {
        if (fillCenterModeEnabled) {
            m_targetLineTypeBackup = targetLineType();
            m_targetControlBackup = this->targetControl();
            setTargetControl(targetControl);
            setTargetLineType(m_sourceLineType);
            m_targetControlComboBox->setEnabled(false);
            m_targetLineButton1->setEnabled(false);
            m_targetLineButton2->setEnabled(false);
            m_targetLineButton3->setEnabled(false);
        }
        m_fillCenterModeEnabled = fillCenterModeEnabled;
        if (!fillCenterModeEnabled) {
            setTargetControl(m_targetControlBackup);
            setTargetLineType(m_targetLineTypeBackup);
            m_targetControlComboBox->setEnabled(m_targetButtonGroup->checkedButton());
            m_targetLineButton1->setEnabled(true);
            m_targetLineButton2->setEnabled(true);
            m_targetLineButton3->setEnabled(true);
        }
    }
}

bool AnchorRow::popupModeEnabled() const
{
    return m_popupModeEnabled;
}

void AnchorRow::setPopupModeEnabled(const Control* control)
{
    Q_ASSERT(m_sourceLineType == AnchorLine::Center);
    if (m_popupModeEnabled != control->popup()) {
        m_popupModeEnabled = control->popup();
        if (m_popupModeEnabled && control->hasWindowAncestor())
            m_targetControlComboBox->addItem(QStringLiteral("Overlay.overlay"));
    }
}

void AnchorRow::setSourceButtonChecked(bool checked)
{
    Q_ASSERT(AnchorLine::isFillCenter(m_sourceLineType));
    m_sourceButton->setChecked(checked);
}

void AnchorRow::clear()
{
    if (AnchorLine::isFillCenter(m_sourceLineType)) {
        if (m_sourceLineType == AnchorLine::Center)
            m_popupModeEnabled = false;
        setSourceButtonChecked(false);
    } else {
        setFillCenterModeEnabled(false);
        setTargetLineType(AnchorLine::Invalid);
        m_marginOffsetSpinBox->setValue(0);
    }
    m_targetControlComboBox->clear();
}

void AnchorRow::onTargetLineTypeChange()
{
    if (AnchorLine::isVertical(m_sourceLineType) == AnchorLine::isVertical(m_targetLineType)) {
        switch (m_targetLineType) {
        case AnchorLine::Left:
        case AnchorLine::Top:
            m_targetLineButton1->setChecked(true);
            m_targetControlComboBox->setEnabled(true);
            return;
        case AnchorLine::HorizontalCenter:
        case AnchorLine::VerticalCenter:
            m_targetLineButton2->setChecked(true);
            m_targetControlComboBox->setEnabled(true);
            return;
        case AnchorLine::Right:
        case AnchorLine::Bottom:
            m_targetLineButton3->setChecked(true);
            m_targetControlComboBox->setEnabled(true);
            return;
        default:
            break;
        }
    }
    m_targetButtonGroup->uncheckAll();
    m_targetControlComboBox->setEnabled(false);
}

void AnchorRow::onSourceButtonCheckedChange(bool checked)
{
    if (AnchorLine::isFillCenter(m_sourceLineType))
        m_targetControlComboBox->setEnabled(checked);
}

void AnchorRow::onTargetButtonClick(QAbstractButton* button, bool checked)
{
    if (checked) {
        if (button == m_targetLineButton1) {
            setTargetLineType(AnchorLine::isVertical(m_sourceLineType)
                              ? AnchorLine::Left : AnchorLine::Top);
        } else if (button == m_targetLineButton2) {
            setTargetLineType(AnchorLine::isVertical(m_sourceLineType)
                              ? AnchorLine::HorizontalCenter : AnchorLine::VerticalCenter);
        } else {
            setTargetLineType(AnchorLine::isVertical(m_sourceLineType)
                              ? AnchorLine::Right : AnchorLine::Bottom);
        }
    } else {
        setTargetLineType(AnchorLine::Invalid);
    }
}

void AnchorRow::updateIcons()
{
    m_sourceButton->setIcon(anchorLinePixmap(m_sourceLineType, this));
    m_targetLineButton1->setIcon(anchorLinePixmap(AnchorLine::isVertical(m_sourceLineType)
                                                  ? AnchorLine::Left : AnchorLine::Top, this));
    m_targetLineButton2->setIcon(anchorLinePixmap(AnchorLine::isVertical(m_sourceLineType)
                                                  ? AnchorLine::HorizontalCenter
                                                  : AnchorLine::VerticalCenter, this));
    m_targetLineButton3->setIcon(anchorLinePixmap(AnchorLine::isVertical(m_sourceLineType)
                                                  ? AnchorLine::Right : AnchorLine::Bottom, this));
}

void AnchorRow::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::ApplicationFontChange
            || event->type() == QEvent::PaletteChange) {
        updateIcons();
    }
    QWidget::changeEvent(event);
}
