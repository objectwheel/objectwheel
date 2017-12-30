#include <loadingbar.h>
#include <QPainter>
#include <QTextDocument>
#include <QTimer>
#include <QScreen>
#include <QApplication>

#define INTERVALF     60
#define INTERVAL      2000
#define HEIGHT_LINE   (4.0 / pS->devicePixelRatio())
#define pS            (QApplication::primaryScreen())
#define PATH_BAR      (":/resources/images/loadingbar.png")
#define SIZE          (QSize(962, 48) / pS->devicePixelRatio())
#define COLOR_DONE    ("#52B548")
#define COLOR_ERROR   ("#b34b4e")
#define COLOR_DEFAULT ("#444444")
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
    setFixedSize(SIZE);
}

void LoadingBar::setText(const QString& text)
{
    _text = text;
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
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawImage(rect(), _image, _image.rect());

    QFont f;
    f.setWeight(QFont::Medium);
    f.setPixelSize(f.pixelSize() - 1);

    QTextDocument doc;
    doc.setDefaultFont(f);
    doc.setHtml(_text.prepend(tr("<font color=\"") + COLOR_TEXT + "\">"));
    doc.drawContents(&painter, rect());

    QPainterPath path;
    path.addRoundedRect(0.5, 0.5, 480, 22.5, 3.5, 3.5);
    painter.setClipPath(path);
    painter.fillRect(QRectF{0.5, 21.0, _progress * 4.8, HEIGHT_LINE}, _color);
}


