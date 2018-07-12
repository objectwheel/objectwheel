#include <progressbar.h>
#include <QPainter>

#define SIZE QSize(0, 6)
#define ADJUST(x) QRectF((x)).adjusted(0.5, 0, -0.5, 0)

ProgressBar::ProgressBar(QWidget *parent) : QProgressBar(parent)
  , m_indeterminate(false)
{
    resize(SIZE);
    setTextVisible(false);
    setFixedHeight(SIZE.height());

    m_indeterminateAnim.setStartValue(0.0);
    m_indeterminateAnim.setEndValue(1.0);
    m_indeterminateAnim.setDuration(1400);
    m_indeterminateAnim.setLoopCount(-1);
    connect(&m_indeterminateAnim, &QVariantAnimation::valueChanged,
            this, qOverload<>(&ProgressBar::update));

    /* Set settings */
    m_settings.backgroundColor = "#12000000";
    m_settings.chunkColor = "#419BF9";
    m_settings.borderColor = "#40000000";
    m_settings.indeterminateColor = "#50ffffff";
    m_settings.borderRadius = SIZE.height() / 2.0;

    setIndeterminate(true);
}

void ProgressBar::triggerSettings()
{
    update();
}

void ProgressBar::setIndeterminate(bool indeterminate)
{
    m_indeterminate = indeterminate;

    if (m_indeterminate)
        m_indeterminateAnim.start();
    else
        m_indeterminateAnim.stop();

    update();
}

bool ProgressBar::indeterminate() const
{
    return m_indeterminate;
}

void ProgressBar::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(rect(), m_settings.borderRadius, m_settings.borderRadius);
    painter.setClipPath(path);

    if (m_indeterminate) {
        painter.fillRect(rect(), m_settings.chunkColor);
        static const qreal chunkWidth = width() / 1.5;

        QRectF leftRect(- chunkWidth + width() * m_indeterminateAnim.currentValue().toReal(), 0, chunkWidth, height());
        QRectF rightRect(width() - chunkWidth + width() * m_indeterminateAnim.currentValue().toReal(), 0, chunkWidth, height());

        QLinearGradient linearGrad(QPointF(leftRect.left(), leftRect.center().y()), QPointF(leftRect.right(), leftRect.center().y()));
        linearGrad.setColorAt(0, Qt::transparent);
        linearGrad.setColorAt(0.4, m_settings.indeterminateColor);
        linearGrad.setColorAt(0.5, m_settings.indeterminateColor);
        linearGrad.setColorAt(0.6, m_settings.indeterminateColor);
        linearGrad.setColorAt(1, Qt::transparent);

        QLinearGradient linearGrad2(QPointF(rightRect.left(), rightRect.center().y()), QPointF(rightRect.right(), rightRect.center().y()));
        linearGrad2.setColorAt(0, Qt::transparent);
        linearGrad2.setColorAt(0.4, m_settings.indeterminateColor);
        linearGrad2.setColorAt(0.5, m_settings.indeterminateColor);
        linearGrad2.setColorAt(0.6, m_settings.indeterminateColor);
        linearGrad2.setColorAt(1, Qt::transparent);

        painter.setPen(Qt::NoPen);
        painter.setBrush(linearGrad);
        painter.drawRect(leftRect);
        painter.setBrush(linearGrad2);
        painter.drawRect(rightRect);
    } else {
        qreal chunkWidth = width() * value() / (maximum() - minimum());
        painter.fillRect(rect(), m_settings.backgroundColor);
        painter.fillRect(rect().adjusted(0, 0, -width() + chunkWidth, 0), m_settings.chunkColor);
    }

    painter.setClipping(false);

    QPen p;
    p.setWidthF(0);
    p.setColor(m_settings.borderColor);
    painter.setPen(p);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(ADJUST(rect()), m_settings.borderRadius, m_settings.borderRadius);
}
