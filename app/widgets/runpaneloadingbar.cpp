#include <runpaneloadingbar.h>
#include <paintutils.h>
#include <interfacesettings.h>
#include <generalsettings.h>

#include <QTimer>
#include <QPainter>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QStyleOption>

namespace {

const int g_counterStart = 50;
int g_counter;
qreal g_barHeight = 2;
QColor g_loadingColor;

void setupDocument(const QWidget* widget, QTextDocument& doc, const QString& text)
{
    doc.setTextWidth(widget->rect().width());
    doc.setDocumentMargin(0);
    doc.setIndentWidth(0);
    QTextBlockFormat bf;
    bf.setLineHeight(widget->rect().height() - 1.5 * widget->devicePixelRatioF(), QTextBlockFormat::FixedHeight);
    bf.setAlignment(Qt::AlignCenter);
    QTextCharFormat cf;
    cf.setForeground(widget->palette().text());
    QTextCursor cursor(&doc);
    cursor.insertHtml(text);
    cursor.select(QTextCursor::Document);
    cursor.mergeBlockFormat(bf);
    cursor.mergeCharFormat(cf);
}
}

RunPaneLoadingBar::RunPaneLoadingBar(QWidget *parent) : QWidget(parent)
  , m_progress(0)
  , m_timerEnding(new QTimer(this))
  , m_timerFader(new QTimer(this))
{
    QPalette p(palette());
    PaintUtils::setPanelButtonPaletteDefaults(p, GeneralSettings::interfaceSettings()->theme == "Light");
    setPalette(p);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    m_timerFader->setInterval(60);
    m_timerEnding->setInterval(1000);

    connect(m_timerEnding, &QTimer::timeout, this, &RunPaneLoadingBar::onEndingTimeout);
    connect(m_timerFader, &QTimer::timeout, this, &RunPaneLoadingBar::onFaderTimeout);
}

void RunPaneLoadingBar::setText(const QString& text)
{
    m_text = text;
    updateGeometry();
    update();
}

void RunPaneLoadingBar::busy(int progress, const QString& text)
{
    g_counter = g_counterStart;
    g_loadingColor = palette().buttonText().color();

    m_text = text;
    m_progress = progress;
    m_timerEnding->stop();
    m_timerFader->stop();

    update();
}

void RunPaneLoadingBar::done(const QString& text)
{
    g_counter = g_counterStart;
    g_loadingColor = "#30acff";

    m_text = text;
    m_progress = 100;
    m_timerFader->stop();
    m_timerEnding->start();

    update();
}

void RunPaneLoadingBar::error(const QString& text)
{
    g_counter = g_counterStart;
    g_loadingColor = "#e05650";

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
    g_loadingColor.setAlpha((g_counter / qreal(g_counterStart)) * 255);

    if (g_counter > 0)
        g_counter--;
    else {
        g_counter = g_counterStart;
        g_loadingColor = "#606467";

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
    PaintUtils::drawPanelButtonBevel(&p, opt);

    // Draw text
    QTextDocument document;
    setupDocument(this, document, m_text);
    document.drawContents(&p, rect());

    QPainterPath bodyPath;
    bodyPath.addRoundedRect(QRectF(rect()).adjusted(0.5, 1, -0.5, -1), 3.65, 3.65);
    p.setClipPath(bodyPath);
    p.fillRect(QRectF{0.5, height() - 1 - g_barHeight,
                      m_progress * (width() - 1) / 100.0, g_barHeight}, g_loadingColor);
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
    QTextDocument document;
    setupDocument(this, document, m_text);
    int computedWidth = fontMetrics().horizontalAdvance(document.toPlainText()) + 16;
    return QSize(qMin(computedWidth, 480), 24);
}
