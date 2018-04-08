#include <loadingbar.h>
#include <fit.h>
#include <dpr.h>

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
    setFixedSize(fit::fx(QSizeF(481, 24)).toSize());

    image = QImage(":/resources/images/loadingbar.png");
    image.setDevicePixelRatio(DPR);

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
    path.addRoundedRect(fit::fx(0.5), fit::fx(0.5), fit::fx(480.0), fit::fx(23.0), fit::fx(3.5), fit::fx(3.5));
    painter.setClipPath(path);
    painter.fillRect(QRectF{fit::fx(0.5), fit::fx(21.5), m_progress * fit::fx(4.8), fit::fx(10)}, loadingColor);
}

namespace {
    void drawText(QPainter *painter, const QString &text, const QRectF& rect)
    {
        QTextDocument doc;
        doc.setTextWidth(rect.width());
        doc.setDocumentMargin(0);
        doc.setIndentWidth(0);

        QTextBlockFormat bf;
        bf.setLineHeight(rect.height() - fit::fx(1.5 * DPR), QTextBlockFormat::FixedHeight);
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
