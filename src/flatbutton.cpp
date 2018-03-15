#include <flatbutton.h>
#include <fit.h>
#include <dpr.h>

#include <QPainter>
#include <QtMath>

#define SIZE (fit::fx(QSizeF(22, 80)).toSize())

namespace {
    QRectF adjust(const QRectF& rect, bool crop = false);
    QColor blendColors(const QColor& color1, const QColor& color2, qreal ratio = 0.5);
}

FlatButton::FlatButton(QWidget* parent) : QPushButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
	setCursor(Qt::PointingHandCursor);

    resize(SIZE);

    /* Set size settings */
    _settings.topColor = "#f0f0f0";
    _settings.bottomColor = "#e0e4e7";
    _settings.textColor = "#2E3A41";
    _settings.borderRadius = 0;
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
        QImage image(qCeil(width() * DPR), qCeil(height() * DPR), QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(DPR);
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
                    const auto& g = qGray(image.pixelColor(i, j).darker(110).rgb());
                    image.setPixelColor(i, j, blendColors(QColor(g, g, g, image.pixelColor(i, j).alpha()), image.pixelColor(i, j)));
                }
            }
        }
        painter.drawImage(QRectF(QPointF(), image.size() / DPR), image, image.rect());
    } else {
        if (_settings.showShadow) {
            /* Limit shadow region */
            const auto& sr = r.adjusted(
                0, fit::fx(1),
                0, fit::fx(1)
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
            const auto& t = qGray(_settings.topColor.darker(110).rgb());
            const auto& b = qGray(_settings.bottomColor.darker(110).rgb());
            bg.setColorAt(0, blendColors(QColor(t, t, t, _settings.topColor.alpha()), _settings.topColor));
            bg.setColorAt(1, blendColors(QColor(b, b, b, _settings.bottomColor.alpha()), _settings.bottomColor));
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
                    int th = QFontMetrics(QFont()).height();
                    ir.moveTo(ir.x(), (r.height() - ir.height() - th - fit::fx(2)) / 2.0);
                } else {
                    int tw = QFontMetrics(QFont()).width(text());
                    ir.moveTo((r.width() - ir.width() - tw - fit::fx(2)) / 2.0, ir.y());
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
                    int th = QFontMetrics(QFont()).height();
                    QTextOption op(Qt::AlignTop | Qt::AlignHCenter);
                    op.setWrapMode(QTextOption::NoWrap);
                    painter.drawText(QRectF(r.left(), ir.bottom() + fit::fx(2), r.width(), th), text(), op);
                } else {
                    int tw = QFontMetrics(QFont()).width(text());
                    QTextOption op(Qt::AlignVCenter | Qt::AlignLeft);
                    op.setWrapMode(QTextOption::NoWrap);
                    painter.drawText(QRectF(ir.right() + fit::fx(2), r.top(), tw, r.height()), text(), op);
                }
            }
        }
    }
}

QSize FlatButton::sizeHint() const
{
    return SIZE;
}

namespace {
    QRectF adjust(const QRectF& rect, bool crop)
    {
        if (crop)
            return rect.adjusted(
                fit::fx(1), fit::fx(1),
               -fit::fx(1), -fit::fx(1)
            );
        else
            return rect;
    }

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