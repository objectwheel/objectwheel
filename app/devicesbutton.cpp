#include <devicesbutton.h>
#include <paintutils.h>
#include <utilsicons.h>
#include <utilityfunctions.h>

#include <QMenu>
#include <QPainter>
#include <QStyleOptionButton>

using namespace Utils;

static const int ICON_SIZE = 18;

DevicesButton::DevicesButton(QWidget *parent) : QPushButton(parent)
  , m_devicesPixmap(":/images/devices.png")
  , m_menu(new QMenu(this))
  , m_actionGroup(new QActionGroup(this))
  , m_myComputerAction(new QAction(this))
{
    m_devicesPixmap.setDevicePixelRatio(devicePixelRatioF());
    m_devicesPixmap = m_devicesPixmap.scaled(devicePixelRatioF() * ICON_SIZE,
                                             devicePixelRatioF() * ICON_SIZE,
                                             Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    m_actionGroup->setExclusive(true);
    m_myComputerAction->setText(tr("My Computer"));
    m_myComputerAction->setIcon(QIcon(":/images/mycomputer.png"));
    addDeviceAction(m_myComputerAction);
    setMenu(m_menu);

    connect(m_menu, &QMenu::triggered,
            this, &DevicesButton::onCurrentDeviceActionChange);

    setCurrentDeviceAction(m_myComputerAction);
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

    // Draw background
    QStyleOptionButton opt;
    opt.initFrom(this);
    opt.state |= isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
    PaintUtils::drawMacStyleButtonBackground(&p, opt, this);

    int left = 0;

    left += 6;
    QRectF iconRect(0, 0, ICON_SIZE, ICON_SIZE);
    iconRect.moveCenter(QRectF(rect()).center());
    iconRect.moveLeft(left);
    p.drawPixmap(iconRect, m_devicesPixmap, m_devicesPixmap.rect());

    left += 6 + iconRect.width();
    int textWidth = fontMetrics().horizontalAdvance(tr("Device"));
    p.setPen(palette().text().color());
    p.drawText(left, 0, textWidth, height(), Qt::AlignCenter, tr("Device"));

    left += 4 + textWidth;
    QPixmap nextPixmap(Icon({{QLatin1String(":/utils/images/next.png"), Theme::IconsBaseColor}}).pixmap());
    nextPixmap = nextPixmap.scaled(devicePixelRatioF() * 12, devicePixelRatioF() * 12,
                                   Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    iconRect.setSize({12, 12});
    iconRect.moveCenter(QRectF(rect()).center());
    iconRect.moveLeft(left);
    p.drawPixmap(iconRect, nextPixmap, nextPixmap.rect());

    left += 6 + iconRect.width();
    QPixmap buttonPixmap(icon().pixmap(UtilityFunctions::window(this), {ICON_SIZE, ICON_SIZE}));
    iconRect.setSize({ICON_SIZE, ICON_SIZE});
    iconRect.moveCenter(QRectF(rect()).center());
    iconRect.moveLeft(left);
    p.drawPixmap(iconRect, buttonPixmap, buttonPixmap.rect());

    left += 6 + iconRect.width();
    textWidth = fontMetrics().horizontalAdvance(text());
    p.setPen(palette().text().color());
    p.drawText(left, 0, textWidth, height(), Qt::AlignCenter, text());
}

QSize DevicesButton::sizeHint() const
{
    return QSize(200, 24);
}