#include <loadingindicator.h>
#include <fit.h>

#include <QMovie>
#include <QPainter>
#include <QApplication>
#include <QScreen>

#define pS (QApplication::primaryScreen())

using namespace Fit;

QMovie* _movie = nullptr;
bool LoadingIndicator::_running = false;

LoadingIndicator::LoadingIndicator(QWidget *parent)
    : QWidget(parent)
{
    resize(sizeHint());
    _movie = new QMovie(this);
    _movie->setFileName(":/resources/images/preloader.gif");
    _movie->setScaledSize(size() * pS->devicePixelRatio());
    _movie->setSpeed(230);
    _movie->start();
    connect(_movie, SIGNAL(frameChanged(int)), SLOT(update()));
}

QSize LoadingIndicator::sizeHint() const
{
    return QSize(fit(20), fit(20));
}

void LoadingIndicator::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    if (!_running)
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawPixmap(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5),
                 _movie->currentPixmap(),
                 QRectF(QPointF(0, 0), size() * pS->devicePixelRatio()));
}

bool LoadingIndicator::running()
{
    return _running;
}

void LoadingIndicator::setRunning(bool running)
{
    _running = running;
}
