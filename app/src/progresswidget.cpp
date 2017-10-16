#include <progresswidget.h>
#include <global.h>
#include <fit.h>
#include <QPainter>
#include <QApplication>

#define PATH_GIF (":/resources/images/preloader.gif")
#define PATH_LOGO (":/resources/images/logo.png")
#define SIZE_GIF (QSize(fit(24), fit(24)))
#define SIZE_LOGO (QSize(fit(160), fit(80)))
#define INTERVAL_WAITEFFECT 600

using namespace Fit;

ProgressWidget::ProgressWidget(CentralWidget* parent)
    : QWidget(parent)
    , _centralWidget(parent)
    , _lastUid(-1)
{
    _centralWidget->addWidget(Screen::PROGRESS, this);
    setAutoFillBackground(true);

    QPalette p(palette());
    p.setColor(QPalette::Window, "#e0e4e7");
    p.setColor(QPalette::Text, "#2e3a41");
    setPalette(p);

    _movie.setFileName(PATH_GIF);
    _movie.setScaledSize(SIZE_GIF * qApp->devicePixelRatio());
    _movie.setSpeed(230);
    _movie.start();
    connect(&_movie, SIGNAL(frameChanged(int)), SLOT(update()));

    _waitEffectTimer.setInterval(INTERVAL_WAITEFFECT);
    connect(&_waitEffectTimer, &QTimer::timeout, [=] {
        if (_waitEffectString.size() < 1)
            _waitEffectString = ".";
        else if (_waitEffectString.size() < 2)
            _waitEffectString = "..";
        else if (_waitEffectString.size() < 3)
            _waitEffectString = "...";
        else
            _waitEffectString = "";
        update();
    });
}

void ProgressWidget::showProgress(const QString& msg)
{
    _msg = msg;
    showProgress();
}

void ProgressWidget::showProgress()
{
    _waitEffectTimer.start();
    _lastUid = _centralWidget->visibleUid();
    _centralWidget->showWidget(Screen::PROGRESS);
}

void ProgressWidget::hideProgress()
{
    if (_lastUid >= 0)
        _centralWidget->showWidget(_lastUid);
    hide();
    _waitEffectTimer.stop();
}

const QString& ProgressWidget::msg() const
{
    return _msg;
}

void ProgressWidget::setMsg(const QString& msg)
{
    _msg = msg;
}

void ProgressWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(palette().text().color());
    painter.setBrush(palette().text());

    int spacing = fit(40);
    int x = width() / 2.0;
    int y = height() - height()/1.618;
    painter.drawPixmap(x - SIZE_LOGO.width()/2.0, y - SIZE_LOGO.height()/2.0,
        SIZE_LOGO.width(), SIZE_LOGO.height(), QPixmap(PATH_LOGO));

    y += (spacing + SIZE_LOGO.height());
    painter.drawPixmap(x - SIZE_GIF.width()/2.0, y - SIZE_GIF.height()/2.0,
        SIZE_GIF.width(), SIZE_GIF.height(),  _movie.currentPixmap());

    y += (SIZE_GIF.height());
    painter.drawText(0, y - fit(10), width(), fit(20),
        Qt::AlignCenter, _msg + _waitEffectString);
}
