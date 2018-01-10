#include <switch.h>
#include <fit.h>
#include <QPainter>
#include <QVariantAnimation>

#define ADJUST(x) ((x).adjusted(3.0, 3.0, -3.0, -3.0))
#define SIZE (fit::fx(QSizeF(48, 30)).toSize())

Switch::Switch(QWidget* parent) : QAbstractButton(parent)
{
    _settings.activeBackgroundColor = "#8BBB56";
    _settings.activeBorderColor = _settings.activeBackgroundColor.darker(120);
    _settings.inactiveBackgroundColor = "#C5C9CC";
    _settings.inactiveBorderColor = _settings.inactiveBackgroundColor.darker(120);
    _settings.handleColor = "#D0D4D7";
    _settings.handleBorderColor = _settings.handleColor.darker(125);
    _settings.animationType = QEasingCurve::OutBack;
    _settings.animationDuration = 300;

    resize(SIZE);
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);

    _animation = new QVariantAnimation(this);
    _animation->setEasingCurve(_settings.animationType );
    _animation->setDuration(_settings.animationDuration);
    _animation->setEndValue(0.0);
    _animation->setStartValue(0.1);
    _animation->start();

    connect(_animation, SIGNAL(valueChanged(QVariant)), SLOT(update()));
    connect(this, SIGNAL(toggled(bool)), SLOT(handleStateChange()));
}

Switch::Settings& Switch::settings()
{
    return _settings;
}

void Switch::triggerSettings()
{
    _animation->setEasingCurve(_settings.animationType);
    _animation->setDuration(_settings.animationDuration);
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
    auto hc = _settings.handleColor;
    auto hbc = _settings.handleBorderColor;
    auto ac = _settings.activeBackgroundColor;
    auto abc = _settings.activeBorderColor;
    auto ic = _settings.inactiveBackgroundColor;
    auto ibc = _settings.inactiveBorderColor;
    auto cc = isChecked() ? ac : ic;
    auto cbc = isChecked() ? abc : ibc;

    /* Draw background circle */
    painter.setBrush(cc);
    painter.setPen(cbc);
    painter.drawRoundedRect(r, r.height() / 2.0, r.height() / 2.0);

    /* Draw the gap that appears whenever state changes */
    if (isChecked()) {
        painter.setBrush(ic);
        painter.setPen(ibc);
        auto r2 = r.adjusted(x, 0, 0, 0);
        painter.drawRoundedRect(r2, r.height() / 2.0, r.height() / 2.0);
    } else {
        painter.setBrush(ac);
        painter.setPen(abc);
        auto r2 = r.adjusted(0, 0, - r.width() + x + r.height(), 0);
        painter.drawRoundedRect(r2, r.height() / 2.0, r.height() / 2.0);
    }

    /* Draw handle shadow */
    auto sr = r.adjusted(x, fit::fx(2.5), - r.width() + x + r.height(), fit::fx(2.5));
    QLinearGradient sg(sr.topLeft(), sr.bottomLeft());
    sg.setColorAt(0, "#08000000");
    sg.setColorAt(1, "#25000000");
    painter.setPen(Qt::NoPen);
    painter.setBrush(sg);
    painter.drawRoundedRect(sr, r.height() / 2.0, r.height() / 2.0);

    /* Draw handle */
    QPen p;
    auto hr = r.adjusted(x, 0, - r.width() + x + r.height(), 0);
    QLinearGradient hg(hr.topLeft(), hr.bottomLeft());
    hg.setColorAt(0, hc.lighter(105));
    hg.setColorAt(1, hc.darker(105));
    p.setColor(hbc);
    p.setWidthF(p.widthF() + 0.2);
    painter.setPen(p);
    painter.setBrush(hg);
    painter.drawRoundedRect(hr, r.height() / 2.0, r.height() / 2.0);
}

QSize Switch::sizeHint() const
{
    return SIZE;
}
