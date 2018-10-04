#include <runpaneloadingbar.h>
#include <paintutils.h>

#include <QTimer>
#include <QPainter>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QStyleOption>

namespace {
    int counter;
    QColor loadingColor;
}

RunPaneLoadingBar::RunPaneLoadingBar(QWidget *parent) : QWidget(parent)
  , m_progress(0)
  , m_timerEnding(new QTimer(this))
  , m_timerFader(new QTimer(this))
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    m_timerFader->setInterval(60);
    m_timerEnding->setInterval(1000);

    connect(m_timerEnding, SIGNAL(timeout()), SLOT(onEndingTimeout()));
    connect(m_timerFader, SIGNAL(timeout()), SLOT(onFaderTimeout()));
}

void RunPaneLoadingBar::setText(const QString& text)
{
    m_text = text;
    updateGeometry();
}

void RunPaneLoadingBar::busy(int progress, const QString& text)
{
    counter = 25;
    loadingColor = "#606467";

    m_text = text;
    m_progress = progress;
    m_timerEnding->stop();
    m_timerFader->stop();

    update();
}

void RunPaneLoadingBar::done(const QString& text)
{
    counter = 25;
    loadingColor = "#30a8f7";

    m_text = text;
    m_progress = 100;
    m_timerFader->stop();
    m_timerEnding->start();

    update();
}

void RunPaneLoadingBar::error(const QString& text)
{
    counter = 25;
    loadingColor = "#C2504B";

    m_text = text;
    m_progress = 100;
    m_timerFader->stop();
    m_timerEnding->start();

    update();
}

void RunPaneLoadingBar::onEndingTimeout()
{
    m_timerEnding->stop();
    m_timerFader->start();
}

void RunPaneLoadingBar::onFaderTimeout()
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

void RunPaneLoadingBar::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(palette().buttonText().color());

    // Draw background
    QStyleOptionFrame opt;
    opt.initFrom(this);
    opt.state |= QStyle::State_Raised;
    PaintUtils::drawMacStyleButtonBackground(&p, opt, this);

    // Draw text
    QTextDocument doc;
    doc.setTextWidth(rect().width());
    doc.setDocumentMargin(0);
    doc.setIndentWidth(0);
    QTextBlockFormat bf;
    bf.setLineHeight(rect().height() - 1.5 * devicePixelRatioF(), QTextBlockFormat::FixedHeight);
    bf.setAlignment(Qt::AlignCenter);
    QTextCharFormat cf;
    cf.setForeground(palette().text());
    QTextCursor cursor(&doc);
    cursor.insertHtml(m_text);
    cursor.select(QTextCursor::Document);
    cursor.mergeBlockFormat(bf);
    cursor.mergeCharFormat(cf);
    doc.drawContents(&p, rect());

    QPainterPath path;
    path.addRoundedRect(0.5, 0.5, 480.0, 23.0, 3.5, 3.5);
    p.setClipPath(path);
    p.fillRect(QRectF{0.5, 21.5, m_progress * 4.8, 10}, loadingColor);
}

QSize RunPaneLoadingBar::sizeHint() const
{
    return QSize(480, 24);
}

QSize RunPaneLoadingBar::minimumSizeHint() const
{
    return recomputeMinimumSizeHint();
}

QSize RunPaneLoadingBar::recomputeMinimumSizeHint() const
{
    int computedWidth = fontMetrics().horizontalAdvance(m_text) + 12;
    return QSize(qMin(computedWidth, 480), 24);
}
