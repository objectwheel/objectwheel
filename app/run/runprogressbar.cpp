#include <runprogressbar.h>
#include <waitingspinnerwidget.h>
#include <paintutils.h>

#include <QtMath>
#include <QPainter>
#include <QTextDocument>
#include <QTextBlock>
#include <QStyleOption>

RunProgressBar::RunProgressBar(QWidget* parent) : QWidget(parent)
  , m_progress(0)
  , m_textFormat(Qt::AutoText)
  , m_progressVisible(true)
  , m_widerLineWidth(0)
  , m_document(new QTextDocument(this))
  , m_busyIndicator(new WaitingSpinnerWidget(this, false, false))
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    setProgressColor(QColor());
    // Since maximumTextWidth() uses width() and maximumTextWidth()
    // is used by updateLine and updateToolTip functions. And they
    // are used by setText(); If user wants to setText on the
    // RunProgressBar before the widget is shown, we must have a
    // valid, proper base size in order to make proper maximumTextWidth
    // calculations. Because QWidget::setVisible, thus adjustSize()
    // function is only called when the widget is shown.
    adjustSize();

    m_document->setIndentWidth(0);
    m_document->setDocumentMargin(0);
    m_document->setDefaultTextOption(Qt::AlignTop | Qt::AlignLeft);

    m_busyIndicator->setStyleSheet("background: transparent");
    m_busyIndicator->setColor(palette().buttonText().color());
    m_busyIndicator->setRoundness(50);
    m_busyIndicator->setMinimumTrailOpacity(15);
    m_busyIndicator->setTrailFadePercentage(100);
    m_busyIndicator->setRevolutionsPerSecond(2);
    m_busyIndicator->setNumberOfLines(11);
    m_busyIndicator->setLineLength(4);
    m_busyIndicator->setInnerRadius(3);
    m_busyIndicator->setLineWidth(1);

    m_springAnimation.setDuration(500);
    m_springAnimation.setEasingCurve(QEasingCurve::OutCirc);
    connect(&m_springAnimation, &QVariantAnimation::valueChanged,
            this, qOverload<>(&RunProgressBar::update));

    m_faderAnimation.setDuration(4000);
    m_faderAnimation.setStartValue(1.0);
    m_faderAnimation.setEndValue(0.0);
    m_faderAnimation.setEasingCurve(QEasingCurve::InQuart);
    connect(&m_faderAnimation, &QVariantAnimation::valueChanged,
            this, qOverload<>(&RunProgressBar::update));
    connect(&m_faderAnimation, &QVariantAnimation::finished,
            this, [=] { m_progressVisible = false; });
}

bool RunProgressBar::isBusy() const
{
    return m_busyIndicator->isSpinning();
}

void RunProgressBar::setBusy(bool busy)
{
    if (m_busyIndicator->isSpinning() == busy)
        return;

    if (busy)
        m_busyIndicator->start();
    else
        m_busyIndicator->stop();
}

int RunProgressBar::progress() const
{
    return m_progress;
}

void RunProgressBar::setProgress(int progress)
{
    if (progress < 0 || progress > 100) {
        qWarning("WARNING: Invalid progress range, correct usage: [0-100]");
        return;
    }

    qreal start = m_springAnimation.state() == QAbstractAnimation::Running
            ? m_springAnimation.currentValue().toReal()
            : (m_progress == 100 ? 0 : m_progress);
    m_springAnimation.stop();
    m_springAnimation.setStartValue(start);
    m_springAnimation.setEndValue(qreal(progress));
    m_springAnimation.start();

    m_progress = progress;
    m_progressVisible = true;

    if (progress == 100)
        m_faderAnimation.start();
    else if (m_faderAnimation.state() == QAbstractAnimation::Running)
        m_faderAnimation.stop();

    update();
}

Qt::TextFormat RunProgressBar::textFormat() const
{
    return m_textFormat;
}

void RunProgressBar::setTextFormat(Qt::TextFormat format)
{
    if (m_textFormat == format)
        return;

    m_textFormat = format;

    QString copy = m_text;
    if (!copy.isNull()) {
        m_text.clear();
        setText(copy);
    }
}

QString RunProgressBar::text() const
{
    return m_text;
}

void RunProgressBar::setText(const QString& text)
{
    if (m_text == text)
        return;

    m_text = text;

    updateDocument();
    updateLine();
    updateToolTip();
    updateGeometry();
    update();
}

QColor RunProgressBar::progressColor() const
{
    return m_progressColor;
}

void RunProgressBar::setProgressColor(const QColor& color)
{
    if (m_progressColor == color)
        return;

    if (color.isValid())
        m_progressColor = color;
    else
        m_progressColor = palette().buttonText().color();

    update();
}

void RunProgressBar::updateLine()
{
    m_line = QTextLine();
    QTextBlock firstBlock = m_document->firstBlock();
    if (firstBlock.isValid()) {
        if (QTextLayout* layout = firstBlock.layout()) {
            layout->beginLayout();
            m_line = layout->createLine();
            if (m_line.isValid()) {
                m_line.setLineWidth(std::numeric_limits<qreal>::max());
                m_widerLineWidth = m_line.naturalTextWidth();
                m_line.setLineWidth(maximumTextWidth());
                m_line.setPosition({0, 0});
            }
            layout->endLayout();
        }
    }
}

void RunProgressBar::updateToolTip()
{
    if (m_widerLineWidth > maximumTextWidth())
        setToolTip(text());
    else
        setToolTip({});
}

void RunProgressBar::updateDocument()
{
    m_document->clear();
    m_document->setDefaultFont(font());

    QTextCharFormat format;
    format.setForeground(palette().buttonText());

    QTextCursor cursor(m_document);
    cursor.select(QTextCursor::Document);
    cursor.mergeCharFormat(format);
    cursor.setPosition(0);

    if (m_textFormat == Qt::RichText)
        cursor.insertHtml(m_text);
    else if (m_textFormat == Qt::PlainText)
        cursor.insertText(m_text);
    else if (Qt::mightBeRichText(m_text))
        cursor.insertHtml(m_text);
    else
        cursor.insertText(m_text);
}

int RunProgressBar::paddingWidth() const
{
    return 2 * m_busyIndicator->width() + 16;
}

int RunProgressBar::maximumTextWidth() const
{
    return width() - paddingWidth();
}

QSize RunProgressBar::sizeHint() const
{
    return {480, 24};
}

QSize RunProgressBar::minimumSizeHint() const
{
    int computedWidth = paddingWidth();
    if (m_line.isValid())
        computedWidth += qCeil(m_line.naturalTextWidth());
    return QSize(qMin(computedWidth, 480), 24);
}

void RunProgressBar::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::FontChange
            || event->type() == QEvent::ApplicationFontChange
            || event->type() == QEvent::PaletteChange) {
        m_busyIndicator->setColor(palette().buttonText().color());
        updateDocument();
        updateLine();
        updateToolTip();
        updateGeometry();
        update();
    }
    QWidget::changeEvent(event);
}

void RunProgressBar::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    m_busyIndicator->move(4, height() / 2.0 - m_busyIndicator->height() / 2.0);

    if (event->size().width() - event->oldSize().width() > 0) {
        updateLine();
        updateToolTip();
    }
}

void RunProgressBar::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw background, Sunken, Raised, On, Off, MouseOver etc states
    // aren't important, we just want to paint a raised button bevel
    QStyleOptionButton option;
    option.initFrom(this);
    PaintUtils::drawPanelButtonBevel(&painter, option);

    // Draw text
    if (m_line.isValid()) {
        QString dots("...");
        qreal top = height() / 2.0 - m_line.height() / 2.0;
        qreal left = width() / 2.0 - m_line.naturalTextWidth() / 2.0;
        m_line.draw(&painter, QPointF(left, top));
        if (m_widerLineWidth > maximumTextWidth()) {
            painter.drawText(QRectF(left + m_line.horizontalAdvance(), top,
                                    fontMetrics().horizontalAdvance(dots), m_line.height()),
                             dots, Qt::AlignVCenter | Qt::AlignLeft);
        }
    }

    // Draw progress
    if (m_progressVisible) {
        QPainterPath bodyPath;
        qreal faderOpacity = m_faderAnimation.currentValue().toReal();
        qreal springProgress = m_springAnimation.currentValue().toReal() * (width() - 1.0) / 100.0;
        bodyPath.addRoundedRect(QRectF(rect()).adjusted(0.5, 1, -0.5, -1), 3.65, 3.65);
        painter.setClipPath(bodyPath);
        painter.setOpacity(m_faderAnimation.state() == QAbstractAnimation::Running ? faderOpacity : 1);
        painter.fillRect(QRectF(0.5, height() - 2.5, springProgress, 1.5), m_progressColor);
    }
}
