#include <splashscreen_p.h>
#include <QResizeEvent>
#include <QLinearGradient>
#include <QPainter>
#include <fit.h>
#include <QPropertyAnimation>

#define DURATION 450

using namespace Fit;

SplashScreenPrivate::SplashScreenPrivate(QWidget *parent)
	: QWidget(parent)
	, showRatio(1.0)
{
	move(0,0);
	resize(parent->size());
	parent->installEventFilter(this);
	connect(&movie, SIGNAL(frameChanged(int)), this, SLOT(update()));
}

bool SplashScreenPrivate::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == parent() && event->type() == QEvent::Resize) {
		auto e = (QResizeEvent*)event;
		resize(e->size());
	}
	return false;
}

void SplashScreenPrivate::setLoadingDevice(QIODevice* device)
{
	movie.setDevice(device);
	movie.start();
}

float SplashScreenPrivate::getShowRatio() const
{
	return showRatio;
}

void SplashScreenPrivate::setShowRatio(float value)
{
	showRatio = value;
}

void SplashScreenPrivate::hide()
{
	QPropertyAnimation *animation = new QPropertyAnimation(this, "showRatio");
	animation->setDuration(DURATION);
	animation->setStartValue(1.0);
	animation->setEndValue(0.001);
	animation->setEasingCurve(QEasingCurve::OutExpo);
	animation->start();
	QObject::connect(animation, SIGNAL(valueChanged(QVariant)), this, SLOT(update()));
	QObject::connect(animation, &QPropertyAnimation::finished, [this]{setHidden(true);showRatio=1;});
	QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
}

void SplashScreenPrivate::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
	if (showRatio > 0.0 && showRatio < 1.0) {
		int caliber = qMax(width(), height()) / 2.0;
		QPainterPath path;
		path.addEllipse(rect().center(), caliber * showRatio, caliber * showRatio);
		painter.setClipPath(path);
	}
	painter.setPen(Qt::NoPen);
	painter.setBrush(backgroundBrush);
	painter.drawRect(rect());

	int spacing = fit(40);
	int x = width() / 2.0;
	int y = height() - height()/1.618;
	painter.drawPixmap(x - iconSize.width()/2.0, y - iconSize.height()/2.0,
					   iconSize.width(), iconSize.height(), icon.pixmap(iconSize));

	y += (spacing + iconSize.height());
	painter.drawPixmap(x - loadingSize.width()/2.0, y - loadingSize.height()/2.0,
					   loadingSize.width(), loadingSize.height(), movie.currentPixmap());

	y += (loadingSize.height());
	painter.setPen(textColor);
	painter.drawText(0, y - fit(10), width(), fit(20), Qt::AlignCenter, text);
}
