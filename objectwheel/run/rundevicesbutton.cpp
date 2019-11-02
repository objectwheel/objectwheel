#include <rundevicesbutton.h>
#include <utilityfunctions.h>
#include <paintutils.h>

#include <QMenu>
#include <QPainter>
#include <QStyleOption>

RunDevicesButton::RunDevicesButton(QWidget* parent) : ToolTipWidget<QPushButton>(parent)
  , m_menu(new ToolTipWidget<QMenu>(this))
{
    setMenu(m_menu);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

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
    for (QAction* action : m_menu->actions()) {
        if (UtilityFunctions::deviceUid(action) == uid) {
            m_menu->setIcon(action->icon());
            m_menu->setTitle(action->text());
            setToolTip(action->toolTip());
            setChecked(action);
            updateGeometry();
            update();
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
    QStyleOptionButton option;
    initStyleOption(&option);

    int left = LEFT_PADDING;
    int textWidth = fontMetrics().horizontalAdvance(text());
    QIcon::Mode iconMode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
    QIcon::State iconState = option.state & QStyle::State_On ? QIcon::On : QIcon::Off;
    const QColor& textColor = option.state & QStyle::State_Sunken
            ? palette().buttonText().color().darker()
            : palette().buttonText().color();

    QPen arrowPen(textColor);
    arrowPen.setWidthF(1.3);
    arrowPen.setCapStyle(Qt::RoundCap);
    arrowPen.setJoinStyle(Qt::MiterJoin);

    // Draw background
    style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &option, &painter, this);

    // Draw icon
    QPixmap pixmap(PaintUtils::pixmap(icon(), iconSize(), this, iconMode, iconState));
    if (option.state & QStyle::State_Sunken)
        pixmap = PaintUtils::renderOverlaidPixmap(pixmap, "#30000000");
    QRect ir(left, 0, iconSize().width(), height());
    int w = pixmap.width() / pixmap.devicePixelRatioF();
    int h = pixmap.height() / pixmap.devicePixelRatioF();
    QPoint point(ir.x() + ir.width() / 2 - w / 2,
                 ir.y() + ir.height() / 2 - h / 2);
    painter.drawPixmap(style()->visualPos(layoutDirection(), ir, point), pixmap);

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
    pixmap = PaintUtils::pixmap(m_menu->icon(), iconSize(), this, iconMode, iconState);
    if (option.state & QStyle::State_Sunken)
        pixmap = PaintUtils::renderOverlaidPixmap(pixmap, "#30000000");
    ir = QRect(left, 0, iconSize().width(), height());
    w = pixmap.width() / pixmap.devicePixelRatioF();
    h = pixmap.height() / pixmap.devicePixelRatioF();
    point = QPoint(ir.x() + ir.width() / 2 - w / 2,
                   ir.y() + ir.height() / 2 - h / 2);
    painter.drawPixmap(style()->visualPos(layoutDirection(), ir, point), pixmap);

    // Draw device name
    left += iconSize().width() + SPACING;
    textWidth = qMin(width() - left - RIGHT_PADDING, fontMetrics().horizontalAdvance(m_menu->title()));
    painter.setPen(textColor);
    painter.drawText(left, 0, textWidth, height(), Qt::AlignVCenter | Qt::AlignLeft,
                     fontMetrics().elidedText(m_menu->title(), Qt::ElideRight, textWidth + 1));
}
