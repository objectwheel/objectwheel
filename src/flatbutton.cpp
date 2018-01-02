#include <fit.h>
#include <flatbutton.h>
#include <QPainter>
#include <QDebug>

#define STYLE_SHEET "\
QPushButton {\
    margin-bottom: %23px;\
    border:0px;\
    border-radius: %1px;\
    color:rgb(%2,%3,%4);\
    background:qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:%5 %6, stop:1 %7);\
} QPushButton::pressed {\
    background:rgb(%8,%9,%10);\
} QPushButton::checked {\
    background:rgb(%11,%12,%13);\
    color:rgb(%14,%15,%16);\
} QPushButton::disabled {\
    background:rgb(%17,%18,%19);\
    color:rgb(%20,%21,%22);\
}"

FlatButton::FlatButton(QWidget *parent)
	: QPushButton(parent)
	, m_Color("#f0f0f0")
	, _disabledColor("#666666")
	, m_CheckedColor("#a5adb8")
	, m_TextColor(Qt::black)
	, _disabledTextColor("#444444")
	, m_CheckedTextColor(Qt::white)
	, m_Radius(fit::fx(4))
	, m_IconButton(false)
	, _down(false)
{
    setFocusPolicy(Qt::NoFocus);
	setCursor(Qt::PointingHandCursor);
	applyTheme();
}

bool FlatButton::IconButton() const
{
	return m_IconButton;
}

void FlatButton::setIconButton(bool IconButton)
{
	m_IconButton = IconButton;
}

int FlatButton::radius() const
{
	return m_Radius;
}

void FlatButton::setRadius(int value)
{
	m_Radius = value;
	applyTheme();
}

const QColor& FlatButton::Color() const
{
	return m_Color;
}

void FlatButton::setColor(const QColor& Color)
{
	m_Color = Color;
	applyTheme();
}

const QColor& FlatButton::DisabledColor() const
{
	return _disabledColor;
}

void FlatButton::setDisabledColor(const QColor& Color)
{
	_disabledColor = Color;
	applyTheme();
}

const QColor& FlatButton::CheckedColor() const
{
	return m_CheckedColor;
}

void FlatButton::setCheckedColor(const QColor& Color)
{
	m_CheckedColor = Color;
	applyTheme();
}

const QColor& FlatButton::TextColor() const
{
	return m_TextColor;
}

void FlatButton::setTextColor(const QColor& TextColor)
{
	m_TextColor = TextColor;
	applyTheme();
}

const QColor& FlatButton::DisabledTextColor() const
{
	return _disabledTextColor;
}

void FlatButton::setDisabledTextColor(const QColor& TextColor)
{
	_disabledTextColor = TextColor;
	applyTheme();
}

const QColor&FlatButton::CheckedTextColor() const
{
	return m_CheckedTextColor;
}

void FlatButton::setCheckedTextColor(const QColor& TextColor)
{
	m_CheckedTextColor = TextColor;
	applyTheme();
}

void FlatButton::applyTheme()
{
    QColor mix(	m_Color.red() * 0.75 + QColor(Qt::black).red() * 0.25,
                m_Color.green() * 0.75 + QColor(Qt::black).green() * 0.25,
                m_Color.blue() * 0.75 + QColor(Qt::black).blue() * 0.25, 255);

    setStyleSheet(QString(STYLE_SHEET).arg(m_Radius)
                  .arg(m_TextColor.red()).arg(m_TextColor.green()).arg(m_TextColor.blue())
                  .arg("0").arg(m_Color.name()).arg(m_Color.darker(115).name())
                  .arg(mix.red()).arg(mix.green()).arg(mix.blue())
                  .arg(m_CheckedColor.red()).arg(m_CheckedColor.green()).arg(m_CheckedColor.blue())
                  .arg(m_CheckedTextColor.red()).arg(m_CheckedTextColor.green()).arg(m_CheckedTextColor.blue())
                  .arg(_disabledColor.red()).arg(_disabledColor.green()).arg(_disabledColor.blue())
                  .arg(_disabledTextColor.red()).arg(_disabledTextColor.green()).arg(_disabledTextColor.blue())
                  .arg(fit::fx(1)));
}

void FlatButton::paintEvent(QPaintEvent* e)
{
	if (m_IconButton) {
		QPainter painter(this);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        QImage image = icon().pixmap(width(),height()).toImage();
		for (int i = 0; i < image.width(); i++) {
			for (int j = 0; j < image.height(); j++) {
				if (isDown() || isChecked()) {
					image.setPixelColor(i, j, image.pixelColor(i, j).darker(150));
                } else if (!isEnabled()) {
                    auto g = qGray(image.pixelColor(i, j).rgb());
                    image.setPixelColor(i, j, QColor(g, g, g, image.pixelColor(i, j).alpha()));
                } else {
					image.setPixel(i, j, image.pixel(i, j));
				}
			}
        }
        painter.drawImage(rect(), image);
    } else {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QColor(0,0,0,30));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect(), m_Radius, m_Radius);
        QPushButton::paintEvent(e);
    }
}
