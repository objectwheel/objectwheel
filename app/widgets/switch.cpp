#include <switch.h>
#include <QPainter>
#include <QVariantAnimation>

#define SIZE (QSize(48, 30))
#define ADJUST(x) ((x).adjusted(\
    2.5 + 0.5, 2.5 + 0.5,\
    - 2.5 - 0.5, - 2.5 - 0.5\
))

namespace {
    QColor disabledColor(const QColor& color);
}

Switch::Switch(QWidget* parent) : QAbstractButton(parent)
{
    _settings.activeBackgroundColor = "#8BBB56";
    _settings.activeBorderColor = _settings.activeBackgroundColor.darker(120);
    _settings.inactiveBackgroundColor = "#cccccc";
    _settings.inactiveBorderColor = _settings.inactiveBackgroundColor.darker(120);
    _settings.handleColor = "#d4d4d4";
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

    connect(_animation, &QVariantAnimation::valueChanged, this, qOverload<>(&Switch::update));
    connect(this, &Switch::toggled, this, &Switch::handleStateChange);
}

Switch::Settings& Switch::settings()
{
    return _settings;
}

void Switch::triggerSettings()
{
    _animation->setEasingCurve(_settings.animationType);
    _animation->setDuration(_settings.animationDuration);
    update();
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

    const auto& r = ADJUST(QRectF(rect()));
    const auto& x = _animation->currentValue().toReal();
    auto hc = _settings.handleColor;
    auto hbc = _settings.handleBorderColor;
    auto ac = _settings.activeBackgroundColor;
    auto abc = _settings.activeBorderColor;
    auto ic = _settings.inactiveBackgroundColor;
    auto ibc = _settings.inactiveBorderColor;

    if (!isEnabled()) {
        hc = disabledColor(hc);
        hbc = disabledColor(hbc);
        ac = disabledColor(ac);
        abc = disabledColor(abc);
        ic = disabledColor(ic);
        ibc = disabledColor(ibc);
    }

    const auto& cc = isChecked() ? ac : ic;
    const auto& cbc = isChecked() ? abc : ibc;

    /* Draw background circle */
    painter.setBrush(cc);
    painter.setPen(cbc);
    painter.drawRoundedRect(r, r.height() / 2.0, r.height() / 2.0);

    /* Draw the gap that appears whenever state changes */
    if (isChecked()) {
        const auto& r2 = r.adjusted(x, 0, 0, 0);
        painter.setBrush(ic);
        painter.setPen(ibc);
        painter.drawRoundedRect(r2, r.height() / 2.0, r.height() / 2.0);
    } else {
        const auto& r2 = r.adjusted(0, 0, - r.width() + x + r.height(), 0);
        painter.setBrush(ac);
        painter.setPen(abc);
        painter.drawRoundedRect(r2, r.height() / 2.0, r.height() / 2.0);
    }

    /* Draw handle shadow */
    const auto& sr = r.adjusted(
        x,
        2.5,
        - r.width() + x + r.height(),
        2.5
    );

    QLinearGradient sg(sr.topLeft(), sr.bottomLeft());
    sg.setColorAt(0, "#60000000");
    sg.setColorAt(1, "#15000000");

    painter.setBrush(sg);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(sr, r.height() / 2.0, r.height() / 2.0);

    /* Draw handle */
    QPen p;
    p.setColor(hbc);
    p.setWidthF(p.widthF() + 0.2);

    const auto& hr = r.adjusted(x, 0,
      - r.width() + x + r.height(), 0);

    QLinearGradient hg(hr.topLeft(), hr.bottomLeft());
    hg.setColorAt(0, hc.lighter(105));
    hg.setColorAt(1, hc.darker(105));

    painter.setPen(p);
    painter.setBrush(hg);
    painter.drawRoundedRect(hr, r.height() / 2.0, r.height() / 2.0);
}

QSize Switch::sizeHint() const
{
    return SIZE;
}

namespace {
    QColor disabledColor(const QColor& color)
    {
        QColor d(color);
        d.setHslF(d.hslHueF(), 0, d.lightnessF(), d.alphaF());
        return d;
    }
}
