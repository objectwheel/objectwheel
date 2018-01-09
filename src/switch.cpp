#include <switch.h>
#include <fit.h>
#include <QPainter>
#include <QVariantAnimation>

#define ADJUST(x) ((x).adjusted(3.0, 3.0, -3.0, -3.0))

Switch::Switch(QWidget *parent) : QAbstractButton(parent)
{
    QPalette p(palette());
    p.setColor(QPalette::Active, backgroundRole(), "#8BBB56");
    p.setColor(QPalette::Inactive, backgroundRole(), "#C5C9CC");
    p.setColor(foregroundRole(), "#D0D4D7");

    setPalette(p);
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    resize(fit::fx(QSizeF(48, 30)).toSize());

    _animation = new QVariantAnimation(this);
    _animation->setEasingCurve(QEasingCurve::OutBack);
    _animation->setDuration(300);
    _animation->setEndValue(0.0);
    _animation->setStartValue(0.1);
    _animation->start();

    connect(_animation, SIGNAL(valueChanged(QVariant)), SLOT(update()));
    connect(this, SIGNAL(toggled(bool)), SLOT(handleStateChange()));
}

void Switch::handleStateChange()
{
    auto r = ADJUST(QRectF(rect()));
    _animation->stop();
    _animation->setStartValue(_animation->currentValue().toReal());
    _animation->setEndValue(isChecked() ? r.width() - r.height() : 0);
    _animation->start();
}

void Switch::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    auto r = ADJUST(QRectF(rect()));
    auto x = _animation->currentValue().toReal();
    auto hc = palette().color(foregroundRole());
    auto ac = palette().color(QPalette::Active, backgroundRole());
    auto ic = palette().color(QPalette::Inactive, backgroundRole());
    auto bc = palette().color(isChecked() ?
        QPalette::Active : QPalette::Inactive,
        backgroundRole()
    );

    /* Draw background circle */
    painter.setBrush(bc);
    painter.setPen(bc.darker(120));
    painter.drawRoundedRect(r, r.height() / 2.0, r.height() / 2.0);

    /* Draw the gap that appears whenever state changes */
    if (isChecked()) {
        painter.setBrush(ic);
        painter.setPen(ic.darker(120));
        auto r2 = r.adjusted(x, 0, 0, 0);
        painter.drawRoundedRect(r2, r.height() / 2.0, r.height() / 2.0);
    } else {
        painter.setBrush(ac);
        painter.setPen(ac.darker(120));
        auto r2 = r.adjusted(0, 0, - r.width() + x + r.height(), 0);
        painter.drawRoundedRect(r2, r.height() / 2.0, r.height() / 2.0);
    }

    /* Draw handle shadow */
    auto sr = r.adjusted(x, fit::fx(2.5), - r.width() + x + r.height(), fit::fx(2.5));
    QLinearGradient sg(sr.topLeft(), sr.bottomLeft());
    sg.setColorAt(0, "#10000000");
    sg.setColorAt(1, "#30000000");
    painter.setPen(Qt::NoPen);
    painter.setBrush(sg);
    painter.drawRoundedRect(sr, r.height() / 2.0, r.height() / 2.0);

    /* Draw handle */
    QPen p;
    auto hr = r.adjusted(x, 0, - r.width() + x + r.height(), 0);
    QLinearGradient hg(hr.topLeft(), hr.bottomLeft());
    hg.setColorAt(0, hc.lighter(105));
    hg.setColorAt(1, hc.darker(105));
    p.setColor(hc.darker(125));
    p.setWidthF(p.widthF() + 0.2);
    painter.setPen(p);
    painter.setBrush(hg);
    painter.drawRoundedRect(hr, r.height() / 2.0, r.height() / 2.0);
}
