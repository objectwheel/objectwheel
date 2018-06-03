#include <transparentcombobox.h>

#include <utils/utilsicons.h>
#include <utils/theme/theme.h>

#include <QPainter>

TransparentComboBox::TransparentComboBox(QWidget* parent) : QComboBox(parent)
  , m_hoverOver(false)
{
    setStyleSheet("border: none;");
}

void TransparentComboBox::enterEvent(QEvent* e)
{
    m_hoverOver = true;
    update();
    QComboBox::enterEvent(e);
}

void TransparentComboBox::leaveEvent(QEvent* e)
{
    m_hoverOver = false;
    update();
    QComboBox::leaveEvent(e);
}

void TransparentComboBox::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_hoverOver)
        painter.fillRect(rect(), Utils::creatorTheme()->color(Utils::Theme::FancyToolButtonHoverColor));
    else
        painter.fillRect(rect(), Qt::transparent);

    painter.setPen(isEnabled() ? Utils::creatorTheme()->color(Utils::Theme::ComboBoxTextColor) :
                                 Utils::creatorTheme()->color(Utils::Theme::TextColorDisabled));
    painter.drawText(rect().adjusted(4, 0, -12, 0),
                     fontMetrics().elidedText(currentText(), Qt::ElideRight, width() - 16),
                     QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

    QRectF arrowRect;
    arrowRect.setSize({10.0, 10.0});
    arrowRect.moveRight(width() - 2);

    static const QPixmap up = Utils::Icon({{":/utils/images/arrowup.png",
                                            Utils::Theme::ComboBoxArrowColor}}).pixmap();
    static const QPixmap down = Utils::Icon({{":/utils/images/arrowdown.png",
                                              Utils::Theme::ComboBoxArrowColor}}).pixmap();
    static const QPixmap upDisabled = Utils::Icon({{":/utils/images/arrowup.png",
                                                    Utils::Theme::ComboBoxArrowColorDisabled}}).pixmap();
    static const QPixmap downDisabled = Utils::Icon({{":/utils/images/arrowdown.png",
                                                      Utils::Theme::ComboBoxArrowColorDisabled}}).pixmap();

    arrowRect.moveBottom(height() / 2.0);
    painter.drawPixmap(arrowRect, isEnabled() ? up : upDisabled, up.rect());

    arrowRect.moveTop(height() / 2.0);
    painter.drawPixmap(arrowRect, isEnabled() ? down : downDisabled, down.rect());
}