#include <progresswidget.h>
#include <fit.h>

#include <QMovie>
#include <QTimer>
#include <QPainter>
#include <QApplication>
#include <QScreen>

#define PATH_GIF  (":/resources/images/preloader.gif")
#define PATH_LOGO (":/resources/images/logo.png")
#define SIZE_GIF  (QSize(fit::fx(24), fit::fx(24)))
#define SIZE_LOGO (QSize(fit::fx(160), fit::fx(80)))
#define pS        (QApplication::primaryScreen())
#define INTERVAL_WAITEFFECT 600

const QPixmap* logoPixmap = nullptr;

ProgressWidget::ProgressWidget(QWidget* parent) : QWidget(parent)
{
    QPalette p(palette());
    p.setColor(QPalette::Window, "#e0e4e7");
    p.setColor(QPalette::Text, "#2e3a41");

    setPalette(p);
    setAutoFillBackground(true);

    logoPixmap =  new QPixmap(QPixmap(PATH_LOGO).scaled(
      SIZE_LOGO * pS->devicePixelRatio()));

    _movie = new QMovie(this);
    _movie->setFileName(PATH_GIF);
    _movie->setScaledSize(SIZE_GIF * pS->devicePixelRatio());
    _movie->setSpeed(230);
    _movie->start();
    connect(_movie, SIGNAL(frameChanged(int)), SLOT(update()));

    _waitEffectTimer = new QTimer(this);
    _waitEffectTimer->setInterval(INTERVAL_WAITEFFECT);
    connect(_waitEffectTimer, &QTimer::timeout, this, [=] {
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

ProgressWidget::~ProgressWidget()
{
    delete logoPixmap;
}

void ProgressWidget::show(const QString& text)
{
    _text = text;
    show();
}

void ProgressWidget::show()
{
    _waitEffectTimer->start();
    QWidget::show();
}

void ProgressWidget::hide()
{
    _waitEffectTimer->stop();
    QWidget::hide();
}

const QString& ProgressWidget::text() const
{
    return _text;
}

void ProgressWidget::setText(const QString& text)
{
    _text = text;
}

void ProgressWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(palette().text().color());
    painter.setBrush(palette().text());

    int spacing = fit::fx(40);
    int x = width() / 2.0;
    int y = height() - height()/1.618;
    auto rect = QRectF(x - SIZE_LOGO.width()/2.0, y - SIZE_LOGO.height()/2.0,
      SIZE_LOGO.width(), SIZE_LOGO.height());
    painter.drawPixmap(rect, *logoPixmap, QRectF(QPointF(),
      rect.size() * pS->devicePixelRatio()));

    y += (spacing + SIZE_LOGO.height());
    rect = QRectF(x - SIZE_GIF.width()/2.0, y - SIZE_GIF.height()/2.0,
      SIZE_GIF.width(), SIZE_GIF.height());
    painter.drawPixmap(rect, _movie->currentPixmap(), QRectF(QPointF(),
      rect.size() * pS->devicePixelRatio()));

    y += (SIZE_GIF.height());
    painter.drawText(0, y - fit::fx(10), width(), fit::fx(20),
                     Qt::AlignCenter, _text + _waitEffectString);
}
