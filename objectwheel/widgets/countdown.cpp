#include <countdown.h>
#include <QTimer>
#include <QPainter>

#define ADJUST(x) ((x).adjusted(0.5, 0.5, -0.5, -0.5))

Countdown::Countdown(QWidget *parent) : QWidget(parent)
  , _second(0)
  , _timer(new QTimer(this))
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    _settings.borderColor = "#22000000";
    _settings.backgroundColor = "#12000000";
    _settings.leftDigitColor = "#7EBE5D";
    _settings.rightDigitColor = "#BE5760";
    _settings.dotColor = "#50000000";
    _settings.textColor = Qt::white;
    _settings.screwColor = "#70000000";
    _settings.lineColor = "#40000000";

    _settings.borderRadius = 4;
    _settings.digitRadius = 4;
    _settings.margins = 8;

    connect(_timer, &QTimer::timeout, this, &Countdown::decrease);
}

Countdown::Settings& Countdown::settings()
{
    return _settings;
}

void Countdown::decrease()
{
    if (_second <= 0) {
        stop();
        emit finished();
    } else
        _second--;
    update();
}

void Countdown::start(int sec)
{
    if (sec < 0)
        return;

    if (sec > 5999)
        sec = 5999;

    if (sec > 0)
        _second = sec;
    _timer->start(1000);
}

void Countdown::stop()
{
    _timer->stop();
    emit stopped();
}

void Countdown::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    qreal m = _settings.margins;                // Margins
    const auto& r = ADJUST(QRectF(rect()));     // Rectangle
    const auto& ir = r.adjusted(m, m, -m, -m);  // Inner rectangle
    qreal dw = ir.width() * 0.189;              // Digit width
    qreal sh = ir.height() * 0.05681;           // Shadow height
    qreal dh = ir.height() - sh / 2.0;          // Digit height
    qreal ds = ir.width() * 0.046;              // Digit space
    qreal scw = ir.width() * 0.0078;            // Screw width
    qreal sch = dh * 0.14;                      // Screw height
    qreal ms = ir.width() * 0.09;               // Middle space
    qreal dts = dh * 0.106;                     // Dot size
    qreal pw = ir.width() * 0.0037;             // Lines' pen width

    const auto& bc = _settings.backgroundColor;
    const auto& bbc = _settings.borderColor;
    const auto& ldc = _settings.leftDigitColor;
    const auto& rdc = _settings.rightDigitColor;
    const auto& dc = _settings.dotColor;
    const auto& tc = _settings.textColor;
    const auto& scc = _settings.screwColor;
    const auto& lc = _settings.lineColor;

    const auto& ldr1 = QRectF(ir.x() + 2 * scw, ir.y(), dw, dh);
    const auto& ldr2 = QRectF(ldr1.right() + ds, ir.y(), dw, dh);
    const auto& rdr1 = QRectF(ldr2.right() + 4 * scw + ms, ir.y(), dw, dh);
    const auto& rdr2 = QRectF(rdr1.right() + ds, ir.y(), dw, dh);

    const auto& ldsr1 = ldr1.adjusted(0, sh, 0, sh);
    const auto& ldsr2 = ldr2.adjusted(0, sh, 0, sh);
    const auto& rdsr1 = rdr2.adjusted(0, sh, 0, sh);
    const auto& rdsr2 = rdr2.adjusted(0, sh, 0, sh);

    /* Draw background */
    painter.setPen(bbc);
    painter.setBrush(bc);
    painter.drawRoundedRect(r, _settings.borderRadius, _settings.borderRadius);

    QLinearGradient sg(ldsr1.topLeft(), ldsr1.bottomLeft());
    sg.setColorAt(0, "#60000000");
    sg.setColorAt(1, "#15000000");

    /* Draw digit shadows */
    painter.setPen(Qt::NoPen);
    painter.setBrush(sg);
    painter.drawRoundedRect(ldsr1, _settings.digitRadius, _settings.digitRadius);

    sg.setStart(ldsr2.topLeft());
    sg.setFinalStop(ldsr2.bottomLeft());
    painter.setBrush(sg);
    painter.drawRoundedRect(ldsr2, _settings.digitRadius, _settings.digitRadius);

    sg.setStart(rdsr1.topLeft());
    sg.setFinalStop(rdsr2.bottomLeft());
    painter.setBrush(sg);
    painter.drawRoundedRect(rdsr1, _settings.digitRadius, _settings.digitRadius);

    sg.setStart(rdsr2.topLeft());
    sg.setFinalStop(rdsr2.bottomLeft());
    painter.setBrush(sg);
    painter.drawRoundedRect(rdsr2, _settings.digitRadius, _settings.digitRadius);

    /* Draw left digit first backgrounds */
    painter.setPen(Qt::NoPen);
    painter.setBrush(ldc.darker(125));
    painter.drawRoundedRect(ldr1.adjusted(0, sh / 2.0, 0, sh / 2.0),
                            _settings.digitRadius, _settings.digitRadius);
    painter.drawRoundedRect(ldr2.adjusted(0, sh / 2.0, 0, sh / 2.0)
                            , _settings.digitRadius, _settings.digitRadius);

    /* Draw right digit first backgrounds */
    painter.setBrush(rdc.darker(125));
    painter.drawRoundedRect(rdr1.adjusted(0, sh / 2.0, 0, sh / 2.0),
                            _settings.digitRadius, _settings.digitRadius);
    painter.drawRoundedRect(rdr2.adjusted(0, sh / 2.0, 0, sh / 2.0),
                            _settings.digitRadius, _settings.digitRadius);

    /* Draw left digit second backgrounds */
    painter.setBrush(ldc);
    painter.drawRoundedRect(ldr1, _settings.digitRadius, _settings.digitRadius);
    painter.drawRoundedRect(ldr2, _settings.digitRadius, _settings.digitRadius);

    /* Draw right digit second backgrounds */
    painter.setBrush(rdc);
    painter.drawRoundedRect(rdr1, _settings.digitRadius, _settings.digitRadius);
    painter.drawRoundedRect(rdr2, _settings.digitRadius, _settings.digitRadius);

    const auto& lscr1 = QRectF(ir.x(), ir.center().y() - sch / 2.0, scw, sch);
    const auto& lscr2 = QRectF(lscr1.right() + 2 * scw + dw, lscr1.y(), scw, sch);
    const auto& lscr3 = QRectF(ldr1.right() + ds - 2 * scw, lscr1.y(), scw, sch);
    const auto& lscr4 = QRectF(lscr3.right() + 2 * scw + dw, lscr1.y(), scw, sch);
    const auto& rscr1 = QRectF(rdr1.x() - 2 * scw, lscr1.y(), scw, sch);
    const auto& rscr2 = QRectF(rscr1.right() + 2 * scw + dw, rscr1.y(), scw, sch);
    const auto& rscr3 = QRectF(rdr1.right() + ds - 2 * scw, rscr1.y(), scw, sch);
    const auto& rscr4 = QRectF(rscr3.right() + 2 * scw + dw, rscr1.y(), scw, sch);

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

    const auto& dr1 = QRectF(ldr2.right() + 2 * scw + (ms / 2.0 - dts / 2.0),
                             ldr2.center().y() - 2 * dts, dts, dts);
    const auto& dr2 = QRectF(ldr2.right() + 2 * scw + (ms / 2.0 - dts / 2.0),
                             ldr2.center().y() + dts, dts, dts);

    /* Draw dots */
    if (_second % 2) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(dc);
        painter.drawRoundedRect(dr1, 1.0, 1.0);
        painter.drawRoundedRect(dr2, 1.0, 1.0);
    }

    QString ld, rd;
    int min = _second / 60;
    int sec = _second % 60;

    if (min < 10)
        ld = "0" + QString::number(min);
    else if (min < 100)
        ld = QString::number(min);

    rd = QString::number(sec);
    if (sec < 10)
        rd = "0" + rd;

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
    return {270, 86};
}

