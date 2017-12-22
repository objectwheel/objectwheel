#include <progresswidget.h>
#include <fit.h>

#include <QMovie>
#include <QTimer>
#include <QPainter>
#include <QApplication>
#include <QScreen>

#define PATH_GIF  (":/resources/images/loader.gif")
#define PATH_LOGO (":/resources/images/logo.png")
#define SIZE_GIF  (QSize(fit::fx(28), fit::fx(28)))
#define SIZE_LOGO (QSize(fit::fx(160), fit::fx(80)))
#define pS        (QApplication::primaryScreen())
#define INTERVAL_WAITEFFECT 800

static QPixmap* logoPixmap;

ProgressWidget::ProgressWidget(QWidget* parent) : QWidget(parent)
{
    QPalette p(palette());
    p.setColor(QPalette::Window, "#e0e4e7");
    p.setColor(QPalette::Text, "#2e3a41");

    setPalette(p);
    setAutoFillBackground(true);

    static auto px(
        QPixmap(PATH_LOGO).scaled(
            SIZE_LOGO * pS->devicePixelRatio(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );
    logoPixmap = &px;

    _movie = new QMovie(this);
    _movie->setFileName(PATH_GIF);
    _movie->setScaledSize(SIZE_GIF * pS->devicePixelRatio());
    _movie->setSpeed(130);
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

void ProgressWidget::show(const QString& text, QWidget* parent)
{
    _text = text;
    show(parent);
}

void ProgressWidget::show(QWidget* parent)
{
    if (parent)
        setParent(parent);

    if (parentWidget())
        setGeometry(parentWidget()->rect());

    QWidget::show();
    raise();

    _waitEffectTimer->start();
}

void ProgressWidget::hide()
{
    _waitEffectTimer->stop();
    QWidget::hide();
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

    QRectF rect(
        x - SIZE_LOGO.width()/2.0,
        y - SIZE_LOGO.height()/2.0,
        SIZE_LOGO.width(),
        SIZE_LOGO.height()
    );

    painter.drawPixmap(
        rect, *logoPixmap,
        QRectF(
            QPointF(),
            rect.size() * pS->devicePixelRatio()
        )
    );

    y += (spacing + SIZE_LOGO.height());

    rect = QRectF(
        x - SIZE_GIF.width()/2.0,
        y - SIZE_GIF.height()/2.0,
        SIZE_GIF.width(),
        SIZE_GIF.height()
    );

    painter.drawPixmap(
        rect,
        _movie->currentPixmap(),
        QRectF(
            QPointF(),
            rect.size() * pS->devicePixelRatio()
        )
    );

    y += (spacing + SIZE_GIF.height());

    painter.drawText(
        0,
        y - fit::fx(10),
        width(),
        fit::fx(20),
        Qt::AlignCenter,
        _text + _waitEffectString
    );
}