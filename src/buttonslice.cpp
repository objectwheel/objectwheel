#include <buttonslice.h>
#include <fit.h>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPainter>

#define SIZE (fit::fx(QSizeF(0, 30)).toSize())
#define ADJUST(x) ((x).adjusted(\
    fit::fx(1) + 0.5, fit::fx(1) + 0.5,\
    - fit::fx(1) - 0.5, - fit::fx(1) - 0.5\
))

namespace {
    QColor blendColors(const QColor& color1, const QColor& color2, qreal ratio = 0.5);
}

ButtonSlice::ButtonSlice(QWidget* parent) : QWidget(parent)
{
    _layout = new QHBoxLayout(this);

    resize(SIZE);

    /* Set size settings */
    _settings.cellWidth = fit::fx(150);
    _settings.borderRadius = fit::fx(8);

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
    connect(element.button, SIGNAL(pressed()), SLOT(update()));
    connect(element.button, SIGNAL(released()), SLOT(update()));

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
        0, fit::fx(1),
        0, fit::fx(1)
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
            const auto& t = qGray(e.topColor.darker(110).rgb());
            const auto& b = qGray(e.bottomColor.darker(110).rgb());
            bg.setColorAt(0, blendColors(QColor(t, t, t, e.topColor.alpha()), e.topColor));
            bg.setColorAt(1, blendColors(QColor(b, b, b, e.bottomColor.alpha()), e.bottomColor));
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
    /* The parameter ratio specifies the weight of the colors when blending. A ratio
     * of 0.8 will result in a blend of 20% color1 and 80% color2.
     * A ratio of 0.5 will therefore be a 50-50 blend of the two colors. */
    QColor blendColors(const QColor& color1, const QColor& color2, qreal ratio)
    {
        int r = color1.red()*(1-ratio) + color2.red()*ratio;
        int g = color1.green()*(1-ratio) + color2.green()*ratio;
        int b = color1.blue()*(1-ratio) + color2.blue()*ratio;
        int a = color1.alpha()*(1-ratio) + color2.alpha()*ratio;
        return QColor(r, g, b, a);
    }
}