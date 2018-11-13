#include <buttonslice.h>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPainter>

#define SIZE (QSize(0, 30))
#define ADJUST(x) ((x).adjusted(\
    1 + 0.5, 1 + 0.5,\
    - 1 - 0.5, - 1 - 0.5\
))

namespace {
    QColor disabledColor(const QColor& color);
}

ButtonSlice::ButtonSlice(QWidget* parent) : QWidget(parent)
{
    _layout = new QHBoxLayout(this);

    resize(SIZE);

    /* Set size settings */
    _settings.cellWidth = 150;
    _settings.borderRadius = 8;

    _layout->setSpacing(0);
    _layout->setContentsMargins(0, 0, 0, 0);
    setFixedHeight(SIZE.height());
}

void ButtonSlice::add(int id, const QColor& topColor, const QColor& bottomColor)
{
    ButtonElement element;
    element.id = id;
    element.topColor = topColor;
    element.bottomColor = bottomColor;
    element.button = new QPushButton;
    element.button->setFocusPolicy(Qt::NoFocus);
    element.button->setAttribute(Qt::WA_MacShowFocusRect, false);
    element.button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    element.button->setStyleSheet(
        "QPushButton:disabled { color:#80ffffff; }"
        "QPushButton { color: white; border: none; background: transparent; }"
    );
    connect(element.button, &QPushButton::pressed, this, qOverload<>(&ButtonSlice::update));
    connect(element.button, &QPushButton::released, this, qOverload<>(&ButtonSlice::update));

    _elements << element;

    _layout->addWidget(element.button);
    setFixedWidth(_settings.cellWidth * _elements.size());
}

QPushButton* ButtonSlice::get(int id)
{
    for (const auto& e : _elements)
        if (e.id == id)
            return e.button;
    return nullptr;
}

ButtonSlice::Settings& ButtonSlice::settings()
{
    return _settings;
}

void ButtonSlice::triggerSettings()
{
    setFixedWidth(_settings.cellWidth * _elements.size());
    update();
}

QSize ButtonSlice::sizeHint() const
{
    return SIZE;
}

void ButtonSlice::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const auto& r = ADJUST(QRectF(rect()));

    /* Limit shadow region */
    const auto& sr = r.adjusted(
        0, 1,
        0, 1
    );

    QPainterPath ph;
    ph.addRoundedRect(sr, _settings.borderRadius, _settings.borderRadius);
    painter.setClipPath(ph);

    /* Draw shadow */
    QLinearGradient sg(sr.topLeft(), sr.bottomLeft());
    sg.setColorAt(0, "#60000000");
    sg.setColorAt(1, "#15000000");

    painter.setBrush(sg);
    painter.setPen(Qt::NoPen);
    painter.drawRect(sr);

    /* Limit background region */
    QPainterPath ph2;
    ph2.addRoundedRect(r, _settings.borderRadius, _settings.borderRadius);
    painter.setClipPath(ph2);

    /* Draw buttons */
    for (int i = 0; i < _elements.size(); i++) {
        const auto& e = _elements.at(i);
        const auto& br = QRectF(
            r.left() + _settings.cellWidth * i,
            r.top(),
            _settings.cellWidth,
            r.height()
        );

        QLinearGradient bg(br.topLeft(), br.bottomLeft());
        if (e.button->isEnabled()) {
            bg.setColorAt(0, (e.button->isDown() || e.button->isChecked()) ? e.topColor.darker(120) : e.topColor);
            bg.setColorAt(1, (e.button->isDown() || e.button->isChecked()) ? e.bottomColor.darker(120) : e.bottomColor);
        } else {
            bg.setColorAt(0, disabledColor(e.topColor));
            bg.setColorAt(1, disabledColor(e.bottomColor));
        }

        painter.setBrush(bg);
        painter.drawRect(br);
    }

    /* Draw seperator lines */
    painter.setPen("#20000000");
    for (int i = 0; i < _elements.size() - 1; i++) {
        painter.drawLine(
            r.left() + _settings.cellWidth * (i + 1),
            r.top(),
            r.left() + _settings.cellWidth * (i + 1),
            r.bottom()
        );
    }
}

namespace {
    QColor disabledColor(const QColor& color)
    {
        QColor d(color);
        d.setHslF(d.hslHueF(), 0, d.lightnessF(), d.alphaF());
        return d;
    }
}