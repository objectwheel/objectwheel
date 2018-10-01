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
{
    m_devicesPixmap.setDevicePixelRatio(devicePixelRatioF());
    m_devicesPixmap = m_devicesPixmap.scaled(devicePixelRatioF() * ICON_SIZE,
                                             devicePixelRatioF() * ICON_SIZE,
                                             Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
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
    nextPixmap = nextPixmap.scaled(devicePixelRatioF() * 12,
                                   devicePixelRatioF() * 12,
                                   Qt::IgnoreAspectRatio,
                                   Qt::SmoothTransformation);
    iconRect.setSize({12, 12});
    iconRect.moveCenter(QRectF(rect()).center());
    iconRect.moveLeft(left);
    p.drawPixmap(iconRect, nextPixmap, nextPixmap.rect());


    if (menu()->activeAction()) {
    left += 6 + iconRect.width();
    QPixmap buttonPixmap(icon().pixmap(UtilityFunctions::window(this), {ICON_SIZE, ICON_SIZE}));
    iconRect.setSize({ICON_SIZE, ICON_SIZE});
    iconRect.moveCenter(QRectF(rect()).center());
    iconRect.moveLeft(left);
    p.drawPixmap(iconRect, buttonPixmap, buttonPixmap.rect());
    }
}

QSize DevicesButton::sizeHint() const
{
    return QSize(200, 24);
}