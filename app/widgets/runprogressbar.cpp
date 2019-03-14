#include <runprogressbar.h>
#include <waitingspinnerwidget.h>
#include <paintutils.h>

#include <QPainter>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QStyleOption>

RunProgressBar::RunProgressBar(QWidget* parent) : QWidget(parent)
  , m_progressVisible(true)
  , m_progress(0)
  , m_document(new QTextDocument(this))
  , m_busyIndicator(new WaitingSpinnerWidget(this, false, false))
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    PaintUtils::setPanelButtonPaletteDefaults(this);
    m_color = palette().buttonText().color();
    m_document->setIndentWidth(0);
    m_document->setDocumentMargin(0);

    m_busyIndicator->setStyleSheet("background: transparent;");
    m_busyIndicator->setColor(palette().buttonText().color());
    m_busyIndicator->setRoundness(50);
    m_busyIndicator->setMinimumTrailOpacity(15);
    m_busyIndicator->setTrailFadePercentage(100);
    m_busyIndicator->setRevolutionsPerSecond(2);
    m_busyIndicator->setNumberOfLines(11);
    m_busyIndicator->setLineLength(4);
    m_busyIndicator->setInnerRadius(3);
    m_busyIndicator->setLineWidth(1);

    m_springAnimation.setDuration(800);
    m_springAnimation.setEasingCurve(QEasingCurve::OutCirc);
    connect(&m_springAnimation, &QVariantAnimation::valueChanged,
            this, qOverload<>(&RunProgressBar::update));

    m_faderAnimation.setDuration(3000);
    m_faderAnimation.setStartValue(1.0);
    m_faderAnimation.setEndValue(0.0);
    m_faderAnimation.setEasingCurve(QEasingCurve::InQuart);
    connect(&m_faderAnimation, &QVariantAnimation::valueChanged,
            this, qOverload<>(&RunProgressBar::update));
    connect(&m_faderAnimation, &QVariantAnimation::finished,
            this, [=] { m_progressVisible = false; });
}

int RunProgressBar::progress() const
{
    return m_progress;
}

void RunProgressBar::setProgress(int progress)
{
    qreal start = m_springAnimation.state() == QAbstractAnimation::Running
            ? m_springAnimation.currentValue().toReal()
            : m_progress;
    m_springAnimation.stop();
    m_springAnimation.setStartValue(start);
    m_springAnimation.setEndValue(qreal(progress));
    m_springAnimation.start();

    m_progress = progress;
    m_progressVisible = true;

    if (progress > 99)
        m_faderAnimation.start();
    else if (m_faderAnimation.state() == QAbstractAnimation::Running)
        m_faderAnimation.stop();

    update();
}

QColor RunProgressBar::color() const
{
    return m_color;
}

void RunProgressBar::setColor(const QColor& color)
{
    m_color = color;
    update();
}

QString RunProgressBar::toHtml(const QByteArray& encoding) const
{
    return m_document->toHtml(encoding);
}

void RunProgressBar::setHtml(const QString& html)
{
    m_document->setHtml(html);
    updateGeometry();
    update();
}

QString RunProgressBar::toPlainText() const
{
    return m_document->toPlainText();
}

void RunProgressBar::setPlainText(const QString& text)
{
    m_document->setPlainText(text);
    updateGeometry();
    update();
}

bool RunProgressBar::isBusy() const
{
    return m_busyIndicator->isSpinning();
}

void RunProgressBar::setBusy(bool busy)
{
    if (busy)
        m_busyIndicator->start();
    else
        m_busyIndicator->stop();
}

void RunProgressBar::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    m_busyIndicator->move(4, height() / 2.0 - m_busyIndicator->height() / 2.0);
}

void RunProgressBar::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw background
    QStyleOptionFrame opt;
    opt.initFrom(this);
    opt.state |= QStyle::State_Raised;
    PaintUtils::drawPanelButtonBevel(&painter, opt);

    // Draw loading bar
    QPainterPath bodyPath;
    bodyPath.addRoundedRect(QRectF(rect()).adjusted(0.5, 1, -0.5, -1), 3.65, 3.65);
    painter.setClipPath(bodyPath);
    painter.setOpacity(m_faderAnimation.state() == QAbstractAnimation::Running
                       ? m_faderAnimation.currentValue().toReal()
                       : (m_progressVisible ? 1 : 0));
    painter.fillRect(QRectF{0.5, height() - 1 - 2.,
                            m_springAnimation.currentValue().toReal() * (width() - 1) / 100.0, 2.}, m_color);
    painter.setOpacity(1);

    // Draw text
    QTextCharFormat format;
    format.setForeground(palette().buttonText());
    QTextCursor cursor(m_document);
    cursor.select(QTextCursor::Document);
    cursor.mergeCharFormat(format);
    painter.translate((width() - m_document->size().width()) / 2.0,
                      (height() - m_document->size().height()) / 2.0);
    m_document->drawContents(&painter);
}

QSize RunProgressBar::sizeHint() const
{
    return {480, 24};
}

QSize RunProgressBar::minimumSizeHint() const
{
    return {qMin(int(m_document->size().width()) + 2 * m_busyIndicator->width() + 16, 480), 24};
}
