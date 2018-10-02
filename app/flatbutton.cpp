#include <flatbutton.h>

#include <QPainter>
#include <QtMath>

#define SIZE (QSize(22, 80))

namespace {

QRectF adjust(const QRectF& rect, bool crop)
{
    if (crop)
        return rect.adjusted(1, 1, -1, -1);
    else
        return rect;
}

QColor disabledColor(const QColor& color)
{
    QColor d(color);
    d.setHslF(d.hslHueF(), d.hslSaturationF() * 0.2, d.lightnessF(), d.alphaF());
    return d;
}
}

FlatButton::FlatButton(QWidget* parent) : QPushButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::PointingHandCursor);

    resize(SIZE);

    /* Set size settings */
    _settings.topColor = "#f0f0f0";
    _settings.bottomColor = "#ececec";
    _settings.textColor = palette().text().color();
    _settings.borderRadius = 0;
    _settings.textMargin = 2;
    _settings.iconButton = false;
    _settings.showShadow = true;
    _settings.textUnderIcon = false;
    _settings.verticalGradient = true;
}

FlatButton::Settings& FlatButton::settings()
{
    return _settings;
}

void FlatButton::triggerSettings()
{
    update();
}

void FlatButton::mouseDoubleClickEvent(QMouseEvent*)
{
    emit doubleClick();
}

void FlatButton::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const auto& r = adjust(rect(), _settings.showShadow);

    if (_settings.iconButton) {
        QImage image(qCeil(width() * devicePixelRatioF()),
                     qCeil(height() * devicePixelRatioF()),
                     QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(devicePixelRatioF());
        image.fill(Qt::transparent);
        QPainter pn(&image);
        icon().paint(
                    &pn,
                    rect(),
                    Qt::AlignLeft | Qt::AlignTop,
                    isEnabled() ? QIcon::Normal : QIcon::Disabled,
                    isChecked() ? QIcon::On : QIcon::Off
                                  );
        pn.end();

        for (int i = 0; i < image.width(); i++) {
            for (int j = 0; j < image.height(); j++) {
                if (isEnabled()) {
                    const auto& c = image.pixelColor(i, j);
                    image.setPixelColor(i, j, (isDown() || isChecked()) ? c.darker(125) : c);
                } else {
                    image.setPixelColor(i, j, disabledColor(image.pixelColor(i, j)));
                }
            }
        }
        painter.drawImage(QRectF(QPointF(), image.size() / devicePixelRatioF()), image, image.rect());
    } else {
        if (_settings.showShadow) {
            /* Limit shadow region */
            const auto& sr = r.adjusted(
                        0, 1,
                        0, 1
                        );

            QPainterPath ph;
            ph.addRoundedRect(sr, _settings.borderRadius, _settings.borderRadius);
            painter.setClipPath(ph);

            /* Draw shadow */
            QLinearGradient sg;
            sg.setColorAt(0, "#70000000");
            sg.setColorAt(1, "#20000000");

            if (_settings.verticalGradient) {
                sg.setStart(sr.topLeft());
                sg.setFinalStop(sr.bottomLeft());
            } else {
                sg.setStart(sr.topRight());
                sg.setFinalStop(sr.topLeft());
            }

            painter.setBrush(sg);
            painter.setPen(Qt::NoPen);
            painter.drawRect(sr);
        }

        /* Limit background region */
        QPainterPath ph2;
        ph2.addRoundedRect(r, _settings.borderRadius, _settings.borderRadius);
        painter.setClipPath(ph2);

        /* Draw background */
        QLinearGradient bg;
        if (_settings.verticalGradient) {
            bg.setStart(r.topLeft());
            bg.setFinalStop(r.bottomLeft());
        } else {
            bg.setStart(r.topRight());
            bg.setFinalStop(r.topLeft());
        }

        if (isEnabled()) {
            bg.setColorAt(0, (isDown() || isChecked()) ? _settings.topColor.darker(125) : _settings.topColor);
            bg.setColorAt(1, (isDown() || isChecked()) ? _settings.bottomColor.darker(125) : _settings.bottomColor);
        } else {
            bg.setColorAt(0, disabledColor(_settings.topColor));
            bg.setColorAt(1, disabledColor(_settings.bottomColor));
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(bg);
        painter.drawRect(r);

        /* Draw icon */
        QRectF ir;
        if (!icon().isNull()) {
            ir = QRectF(QPointF(), iconSize());
            ir.moveCenter(r.center());
            if (!text().isEmpty()) {
                if (_settings.textUnderIcon) {
                    int th = fontMetrics().height();
                    ir.moveTo(ir.x(), (r.height() - ir.height() - th - _settings.textMargin) / 2.0);
                } else {
                    int tw = fontMetrics().horizontalAdvance(text());
                    ir.moveTo((r.width() - ir.width() - tw - _settings.textMargin) / 2.0, ir.y());
                }
            }
            icon().paint(
                        &painter,
                        ir.toRect(),
                        Qt::AlignLeft | Qt::AlignTop,
                        isEnabled() ? QIcon::Normal : QIcon::Disabled,
                        isChecked() ? QIcon::On : QIcon::Off
                                      );
        }

        /* Draw text */
        painter.setPen(_settings.textColor);
        if (!text().isEmpty()) {
            if (icon().isNull()) {
                painter.drawText(r, text(), QTextOption(Qt::AlignCenter));
            } else {
                if (_settings.textUnderIcon) {
                    int th = fontMetrics().height();
                    QTextOption op(Qt::AlignTop | Qt::AlignHCenter);
                    op.setWrapMode(QTextOption::NoWrap);
                    painter.drawText(QRectF(r.left(), ir.bottom() + _settings.textMargin, r.width(), th), text(), op);
                } else {
                    int tw = fontMetrics().horizontalAdvance(text());
                    QTextOption op(Qt::AlignVCenter | Qt::AlignLeft);
                    op.setWrapMode(QTextOption::NoWrap);
                    painter.drawText(QRectF(ir.right() + _settings.textMargin, r.top(), tw, r.height()), text(), op);
                }
            }
        }
    }
}

QSize FlatButton::sizeHint() const
{
    return SIZE;
}