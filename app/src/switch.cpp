#include <fit.h>
#include <switch.h>
#include <QPainter>
#include <QTimer>

#define DIFF (1.5)

using namespace Fit;

Switch::Switch(QWidget *parent)
	: QAbstractButton(parent)
	, m_x(fit(DIFF))
	, m_indicatorWidth(height() - 2.0 * fit(DIFF))
	, m_shifterTimer(new QTimer(this))
	, m_indicatorColor(Qt::white)
	, m_activeColor("#1e8145")
	, m_passiveColor("#c03638")
{
	setCheckable(true);
	m_shifterTimer->setInterval(10);
	connect(m_shifterTimer, SIGNAL(timeout()), this, SLOT(shiftIndicator()));
	connect(this, SIGNAL(toggled(bool)), m_shifterTimer, SLOT(start()));
}

void Switch::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.setPen(Qt::NoPen);
	p.setRenderHint(QPainter::Antialiasing);
	if (isEnabled()) {
		/* Draw background */
		p.setBrush(isChecked() ? m_activeColor : m_passiveColor);
		p.drawRoundedRect(QRectF(0, 0, width(), height()), height() / 2.0, height() / 2.0);

		/* Draw indicator shadow */
		QRectF iRect(m_x, fit(DIFF), m_indicatorWidth, m_indicatorWidth);
		QLinearGradient gradient(iRect.x()+iRect.width()/2.0,iRect.y()+iRect.height()/2.0,
								 iRect.x()+iRect.width()/2.0,iRect.y()+iRect.height());
		gradient.setColorAt(1, QColor(0,0,0,0));
		gradient.setColorAt(0, QColor(0,0,0,50));
		p.setBrush(gradient);
		p.drawEllipse(QRectF(m_x, 2.0 * fit(DIFF), m_indicatorWidth, m_indicatorWidth));

		/* Draw indicator */
		p.setBrush(m_indicatorColor);
		p.drawEllipse(QRectF(m_x, fit(DIFF), m_indicatorWidth, m_indicatorWidth));

		/* Draw background border */
		QPen pen(QColor(0,0,0,40));
		pen.setWidth(fit(DIFF));
		p.setPen(pen);
		p.setBrush(QColor(0,0,0,0));
		p.drawRoundedRect(QRectF(fit(DIFF)/2.0, fit(DIFF)/2.0, width() - fit(DIFF), height()-fit(DIFF)),
						  height() / 2.0 - fit(DIFF)/2.0, height() / 2.0 - fit(DIFF)/2.0);

	} else {
		/* Draw background */
		p.setBrush(QColor("#000000"));
		p.setOpacity(0.120);
		p.drawRoundedRect(QRectF(0, 0, width(), height()), height() / 2.0, height() / 2.0);

		/* Draw indicator */
		p.setOpacity(1.0);
		p.setBrush(QColor("#bdbdbd"));
		p.drawEllipse(QRectF(m_x, fit(DIFF), m_indicatorWidth, m_indicatorWidth));
	}
}

void Switch::resizeEvent(QResizeEvent* e)
{
	m_indicatorWidth = height() - 2.0 * fit(DIFF);
	if (isChecked() && !m_shifterTimer->isActive()) {
		m_x = width() - m_indicatorWidth - fit(DIFF);
		repaint();
	} else if (!m_shifterTimer->isActive()) {
		m_x = fit(DIFF);
	}
	QAbstractButton::resizeEvent(e);
}

void Switch::shiftIndicator()
{
	int step = (width() - m_indicatorWidth - 2 * fit(DIFF)) / 6.0;
	if (isChecked()) {
		m_x += step ? step : 1;
		if (m_x >= width() - m_indicatorWidth - fit(DIFF)) {
			m_x = width() - m_indicatorWidth - fit(DIFF);
			m_shifterTimer->stop();
		}
		update();
	} else {
		m_x -= step ? step : 1;
		if (m_x <= fit(DIFF)) {
			m_x = fit(DIFF);
			m_shifterTimer->stop();
		}
		update();
	}
}

QSize Switch::sizeHint() const
{
	return minimumSizeHint();
}

QSize Switch::minimumSizeHint() const
{
	return QSize(52, 31);
}

void Switch::setIndicatorColor(const QColor& color)
{
	m_indicatorColor = color;
}

void Switch::setActiveColor(const QColor& color)
{
	m_activeColor = color;
}

void Switch::setPassiveColor(const QColor& color)
{
	m_passiveColor = color;
}

const QColor& Switch::indicatorColor() const
{
	return m_indicatorColor;
}

const QColor& Switch::activeColor() const
{
	return m_activeColor;
}

const QColor& Switch::passiveColor() const
{
	return m_passiveColor;
}
