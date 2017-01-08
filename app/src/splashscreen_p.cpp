#include <splashscreen_p.h>
#include <QResizeEvent>
#include <QLinearGradient>
#include <QPainter>
#include <fit.h>

using namespace Fit;

SplashScreenPrivate::SplashScreenPrivate(QWidget *parent)
	: QWidget(parent)
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

void SplashScreenPrivate::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);

	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBrush(backgroundBrush);
	painter.drawRect(rect());

//	float h = height() - iconSize.height();
//	painter.drawPixmap(rect().adjusted(width()/2.0 - iconSize.width()/2.0,
//									   h - h/1.618,
//									   -(width()/2.0 - iconSize.width()/2.0),
//									   -(h/1.618)), icon.pixmap(iconSize));

	float h2 = height() - loadingSize.height();
	painter.drawPixmap(rect().adjusted(width()/2.0 - loadingSize.width()/2.0,
									  h2/1.618 ,
									   -(width()/2.0 - loadingSize.width()/2.0),
									   -(h2 - h2/1.618)), movie.currentPixmap());

	QFont f;
	f.setPixelSize(fit(40));
	painter.setFont(f);
	float h = height() - fit(50);
	painter.setPen(Qt::white);
	painter.drawText(0, h - h/1.618, width(), fit(50), Qt::AlignCenter, "Objectwheel");


	QFont f2;
	f2.setPixelSize(fit(14));
	painter.setFont(f2);
	painter.drawText(0, h/1.618 + loadingSize.height(), width(), fit(20), Qt::AlignCenter, "Launching...");
}
