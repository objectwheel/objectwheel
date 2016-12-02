#include <fit.h>
#include <flatbutton.h>
#include <QGraphicsDropShadowEffect>

#define STYLE_SHEET "\
QPushButton {\
	border:0px;\
	border-radius: %1px;\
	color:rgb(%2,%3,%4);\
	background:rgb(%5,%6,%7);\
} QPushButton::pressed {\
	background:rgb(%8,%9,%10);\
} QPushButton::checked {\
	background:rgb(%11,%12,%13);\
	color:rgb(%14,%15,%16);\
} QPushButton::disabled {\
	background:rgb(%17,%18,%19);\
	color:rgb(%20,%21,%22);\
}"

using namespace Fit;

FlatButton::FlatButton(QWidget *parent)
	: QPushButton(parent)
	, m_Color("#f0f0f0")
	, m_DisabledColor("#666666")
	, m_CheckedColor("#a5adb8")
	, m_TextColor(Qt::black)
	, m_DisabledTextColor("#444444")
	, m_CheckedTextColor(Qt::white)
	, m_Shadow(new QGraphicsDropShadowEffect)
{
	m_Shadow->setBlurRadius(fit(5));
	m_Shadow->setOffset(0,fit(2));
	m_Shadow->setColor(QColor(0, 0, 0, 100));
	setGraphicsEffect(m_Shadow);

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
	return m_DisabledColor;
}

void FlatButton::setDisabledColor(const QColor& Color)
{
	m_DisabledColor = Color;
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
	return m_DisabledTextColor;
}

void FlatButton::setDisabledTextColor(const QColor& TextColor)
{
	m_DisabledTextColor = TextColor;
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

	setStyleSheet(QString(STYLE_SHEET).arg(fit(4))
				  .arg(m_TextColor.red()).arg(m_TextColor.green()).arg(m_TextColor.blue())
				  .arg(m_Color.red()).arg(m_Color.green()).arg(m_Color.blue())
				  .arg(mix.red()).arg(mix.green()).arg(mix.blue())
				  .arg(m_CheckedColor.red()).arg(m_CheckedColor.green()).arg(m_CheckedColor.blue())
				  .arg(m_CheckedTextColor.red()).arg(m_CheckedTextColor.green()).arg(m_CheckedTextColor.blue())
				  .arg(m_DisabledColor.red()).arg(m_DisabledColor.green()).arg(m_DisabledColor.blue())
				  .arg(m_DisabledTextColor.red()).arg(m_DisabledTextColor.green()).arg(m_DisabledTextColor.blue()));
}
