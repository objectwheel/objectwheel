#include <devicesbutton.h>
#include <paintutils.h>
#include <utilsicons.h>
#include <utilityfunctions.h>

#include <QMenu>
#include <QPainter>
#include <QStyleOptionButton>
#include <QTimer>

using namespace Utils;

namespace {
const int g_iconSize = 18;
const int g_leftPadding = 6;
const int g_rightPadding = 12;
const int g_spacing = 3;
const int g_arrowSize = 12;
const char* g_iconText = "Devices";
}

DevicesButton::DevicesButton(QWidget *parent) : QPushButton(parent)
  , m_devicesPixmap(":/images/devices.png")
  , m_menu(new QMenu(this))
  , m_actionGroup(new QActionGroup(this))
  , m_myComputerAction(new QAction(this))
{
    m_devicesPixmap.setDevicePixelRatio(devicePixelRatioF());
    m_devicesPixmap = m_devicesPixmap.scaled(devicePixelRatioF() * g_iconSize,
                                             devicePixelRatioF() * g_iconSize,
                                             Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    m_actionGroup->setExclusive(true);
    m_myComputerAction->setText(tr("My Computer"));
    m_myComputerAction->setIcon(QIcon(":/images/mycomputer.png"));

    addDeviceAction(m_myComputerAction);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setCurrentDeviceAction(m_myComputerAction);
    setMenu(m_menu);

    QTimer::singleShot(10000, [=]{
        auto a = new QAction(this);
        a->setText(tr("My Computer Hello World Bulk"));
        a->setIcon(QIcon(":/images/mycomputer.png"));
        addDeviceAction(a);
        // setCurrentDeviceAction(a);
    });

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
    p.setBrush(palette().text());
    p.setPen(palette().text().color());
    p.setRenderHint(QPainter::Antialiasing);

    // Draw background
    QStyleOptionButton opt;
    opt.initFrom(this);
    opt.state |= isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
    PaintUtils::drawMacStyleButtonBackground(&p, opt, this);

    int left = g_leftPadding;
    QRectF iconRect(0, 0, g_iconSize, g_iconSize);
    iconRect.moveCenter(QRectF(rect()).center());
    iconRect.moveLeft(left);
    p.drawPixmap(iconRect, m_devicesPixmap, m_devicesPixmap.rect());

    left += g_iconSize + g_spacing;
    int textWidth = fontMetrics().horizontalAdvance(tr(g_iconText));
    p.drawText(left, 0, textWidth, height(), Qt::AlignCenter, tr(g_iconText));

    left += textWidth + g_spacing;
    QPixmap nextPixmap(Icon({{QLatin1String(":/utils/images/next.png"), Theme::IconsBaseColor}}).pixmap());
    nextPixmap = nextPixmap.scaled(devicePixelRatioF() * 12, devicePixelRatioF() * 12,
                                   Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    iconRect.setSize({12, 12});
    iconRect.moveCenter(QRectF(rect()).center());
    iconRect.moveLeft(left);
    p.drawPixmap(iconRect, nextPixmap, nextPixmap.rect());

    left += g_arrowSize + g_spacing;
    QPixmap buttonPixmap(icon().pixmap(UtilityFunctions::window(this), {g_iconSize, g_iconSize}));
    iconRect.setSize({g_iconSize, g_iconSize});
    iconRect.moveCenter(QRectF(rect()).center());
    iconRect.moveLeft(left);
    p.drawPixmap(iconRect, buttonPixmap, buttonPixmap.rect());

    left += g_iconSize + g_spacing;
    textWidth = qMin(width() - left - g_rightPadding, fontMetrics().horizontalAdvance(text()));
    p.drawText(left, 0, textWidth, height(), Qt::AlignVCenter | Qt::AlignLeft,
               fontMetrics().elidedText(text(), Qt::ElideRight, textWidth + 1));

    left += textWidth + (g_rightPadding - 4) / 2.0;
    QPointF bp(left, 16);
    QPointF pts[3] = {{0, 0}, {4, 0}, {2, 3}};
    pts[0] += bp;
    pts[1] += bp;
    pts[2] += bp;
    p.drawPolygon(pts, 3);
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
            g_iconSize +
            g_spacing +
            fontMetrics().horizontalAdvance(tr(g_iconText)) +
            g_spacing +
            g_arrowSize +
            g_spacing +
            g_iconSize +
            g_spacing +
            fontMetrics().horizontalAdvance(text()) +
            g_rightPadding;

    return QSize(computedWidth, 24);
}