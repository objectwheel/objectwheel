#include <progressbar.h>
#include <fit.h>
#include <QPainter>

#define SIZE fit::fx(QSizeF(0, 6)).toSize()
#define ADJUST(x) QRectF((x)).adjusted(0.5, 0, -0.5, 0)

ProgressBar::ProgressBar(QWidget *parent) : QProgressBar(parent)
{
    resize(SIZE);
    setTextVisible(false);
    setFixedHeight(SIZE.height());

    /* Set settings */
    m_settings.backgroundColor = "#12000000";
    m_settings.chunkColor = "#419BF9";
    m_settings.borderColor = "#40000000";
    m_settings.borderRadius = SIZE.height() / 2.0;
}

void ProgressBar::triggerSettings()
{
    update();
}

void ProgressBar::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(rect(), m_settings.borderRadius, m_settings.borderRadius);
    painter.setClipPath(path);

    qreal chunkWidth = width() * value() / (maximum() - minimum());

    painter.fillRect(rect(), m_settings.backgroundColor);
    painter.fillRect(rect().adjusted(0, 0, -width() + chunkWidth, 0), m_settings.chunkColor);
    painter.setClipping(false);

    QPen p;
    p.setWidthF(0);
    p.setColor(m_settings.borderColor);
    painter.setPen(p);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(ADJUST(rect()), m_settings.borderRadius, m_settings.borderRadius);
}
