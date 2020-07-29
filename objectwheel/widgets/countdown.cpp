#include <countdown.h>
#include <QTimer>
#include <QPainter>

Countdown::Countdown(QWidget* parent) : QWidget(parent)
  , m_second(0)
  , m_timer(new QTimer(this))
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    m_settings.borderColor = QLatin1String("#28000000");
    m_settings.backgroundColor = QLatin1String("#18000000");
    m_settings.leftDigitColor = QLatin1String("#7EBE5D");
    m_settings.rightDigitColor = QLatin1String("#BE5760");
    m_settings.dotColor = QLatin1String("#55000000");
    m_settings.textColor = Qt::white;
    m_settings.screwColor = QLatin1String("#75000000");
    m_settings.lineColor = QLatin1String("#45000000");
    m_settings.borderRadius = 4;
    m_settings.digitRadius = 4;
    m_settings.margins = 8;

    connect(m_timer, &QTimer::timeout, this, &Countdown::decrease);
}

Countdown::Settings& Countdown::settings()
{
    return m_settings;
}

void Countdown::decrease()
{
    if (m_second <= 0) {
        stop();
        emit finished();
    } else {
        m_second--;
    }
    update();
}

void Countdown::start(int sec)
{
    if (sec < 0)
        return;
    if (sec > 5999)
        sec = 5999;
    if (sec > 0)
        m_second = sec;
    m_timer->start(1000);
}

void Countdown::stop()
{
    m_timer->stop();
    emit stopped();
}

void Countdown::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    qreal m = m_settings.margins;                                    // Margins
    const QRectF& r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5); // Rectangle
    const QRectF& ir = r.adjusted(m, m, -m, -m);                     // Inner rectangle
    qreal dw  = ir.width() * 0.189;                                  // Digit width
    qreal sh  = ir.height() * 0.05681;                               // Shadow height
    qreal dh  = ir.height() - sh / 2.0;                              // Digit height
    qreal ds  = ir.width() * 0.046;                                  // Digit space
    qreal scw = ir.width() * 0.0078;                                 // Screw width
    qreal sch = dh * 0.14;                                           // Screw height
    qreal ms  = ir.width() * 0.09;                                   // Middle space
    qreal dts = dh * 0.106;                                          // Dot size
    qreal pw  = ir.width() * 0.0037;                                 // Lines' pen width

    const QColor& bc  = m_settings.backgroundColor;
    const QColor& bbc = m_settings.borderColor;
    const QColor& ldc = m_settings.leftDigitColor;
    const QColor& rdc = m_settings.rightDigitColor;
    const QColor& dc  = m_settings.dotColor;
    const QColor& tc  = m_settings.textColor;
    const QColor& scc = m_settings.screwColor;
    const QColor& lc  = m_settings.lineColor;

    const QRectF& ldr1 = QRectF(ir.x() + 2 * scw, ir.y(), dw, dh);
    const QRectF& ldr2 = QRectF(ldr1.right() + ds, ir.y(), dw, dh);
    const QRectF& rdr1 = QRectF(ldr2.right() + 4 * scw + ms, ir.y(), dw, dh);
    const QRectF& rdr2 = QRectF(rdr1.right() + ds, ir.y(), dw, dh);

    const QRectF& ldsr1 = ldr1.adjusted(0, sh, 0, sh);
    const QRectF& ldsr2 = ldr2.adjusted(0, sh, 0, sh);
    const QRectF& rdsr1 = rdr2.adjusted(0, sh, 0, sh);
    const QRectF& rdsr2 = rdr2.adjusted(0, sh, 0, sh);

    /* Draw background */
    painter.setPen(bbc);
    painter.setBrush(bc);
    painter.drawRoundedRect(r, m_settings.borderRadius, m_settings.borderRadius);

    QLinearGradient sg(ldsr1.topLeft(), ldsr1.bottomLeft());
    sg.setColorAt(0, QLatin1String("#60000000"));
    sg.setColorAt(1, QLatin1String("#15000000"));

    /* Draw digit shadows */
    painter.setPen(Qt::NoPen);
    painter.setBrush(sg);
    painter.drawRoundedRect(ldsr1, m_settings.digitRadius, m_settings.digitRadius);

    sg.setStart(ldsr2.topLeft());
    sg.setFinalStop(ldsr2.bottomLeft());
    painter.setBrush(sg);
    painter.drawRoundedRect(ldsr2, m_settings.digitRadius, m_settings.digitRadius);

    sg.setStart(rdsr1.topLeft());
    sg.setFinalStop(rdsr2.bottomLeft());
    painter.setBrush(sg);
    painter.drawRoundedRect(rdsr1, m_settings.digitRadius, m_settings.digitRadius);

    sg.setStart(rdsr2.topLeft());
    sg.setFinalStop(rdsr2.bottomLeft());
    painter.setBrush(sg);
    painter.drawRoundedRect(rdsr2, m_settings.digitRadius, m_settings.digitRadius);

    /* Draw left digit first backgrounds */
    painter.setPen(Qt::NoPen);
    painter.setBrush(ldc.darker(125));
    painter.drawRoundedRect(ldr1.adjusted(0, sh / 2.0, 0, sh / 2.0),
                            m_settings.digitRadius, m_settings.digitRadius);
    painter.drawRoundedRect(ldr2.adjusted(0, sh / 2.0, 0, sh / 2.0)
                            , m_settings.digitRadius, m_settings.digitRadius);

    /* Draw right digit first backgrounds */
    painter.setBrush(rdc.darker(125));
    painter.drawRoundedRect(rdr1.adjusted(0, sh / 2.0, 0, sh / 2.0),
                            m_settings.digitRadius, m_settings.digitRadius);
    painter.drawRoundedRect(rdr2.adjusted(0, sh / 2.0, 0, sh / 2.0),
                            m_settings.digitRadius, m_settings.digitRadius);

    /* Draw left digit second backgrounds */
    painter.setBrush(ldc);
    painter.drawRoundedRect(ldr1, m_settings.digitRadius, m_settings.digitRadius);
    painter.drawRoundedRect(ldr2, m_settings.digitRadius, m_settings.digitRadius);

    /* Draw right digit second backgrounds */
    painter.setBrush(rdc);
    painter.drawRoundedRect(rdr1, m_settings.digitRadius, m_settings.digitRadius);
    painter.drawRoundedRect(rdr2, m_settings.digitRadius, m_settings.digitRadius);

    const QRectF& lscr1 = QRectF(ir.x(), ir.center().y() - sch / 2.0, scw, sch);
    const QRectF& lscr2 = QRectF(lscr1.right() + 2 * scw + dw, lscr1.y(), scw, sch);
    const QRectF& lscr3 = QRectF(ldr1.right() + ds - 2 * scw, lscr1.y(), scw, sch);
    const QRectF& lscr4 = QRectF(lscr3.right() + 2 * scw + dw, lscr1.y(), scw, sch);
    const QRectF& rscr1 = QRectF(rdr1.x() - 2 * scw, lscr1.y(), scw, sch);
    const QRectF& rscr2 = QRectF(rscr1.right() + 2 * scw + dw, rscr1.y(), scw, sch);
    const QRectF& rscr3 = QRectF(rdr1.right() + ds - 2 * scw, rscr1.y(), scw, sch);
    const QRectF& rscr4 = QRectF(rscr3.right() + 2 * scw + dw, rscr1.y(), scw, sch);

    /* Draw screws */
    painter.setBrush(scc);
    painter.drawRoundedRect(lscr1, scw / 2.0, scw / 2.0);
    painter.drawRoundedRect(lscr2, scw / 2.0, scw / 2.0);
    painter.drawRoundedRect(lscr3, scw / 2.0, scw / 2.0);
    painter.drawRoundedRect(lscr4, scw / 2.0, scw / 2.0);
    painter.drawRoundedRect(rscr1, scw / 2.0, scw / 2.0);
    painter.drawRoundedRect(rscr2, scw / 2.0, scw / 2.0);
    painter.drawRoundedRect(rscr3, scw / 2.0, scw / 2.0);
    painter.drawRoundedRect(rscr4, scw / 2.0, scw / 2.0);

    /* Draw lines */
    QPen p;
    p.setColor(lc);
    p.setWidthF(qMax(1.0, pw));
    painter.setPen(p);
    painter.drawLine(ldr1.left(), ldr1.center().y(), ldr1.right(), ldr1.center().y());
    painter.drawLine(ldr2.left(), ldr2.center().y(), ldr2.right(), ldr2.center().y());
    painter.drawLine(rdr1.left(), rdr1.center().y(), rdr1.right(), rdr1.center().y());
    painter.drawLine(rdr2.left(), rdr2.center().y(), rdr2.right(), rdr2.center().y());

    const QRectF& dr1 = QRectF(ldr2.right() + 2 * scw + (ms / 2.0 - dts / 2.0),
                               ldr2.center().y() - 2 * dts, dts, dts);
    const QRectF& dr2 = QRectF(ldr2.right() + 2 * scw + (ms / 2.0 - dts / 2.0),
                               ldr2.center().y() + dts, dts, dts);

    /* Draw dots */
    if (m_second % 2) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(dc);
        painter.drawRoundedRect(dr1, 1.0, 1.0);
        painter.drawRoundedRect(dr2, 1.0, 1.0);
    }

    QString ld, rd;
    int min = m_second / 60;
    int sec = m_second % 60;

    if (min < 10)
        ld = QLatin1Char('0') + QString::number(min);
    else if (min < 100)
        ld = QString::number(min);

    rd = QString::number(sec);
    if (sec < 10)
        rd = QLatin1Char('0') + rd;

    QFont f;
    f.setPixelSize(dh - 4);
    f.setBold(true);
    painter.setFont(f);

    painter.setPen(tc);
    painter.drawText(ldr1, ld.at(0), QTextOption(Qt::AlignCenter));
    painter.drawText(ldr2, ld.at(1), QTextOption(Qt::AlignCenter));
    painter.drawText(rdr1, rd.at(0), QTextOption(Qt::AlignCenter));
    painter.drawText(rdr2, rd.at(1), QTextOption(Qt::AlignCenter));
}

QSize Countdown::sizeHint() const
{
    return QSize(270, 86);
}
