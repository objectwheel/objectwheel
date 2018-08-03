#include <toolbutton.h>
#include <wfw.h>

#include <utils/theme/theme.h>

#include <QPainter>
#include <QPixmap>

ToolButton::ToolButton(QWidget* parent) : QAbstractButton(parent)
  , m_hoverOver(false)
{
    setIconSize({16, 16});
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

QSize ToolButton::sizeHint() const
{
    return {24, 24};
}

QSize ToolButton::minimumSizeHint() const
{
    return {24, 24};
}

void ToolButton::enterEvent(QEvent* e)
{
    m_hoverOver = true;
    update();
    QAbstractButton::enterEvent(e);
}

void ToolButton::leaveEvent(QEvent* e)
{
    m_hoverOver = false;
    update();
    QAbstractButton::leaveEvent(e);
}

void ToolButton::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (isDown() || (isCheckable() && isChecked())) {
        painter.fillRect(rect(), Utils::creatorTheme()->color(
                             Utils::Theme::FancyToolButtonSelectedColor));
    } else if (m_hoverOver) {
        painter.fillRect(rect(), Utils::creatorTheme()->color(
                             Utils::Theme::FancyToolButtonHoverColor));
    } else {
        painter.fillRect(rect(), Qt::transparent);
    }

    QRectF iconRect({}, QSizeF{iconSize()});
    iconRect.moveCenter(QRectF(rect()).center());

    if (iconSize().width() < width() && iconSize().height() < height()) {
        if (isEnabled()) {
            painter.drawPixmap(iconRect, icon().pixmap(wfw(this), iconSize(), QIcon::Normal),
                               QRectF{{}, QSizeF(iconSize()) * devicePixelRatioF()});
        } else {
            painter.drawPixmap(iconRect, icon().pixmap(wfw(this), iconSize(), QIcon::Disabled),
                               QRectF{{}, QSizeF(iconSize()) * devicePixelRatioF()});
        }
    }
}
