#include <fit.h>
#include <flatbutton.h>
#include <QGraphicsDropShadowEffect>

FlatButton::FlatButton(QWidget *parent)
	: QPushButton(parent)
	, m_Color("#e0e0e0")
	, m_TextColor(Qt::black)
	, m_Shadow(new QGraphicsDropShadowEffect)
{
	m_Shadow->setBlurRadius(5);
	m_Shadow->setOffset(0,2);
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

const QColor& FlatButton::TextColor() const
{
	return m_TextColor;
}

void FlatButton::setTextColor(const QColor& TextColor)
{
	m_TextColor = TextColor;
	applyTheme();
}

void FlatButton::applyTheme()
{
	QColor mix(	m_Color.red() * 0.85 + QColor(Qt::black).red() * 0.15,
				m_Color.green() * 0.85 + QColor(Qt::black).green() * 0.15,
				m_Color.blue() * 0.85 + QColor(Qt::black).blue() * 0.15, 255);

	setStyleSheet(QString("QPushButton {border:0px; border-radius: %4px;"
						  "color:rgb(%5,%6,%7); background:rgb(%1,%2,%3);}"
						  "QPushButton::pressed {background:rgb(%8,%9,%10);}"
						  "QPushButton::checked {background:rgb(%8,%9,%10);}"
						  "QPushButton::disabled {color:rgb(%11,%12,%13);}")
				  .arg(m_Color.red()).arg(m_Color.green()).arg(m_Color.blue()).arg(fit(4))
				  .arg(m_TextColor.red()).arg(m_TextColor.green()).arg(m_TextColor.blue())
				  .arg(mix.red()).arg(mix.green()).arg(mix.blue())
				  .arg(mix.red()).arg(mix.green()).arg(mix.blue()));
}
