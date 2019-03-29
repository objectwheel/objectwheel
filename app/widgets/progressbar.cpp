#include <progressbar.h>
#include <QPainter>

ProgressBar::ProgressBar(QWidget *parent) : QProgressBar(parent)
  , m_indeterminate(false)
{
    resize(sizeHint());
    setTextVisible(false);
    setIndeterminate(false);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

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
    m_settings.indeterminateColor = "#45ffffff";
    m_settings.borderRadius = height() / 2.0;
}

bool ProgressBar::isIndeterminate() const
{
    return m_indeterminate;
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

QSize ProgressBar::sizeHint() const
{
    return {100, 7};
}

QSize ProgressBar::minimumSizeHint() const
{
    return {12, 7};
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

    painter.setPen(m_settings.borderColor);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(QRectF(rect()), m_settings.borderRadius, m_settings.borderRadius);
}
