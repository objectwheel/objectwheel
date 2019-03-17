#include <rundevicesbutton.h>
#include <utilityfunctions.h>
#include <paintutils.h>

#include <QMenu>
#include <QPainter>
#include <QStyleOption>

RunDevicesButton::RunDevicesButton(QWidget* parent) : QPushButton(parent)
  , m_menu(new QMenu(this))
{
    setMenu(m_menu);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    PaintUtils::setPanelButtonPaletteDefaults(this);
    m_menu->setToolTipsVisible(true);
    connect(m_menu, &QMenu::triggered, this, [=] (QAction* action) {
        setCurrentDevice(UtilityFunctions::deviceUid(action));
    });
}

bool RunDevicesButton::hasDevice(const QString& uid) const
{
    for (const QAction* action : m_menu->actions()) {
        if (UtilityFunctions::deviceUid(action) == uid)
            return true;
    }
    return false;
}

void RunDevicesButton::addDevice(const DeviceInfo& deviceInfo)
{
    if (hasDevice(UtilityFunctions::deviceUid(deviceInfo)))
        return;

    auto action = new QAction(this);
    action->setCheckable(true);
    action->setText(UtilityFunctions::deviceName(deviceInfo));
    action->setIcon(UtilityFunctions::deviceIcon(deviceInfo));
    action->setToolTip(UtilityFunctions::deviceInfoToolTip(deviceInfo));
    UtilityFunctions::setDeviceInfo(action, deviceInfo);

    m_menu->addAction(action);
}

void RunDevicesButton::removeDevice(const QString& uid)
{
    for (QAction* action : m_menu->actions()) {
        if (UtilityFunctions::deviceUid(action) == uid) {
            m_menu->removeAction(action);
            action->deleteLater();
            break;
        }
    }
}

QString RunDevicesButton::currentDevice() const
{
    for (const QAction* action : m_menu->actions()) {
        if (action->isChecked())
            return UtilityFunctions::deviceUid(action);
    }
    return QString();
}

void RunDevicesButton::setCurrentDevice(const QString& uid)
{
    static const auto setChecked = [this] (QAction* action) {
        for (QAction* act : m_menu->actions())
            act->setChecked(false);
        action->setChecked(true);
    };
    if (currentDevice() == uid)
        return;
    for (QAction* action : m_menu->actions()) {
        if (UtilityFunctions::deviceUid(action) == uid) {
            m_menu->setIcon(action->icon());
            m_menu->setTitle(action->text());
            setToolTip(action->toolTip());
            setChecked(action);
            updateGeometry();
            break;
        }
    }
}

QSize RunDevicesButton::sizeHint() const
{
    const int computedWidth = LEFT_PADDING
            + iconSize().width() + SPACING
            + fontMetrics().horizontalAdvance(text()) + SPACING + FORWARD_ARROW_LENGTH + SPACING
            + iconSize().width() + SPACING
            + fontMetrics().horizontalAdvance(m_menu->title()) + RIGHT_PADDING;
    return QSize(qMin(computedWidth, 270), 24);
}

QSize RunDevicesButton::minimumSizeHint() const
{
    return QSize(150, 24);
}

void RunDevicesButton::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Settings
    int left = LEFT_PADDING;
    int textWidth = fontMetrics().horizontalAdvance(text());
    bool isSunken = isDown() || (isCheckable() && isChecked());
    QIcon::Mode iconMode = isEnabled() ? isSunken ? QIcon::Active : QIcon::Normal : QIcon::Disabled;
    const QColor& textColor = isSunken ? palette().buttonText().color().darker() : palette().buttonText().color();

    QPen arrowPen(textColor);
    arrowPen.setWidthF(1.3);
    arrowPen.setCapStyle(Qt::RoundCap);
    arrowPen.setJoinStyle(Qt::MiterJoin);

    // Draw background
    QStyleOptionButton option;
    option.initFrom(this);
    option.state |= isSunken ? QStyle::State_Sunken : QStyle::State_Raised;
    PaintUtils::drawPanelButtonBevel(&painter, option);

    // Draw icon
    icon().paint(&painter, left, 0, iconSize().width(), height(), Qt::AlignCenter, iconMode);

    // Draw text
    left += iconSize().width() + SPACING;
    painter.setPen(textColor);
    painter.drawText(left, 0, textWidth, height(), Qt::AlignCenter, text());

    // Draw forward arrow
    left += textWidth + SPACING;
    QPointF topLeft(left + FORWARD_ARROW_LENGTH / 4.0, height() / 2.0 - FORWARD_ARROW_LENGTH / 2.0);
    QPointF points[] = {{0, 0}, {FORWARD_ARROW_LENGTH / 2.0, FORWARD_ARROW_LENGTH / 2.0}, {0, FORWARD_ARROW_LENGTH}};
    points[0] += topLeft; points[1] += topLeft; points[2] += topLeft;
    painter.setPen(arrowPen);
    painter.drawPolyline(points, 3);

    // Draw device icon
    left += FORWARD_ARROW_LENGTH + SPACING;
    m_menu->icon().paint(&painter, left, 0, iconSize().width(), height(), Qt::AlignCenter, iconMode);

    // Draw device name
    left += iconSize().width() + SPACING;
    textWidth = qMin(width() - left - RIGHT_PADDING, fontMetrics().horizontalAdvance(m_menu->title()));
    painter.setPen(textColor);
    painter.drawText(left, 0, textWidth, height(), Qt::AlignVCenter | Qt::AlignLeft,
                     fontMetrics().elidedText(m_menu->title(), Qt::ElideRight, textWidth + 1));

    // Draw menu down arrow
    if (isSunken || UtilityFunctions::hasHover(this)) {
        QPointF topLeft(width() - 2.5 * DOWN_ARROW_LENGTH, height() - 2 * DOWN_ARROW_LENGTH);
        QPointF points[] = {{0, 0}, {DOWN_ARROW_LENGTH / 2.0, DOWN_ARROW_LENGTH / 2.0}, {DOWN_ARROW_LENGTH, 0}};
        points[0] += topLeft; points[1] += topLeft; points[2] += topLeft;
        painter.setPen(arrowPen);
        painter.setBrush(textColor);
        painter.drawPolygon(points, 3);
    }
}
