#include <loadingindicator.h>
#include <fit.h>

#include <QTimer>
#include <QPainter>
#include <QApplication>
#include <QScreen>

#define pS (QApplication::primaryScreen())

LoadingIndicator::LoadingIndicator(QWidget *parent)
    : QWidget(parent)
    , _running(true)
{
    _timer = new QTimer(this);
    _timer->setInterval(15); // For 30 fps
    _timer->start();

    setFixedSize(fit::fx(QSizeF{20, 20}).toSize());
    connect(_timer, SIGNAL(timeout()), SLOT(update()));
}

void LoadingIndicator::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    if (!_running)
        return;

    static QTransform transform;
    transform.translate(width() / 2.0, height() / 2.0);
    transform.rotate(48);
    transform.translate(-width() / 2.0, -height() / 2.0);

    QPainter p(this);
    p.setTransform(transform);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawImage(
        rect().adjusted(1, 1, -1, -1),
        _scaled,
        QRectF(
            QPointF(0, 0),
            size() * pS->devicePixelRatio()
        )
    );
}

void LoadingIndicator::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    _scaled = _image.scaled(
        size() * pS->devicePixelRatio(),
        Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation
    );
}

bool LoadingIndicator::running() const
{
    return _running;
}

void LoadingIndicator::setRunning(bool running)
{
    _running = running;

    if (running)
        _timer->start();
    else
        _timer->stop();
}

void LoadingIndicator::setImage(const QImage& image)
{
    _image = image;
    _image.setDevicePixelRatio(pS->devicePixelRatio());
    _scaled = _image.scaled(
        size() * pS->devicePixelRatio(),
        Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation
    );
}

void LoadingIndicator::start()
{
    setRunning(true);
}

void LoadingIndicator::stop()
{
    setRunning(false);
}
