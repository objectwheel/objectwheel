#include <devicesbutton.h>
#include <paintutils.h>
#include <utilsicons.h>
#include <utilityfunctions.h>

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
  , m_myComputerAction(new QAction(this))
{
    QPalette p(palette());
    PaintUtils::setPanelButtonPaletteDefaults(p);
    setPalette(p);

    m_actionGroup->setExclusive(true);
    m_myComputerAction->setText(tr("My Computer"));
    m_myComputerAction->setIcon(QIcon(":/images/mycomputer.png"));

    addDeviceAction(m_myComputerAction);
    setCurrentDeviceAction(m_myComputerAction);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    setMenu(m_menu);
    connect(m_menu, &QMenu::triggered,
            this, &DevicesButton::onCurrentDeviceActionChange);
}

QList<QAction*> DevicesButton::deviceActions() const
{
    return m_actionGroup->actions();
}

void DevicesButton::addDeviceAction(QAction* action)
{
    action->setCheckable(true);
    m_actionGroup->addAction(action);
    m_menu->addAction(action);
}

void DevicesButton::addDeviceActions(const QList<QAction*>& actions)
{
    for (QAction* action : actions) {
        action->setCheckable(true);
        m_actionGroup->addAction(action);
    }
    m_menu->addActions(actions);
}

void DevicesButton::insertDeviceAction(QAction* before, QAction* action)
{
    action->setCheckable(true);
    m_actionGroup->addAction(action);
    m_menu->insertAction(before, action);
}

void DevicesButton::insertDeviceActions(QAction* before, const QList<QAction*>& actions)
{
    for (QAction* action : actions) {
        action->setCheckable(true);
        m_actionGroup->addAction(action);
    }
    m_menu->insertActions(before, actions);
}

void DevicesButton::removeDeviceAction(QAction* action)
{
    m_menu->removeAction(action);
    m_actionGroup->removeAction(action);
}

void DevicesButton::setCurrentDeviceAction(QAction* action)
{
    setIcon(action->icon());
    setText(action->text());
    action->setChecked(true);
    emit currentDeviceActionChanged(action);
}

void DevicesButton::onCurrentDeviceActionChange(QAction* action)
{
    setCurrentDeviceAction(action);
    emit currentDeviceActionTriggered(action);
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