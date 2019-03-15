#include <rundevicesbutton.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QMenu>
#include <QPainter>
#include <QStyleOption>

RunDevicesButton::RunDevicesButton(QWidget *parent) : QPushButton(parent)
  , m_menu(new QMenu(this))
{
    PaintUtils::setPanelButtonPaletteDefaults(this);

    m_menu->setToolTipsVisible(true);

    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    setMenu(m_menu);
    connect(m_menu, &QMenu::triggered, this, &RunDevicesButton::onTriggered);
}

QString RunDevicesButton::activeDevice() const
{
    for (QAction* action : m_menu->actions()) {
        if (action->isChecked())
            return action->property("_ow_deviceInfo").value<QVariantMap>().value("deviceUid").toString();
    }
    return QString();
}

void RunDevicesButton::setActiveDevice(const QString& uid)
{
    for (QAction* action : m_menu->actions()) {
        const QVariantMap& deviceInfo = action->property("_ow_deviceInfo").value<QVariantMap>();
        if (deviceInfo["deviceUid"].toString() == uid) {
            m_menu->setIcon(action->icon());
            m_menu->setTitle(action->text());
            setToolTip(action->toolTip());
            for (QAction* action : m_menu->actions()) {
                if (action->isChecked())
                    action->setChecked(false);
            }
            action->setChecked(true);
            updateGeometry();
            break;
        }
    }
}

bool RunDevicesButton::hasDevice(const QString& uid) const
{
    for (const QAction* action : m_menu->actions()) {
        const QVariantMap& deviceInfo = action->property("_ow_deviceInfo").value<QVariantMap>();
        if (deviceInfo["deviceUid"].toString() == uid)
            return true;
    }
    return false;
}

void RunDevicesButton::addDevice(const QVariantMap& deviceInfo)
{
    if (hasDevice(deviceInfo["deviceUid"].toString()))
        return;

    auto deviceAction = new QAction(this);
    deviceAction->setProperty("_ow_deviceInfo", deviceInfo);
    deviceAction->setText(UtilityFunctions::deviceName(deviceInfo));
    deviceAction->setIcon(UtilityFunctions::deviceIcon(deviceInfo));
    deviceAction->setToolTip(UtilityFunctions::deviceInfoToolTip(deviceInfo));
    deviceAction->setCheckable(true);

    m_menu->addAction(deviceAction);

    setActiveDevice(deviceInfo["deviceUid"].toString());
}

void RunDevicesButton::removeDevice(const QString& uid)
{
    for (QAction* action : m_menu->actions()) {
        const QVariantMap& deviceInfo = action->property("_ow_deviceInfo").value<QVariantMap>();
        if (deviceInfo["deviceUid"].toString() == uid) {
            if (action->isChecked())
                setActiveDevice("000000000000");
            m_menu->removeAction(action);
            break;
        }
    }
}

QSize RunDevicesButton::sizeHint() const
{
    int left = 8;
    int right = 12;
    int spacing = 4;
    const QSizeF& arrowSize = iconSize() - QSizeF(4, 4);
    int computedWidth = left + iconSize().width() + spacing +
            fontMetrics().horizontalAdvance(text()) + spacing +
            arrowSize.width() + spacing +
            iconSize().width() + spacing +
            fontMetrics().horizontalAdvance(m_menu->title()) + right;
    return QSize(qMin(computedWidth, 270), 24);
}

QSize RunDevicesButton::minimumSizeHint() const
{
    return QSize(150, 24);
}

void RunDevicesButton::onTriggered(QAction* action)
{
    const QVariantMap& deviceInfo = action->property("_ow_deviceInfo").value<QVariantMap>();
    const QString& deviceUid = deviceInfo["deviceUid"].toString();
    setActiveDevice(deviceUid);
    emit triggered(deviceUid);
}

void RunDevicesButton::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(isDown() ? palette().buttonText().color().darker() : palette().buttonText().color());

    // Draw background
    QStyleOptionButton opt;
    opt.initFrom(this);
    opt.state |= isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
    PaintUtils::drawPanelButtonBevel(&p, opt);

    // Draw devices icon
    int left = 8;
    int right = 12;
    int spacing = 4;
    const QPixmap& dp = UtilityFunctions::pixmap(this, icon(), iconSize());
    p.drawPixmap(UtilityFunctions::verticalAlignedRect(iconSize(), rect(), left), dp, dp.rect());

    // Draw devices text
    left += iconSize().width() + spacing;
    int textWidth = fontMetrics().horizontalAdvance(text());
    p.drawText(left, 0, textWidth, height(), Qt::AlignCenter, text());

    // Draw arrow
    left += textWidth + spacing;
    const QSizeF& arrowSize = iconSize() - QSizeF(4, 4);
    const QPixmap nextPixmap = UtilityFunctions::scaled(
                PaintUtils::renderMaskedPixmap(":/utils/images/next@2x.png", p.pen().color(), this),
                (arrowSize * devicePixelRatioF()).toSize());
    p.drawPixmap(UtilityFunctions::verticalAlignedRect(arrowSize, rect(), left), nextPixmap,
                 nextPixmap.rect());

    // Draw device icon
    left += arrowSize.width() + spacing;
    const QPixmap& buttonPixmap = UtilityFunctions::pixmap(this, m_menu->icon(), iconSize());
    p.drawPixmap(UtilityFunctions::verticalAlignedRect(iconSize(), rect(), left), buttonPixmap, buttonPixmap.rect());

    // Draw device name
    left += iconSize().width() + spacing;
    textWidth = qMin(width() - left - right, fontMetrics().horizontalAdvance(m_menu->title()));
    p.drawText(left, 0, textWidth, height(), Qt::AlignVCenter | Qt::AlignLeft,
               fontMetrics().elidedText(m_menu->title(), Qt::ElideRight, textWidth + 1));

    // Draw menu down arrow
    if (isDown() || UtilityFunctions::hasHover(this)) {
        left += textWidth + (right - 4.5) / 2.0;
        PaintUtils::drawMenuDownArrow(&p, QPointF(left, 16), opt);
    }
}
