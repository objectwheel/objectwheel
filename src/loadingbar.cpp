#include <loadingbar.h>
#include <fit.h>
#include <QPainter>
#include <QTextDocument>
#include <QTimer>
#include <QScreen>
#include <QApplication>

#define INTERVALF     60
#define INTERVAL      1000
#define pS            (QApplication::primaryScreen())
#define PATH_BAR      (":/resources/images/loadingbar.png")
#define SIZE          (fit::fx(QSizeF{481, 24}))
#define COLOR_DONE    ("#30a8f7")
#define COLOR_ERROR   ("#b34b4e")
#define COLOR_DEFAULT ("#9C7650")
#define COLOR_TEXT    ("#2a2d33")

static int counter = 25;

LoadingBar::LoadingBar(QWidget *parent) : QWidget(parent)
  , _progress(0)
  , _image(PATH_BAR)
  , _color(COLOR_DEFAULT)
{
    _timer = new QTimer(this);
    _timer->setInterval(INTERVAL);
    connect(_timer, SIGNAL(timeout()), SLOT(handleEnding()));

    _timerFader = new QTimer(this);
    _timerFader->setInterval(INTERVALF);
    connect(_timerFader, SIGNAL(timeout()), SLOT(handleFader()));

    _image.setDevicePixelRatio(pS->devicePixelRatio());
    _image = _image.scaled(
        (SIZE * pS->devicePixelRatio()).toSize(),
        Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation
    );

    setFixedSize(SIZE.toSize() + QSize(1, 1));
}

void LoadingBar::setText(const QString& text)
{
    _text = text;
    _text.prepend(tr("<font color=\"") + COLOR_TEXT + "\">");
}

void LoadingBar::busy(int progress, const QString& text)
{
    counter = 25;
    _timer->stop();
    _timerFader->stop();
    _color = COLOR_DEFAULT;
    _text = text;
    _progress = progress;
    update();
}

void LoadingBar::done(const QString& text)
{
    counter = 25;
    _timer->stop();
    _timerFader->stop();
    _color = COLOR_DONE;
    _timer->start();
    _text = text;
    _progress = 100;
    update();
}

void LoadingBar::error(const QString& text)
{
    counter = 25;
    _timer->stop();
    _timerFader->stop();
    _color = COLOR_ERROR;
    _timer->start();
    _text = text;
    _progress = 100;
    update();
}

void LoadingBar::handleEnding()
{
    _timer->stop();
    _timerFader->start();
}

void LoadingBar::handleFader()
{
    _color.setAlpha(counter * 10);

    if (counter > 0) {
        counter--;
    } else {
        counter = 25;
        _progress = 0;
        _color = COLOR_DEFAULT;
        _timerFader->stop();
    }

    update();
}

void LoadingBar::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.drawImage(QRectF{QPointF(), SIZE}, _image, _image.rect());

    QFont f;
    #if defined(Q_OS_WIN)
    f.setWeight(QFont::Normal);
    #else
    f.setWeight(QFont::Medium);
    #endif
    f.setPixelSize(f.pixelSize() - 1);

    QTextDocument doc;
    doc.setDefaultFont(f);
    doc.setHtml(_text);
    doc.drawContents(&painter, QRectF{QPointF(), SIZE});

    QPainterPath path;
    path.addRoundedRect(fit::fx(0.5), fit::fx(0.5), fit::fx(480.0), fit::fx(23.0), fit::fx(3.5), fit::fx(3.5));
    painter.setClipPath(path);
    painter.fillRect(QRectF{fit::fx(0.5), fit::fx(21.5), _progress * fit::fx(4.8), fit::fx(10)}, _color);
}


