#include <devicesbutton.h>
#include <paintutils.h>
#include <utilsicons.h>
#include <utilityfunctions.h>
#include <generalsettings.h>
#include <interfacesettings.h>

#include <QMenu>
#include <QPainter>
#include <QStyleOptionButton>

using namespace Utils;
using namespace PaintUtils;
using namespace UtilityFunctions;

namespace {

const QSizeF& g_iconSize = {18, 18};
const QSizeF& g_arrowSize = {12, 12};
const int g_leftPadding = 6;
const int g_rightPadding = 10;
const int g_spacing = 3;
const char* g_iconText = "Devices";
}

DevicesButton::DevicesButton(QWidget *parent) : QPushButton(parent)
  , m_devicesIcon(":/images/devices.png")
  , m_menu(new QMenu(this))
  , m_actionGroup(new QActionGroup(this))
{
    QPalette p(palette());
    PaintUtils::setPanelButtonPaletteDefaults(p, GeneralSettings::interfaceSettings()->theme == "Light");
    setPalette(p);

    m_menu->setToolTipsVisible(true);
    m_actionGroup->setExclusive(true);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    setMenu(m_menu);
    connect(m_menu, &QMenu::triggered, this, &DevicesButton::onMenuItemActivation);

    addDevice(UtilityFunctions::deviceInfo());
}

void DevicesButton::addDevice(const QVariantMap& deviceInfo)
{
    if (deviceExists(deviceInfo["deviceUid"].toString()))
        return;

    auto deviceAction = new QAction(this);
    deviceAction->setProperty("_ow_deviceInfo", deviceInfo);
    deviceAction->setText(UtilityFunctions::deviceName(deviceInfo));
    deviceAction->setIcon(UtilityFunctions::deviceIcon(deviceInfo));
    deviceAction->setToolTip(UtilityFunctions::deviceInfoToolTip(deviceInfo));
    deviceAction->setCheckable(true);

    m_actionGroup->addAction(deviceAction);
    m_menu->addAction(deviceAction);

    setActiveDevice(deviceInfo["deviceUid"].toString());
}

void DevicesButton::removeDevice(const QString& uid)
{
    for (QAction* action : m_actionGroup->actions()) {
        const QVariantMap& deviceInfo = action->property("_ow_deviceInfo").value<QVariantMap>();
        if (deviceInfo["deviceUid"].toString() == uid) {
            if (action->isChecked())
                setActiveDevice("000000000000");
            m_menu->removeAction(action);
            m_actionGroup->removeAction(action);
            break;
        }
    }
}

void DevicesButton::setActiveDevice(const QString& uid)
{
    for (QAction* action : m_actionGroup->actions()) {
        const QVariantMap& deviceInfo = action->property("_ow_deviceInfo").value<QVariantMap>();
        if (deviceInfo["deviceUid"].toString() == uid) {
            setIcon(action->icon());
            setText(action->text());
            setToolTip(action->toolTip());
            action->setChecked(true);
            emit activeDeviceChanged(uid);
            break;
        }
    }
}

QString DevicesButton::activeDevice() const
{
    for (QAction* action : m_actionGroup->actions()) {
        if (action->isChecked())
            return action->property("_ow_deviceInfo").value<QVariantMap>().value("deviceUid").toString();
    }
    return QString();
}

void DevicesButton::onMenuItemActivation(QAction* action)
{
    const QVariantMap& deviceInfo = action->property("_ow_deviceInfo").value<QVariantMap>();
    const QString& deviceUid = deviceInfo["deviceUid"].toString();
    setActiveDevice(deviceUid);
    emit activeDeviceTriggered(deviceUid);
}

bool DevicesButton::deviceExists(const QString& uid) const
{
    for (const QAction* action : m_actionGroup->actions()) {
        const QVariantMap& deviceInfo = action->property("_ow_deviceInfo").value<QVariantMap>();
        if (deviceInfo["deviceUid"].toString() == uid)
            return true;
    }
    return false;
}

void DevicesButton::paintEvent(QPaintEvent*)
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
    int left = g_leftPadding;
    const QPixmap& dp = pixmap(this, m_devicesIcon, g_iconSize);
    p.drawPixmap(verticalAlignedRect(g_iconSize, rect(), left), dp, dp.rect());

    // Draw devices text
    left += g_iconSize.width() + g_spacing;
    int textWidth = fontMetrics().horizontalAdvance(tr(g_iconText));
    p.drawText(left, 0, textWidth, height(), Qt::AlignCenter, tr(g_iconText));

    // Draw arrow
    left += textWidth + g_spacing;
    const QPixmap nextPixmap = scaled(renderMaskedPixmap(":/utils/images/next@2x.png", p.pen().color(), this),
                                      (g_arrowSize * devicePixelRatioF()).toSize());
    p.drawPixmap(verticalAlignedRect(g_arrowSize, rect(), left), nextPixmap, nextPixmap.rect());

    // Draw device icon
    left += g_arrowSize.width() + g_spacing;
    const QPixmap& buttonPixmap = pixmap(this, icon(), g_iconSize);
    p.drawPixmap(verticalAlignedRect(g_iconSize, rect(), left), buttonPixmap, buttonPixmap.rect());

    // Draw device name
    left += g_iconSize.width() + g_spacing;
    textWidth = qMin(width() - left - g_rightPadding, fontMetrics().horizontalAdvance(text()));
    p.drawText(left, 0, textWidth, height(), Qt::AlignVCenter | Qt::AlignLeft,
               fontMetrics().elidedText(text(), Qt::ElideRight, textWidth + 1));

    // Draw menu down arrow
    if (isDown() || hasHover(this)) {
        left += textWidth + (g_rightPadding - 4.5) / 2.0;
        PaintUtils::drawMenuDownArrow(&p, QPointF(left, 16), opt);
    }
}

QSize DevicesButton::minimumSizeHint() const
{
    return QSize(150, 24);
}

QSize DevicesButton::sizeHint() const
{
    return recomputeSizeHint();
}

QSize DevicesButton::recomputeSizeHint() const
{
    int computedWidth = g_leftPadding +
            g_iconSize.width() +
            g_spacing +
            fontMetrics().horizontalAdvance(tr(g_iconText)) +
            g_spacing +
            g_arrowSize.width() +
            g_spacing +
            g_iconSize.width() +
            g_spacing +
            fontMetrics().horizontalAdvance(text()) +
            g_rightPadding;
    return QSize(qMin(computedWidth, 270), 24);
}