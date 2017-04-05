#include <splashscreen_p.h>
#include <QResizeEvent>
#include <QLinearGradient>
#include <QPainter>
#include <fit.h>
#include <QPropertyAnimation>
#include <QQuickWidget>
#include <QQmlComponent>
#include <QQuickItem>

#define DURATION 1000
#define LOADING_TIME 800
#define WAIT_EFFECT_INTERVAL 800
#define SHOWER_TIMER_INTERVAL 200

using namespace Fit;

SplashScreenPrivate::SplashScreenPrivate(QWidget *parent)
	: QWidget(parent)
	, loadingWidget(this)
	, prevBusyIndicator(nullptr)
	, waitEffectTimer(this)
	, showRatio(1.0)
{
	move(0,0);
	resize(parent->size());
	parent->installEventFilter(this);
    opacityEffect.setOpacity(1);
    setGraphicsEffect(&opacityEffect);

	loadingWidget.setSource(QUrl("qrc:/resources/qmls/empty-item.qml"));
	loadingWidget.setStyleSheet("background:transparent;");
	loadingWidget.setAttribute(Qt::WA_AlwaysStackOnTop);
	loadingWidget.setClearColor(Qt::transparent);
	loadingWidget.setResizeMode(QQuickWidget::SizeRootObjectToView);

	waitEffectTimer.setInterval(WAIT_EFFECT_INTERVAL);
	connect(&waitEffectTimer, &QTimer::timeout, [=] {
		if (waitEffectString.size() < 1) {
			waitEffectString = ".";
		} else if (waitEffectString.size() < 2) {
			waitEffectString = "..";
		} else if (waitEffectString.size() < 3) {
			waitEffectString = "...";
		} else {
			waitEffectString = "";
		}
		update();
	});

    showerTimer.setInterval(17); //For 60fps
    QObject::connect(&showerTimer, &QTimer::timeout, [=]{
        static qreal step = 1.0 / (SHOWER_TIMER_INTERVAL/17.0);
        if (opacityEffect.opacity() < 1) {
            opacityEffect.setOpacity(opacityEffect.opacity() + step);
        } else {
            opacityEffect.setOpacity(1);
            showerTimer.stop();
        }
    });
}

bool SplashScreenPrivate::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == parent() && event->type() == QEvent::Resize) {
		auto e = (QResizeEvent*)event;
		resize(e->size());
	}
	return false;
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
	waitEffectTimer.stop();
	loadingWidget.hide();
    QPropertyAnimation *animation = new QPropertyAnimation(this, "showRatio");
    animation->setDuration(DURATION);
    animation->setStartValue(1.0);
    animation->setEndValue(0.001);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->start();
    QObject::connect(animation, SIGNAL(valueChanged(QVariant)), this, SLOT(update()));
    QObject::connect(animation, &QPropertyAnimation::finished, [this]{setHidden(true);showRatio=1;});
    QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
}

void SplashScreenPrivate::show(const bool animated)
{
	if (prevBusyIndicator) prevBusyIndicator->deleteLater();

	QQmlComponent component(loadingWidget.engine());
	component.setData(QByteArray().insert(0, QString("import QtQuick 2.7\n\
					  import QtQuick.Controls 1.4\n\
					  import QtQuick.Controls.Styles 1.4\n\
					  BusyIndicator {\n\
					  anchors.fill: parent\n\
					  style: BusyIndicatorStyle {\n\
					  indicator: Image {\n\
						  visible: control.running\n\
						  source: \"%1\"\n\
						  width: parent.width\n\
						  height: parent.height\n\
						  RotationAnimator on rotation {\n\
							  running: control.running\n\
							  loops: Animation.Infinite\n\
							  duration: %2\n\
							  from: 0 ; to: 360\n\
						  }\n\
					  }\n\
				  }}").arg(loadingImageFilename).arg(LOADING_TIME)), QUrl());
	prevBusyIndicator = (QQuickItem*)component.create(loadingWidget.rootContext());
	if (component.isError() || !prevBusyIndicator) qFatal("asa");
	prevBusyIndicator->setParentItem(loadingWidget.rootObject());

	waitEffectTimer.start();
	loadingWidget.show();
	showFullScreen();
	raise();

    if(animated) {
        opacityEffect.setOpacity(0);
        showerTimer.start();
    }
}

void SplashScreenPrivate::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
	if (showRatio > 0.0 && showRatio < 1.0) {
//		int caliber = qMax(width(), height()) / 2.0;
//		QPainterPath path;
//		path.addEllipse(rect().center(), caliber * showRatio, caliber * showRatio);
//		painter.setClipPath(path);
		painter.setOpacity(showRatio);
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
	loadingWidget.setGeometry(x - loadingSize.width()/2.0, y - loadingSize.height()/2.0,
							  loadingSize.width(), loadingSize.height());

	y += (loadingSize.height());
	painter.setPen(textColor);
	painter.drawText(0, y - fit(10), width(), fit(20), Qt::AlignCenter, text + waitEffectString);
}
