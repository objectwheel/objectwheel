#include <loadingbar.h>

#include <QTimer>
#include <QPainter>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QTextBlockFormat>

namespace {
    int counter;
    QImage image;
    QColor loadingColor;
    void drawText(QPainter *painter, const QString &text, const QRectF& rect);
}

LoadingBar::LoadingBar(QWidget *parent) : QWidget(parent)
  , m_progress(0)
  , m_timerEnding(new QTimer(this))
  , m_timerFader(new QTimer(this))
{
    setFixedSize(QSize(481, 24));

    image = QImage(":/images/loadingbar.png");
    image.setDevicePixelRatio(devicePixelRatioF());

    m_timerFader->setInterval(60);
    m_timerEnding->setInterval(1000);

    connect(m_timerEnding, SIGNAL(timeout()), SLOT(onEndingTimeout()));
    connect(m_timerFader, SIGNAL(timeout()), SLOT(onFaderTimeout()));
}

void LoadingBar::setText(const QString& text)
{
    m_text = text;
}

void LoadingBar::busy(int progress, const QString& text)
{
    counter = 25;
    loadingColor = "#606467";

    m_text = text;
    m_progress = progress;
    m_timerEnding->stop();
    m_timerFader->stop();

    update();
}

void LoadingBar::done(const QString& text)
{
    counter = 25;
    loadingColor = "#30a8f7";

    m_text = text;
    m_progress = 100;
    m_timerFader->stop();
    m_timerEnding->start();

    update();
}

void LoadingBar::error(const QString& text)
{
    counter = 25;
    loadingColor = "#C2504B";

    m_text = text;
    m_progress = 100;
    m_timerFader->stop();
    m_timerEnding->start();

    update();
}

void LoadingBar::onEndingTimeout()
{
    m_timerEnding->stop();
    m_timerFader->start();
}

void LoadingBar::onFaderTimeout()
{
    loadingColor.setAlpha(counter * 10);

    if (counter > 0)
        counter--;
    else {
        counter = 25;
        loadingColor = "#606467";

        m_progress = 0;
        m_timerFader->stop();
    }

    update();
}

void LoadingBar::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawImage(rect(), image, image.rect());

    drawText(&painter, m_text, rect());

    QPainterPath path;
    path.addRoundedRect(0.5, 0.5, 480.0, 23.0, 3.5, 3.5);
    painter.setClipPath(path);
    painter.fillRect(QRectF{0.5, 21.5, m_progress * 4.8, 10}, loadingColor);
}

namespace {
    void drawText(QPainter *painter, const QString &text, const QRectF& rect)
    {
        QTextDocument doc;
        doc.setTextWidth(rect.width());
        doc.setDocumentMargin(0);
        doc.setIndentWidth(0);

        QTextBlockFormat bf;
        bf.setLineHeight(rect.height() - 1.5 * painter->device()->devicePixelRatioF(),
                         QTextBlockFormat::FixedHeight);
        bf.setAlignment(Qt::AlignCenter);

        QTextCharFormat cf;
        cf.setForeground(Qt::black);

        QTextCursor cursor(&doc);
        cursor.insertHtml(text);
        cursor.select(QTextCursor::Document);
        cursor.mergeBlockFormat(bf);
        cursor.mergeCharFormat(cf);

        doc.drawContents(painter, rect);
    }
}
