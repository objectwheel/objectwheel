#include <flatbutton.h>
#include <fit.h>
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QtMath>

#define pS QApplication::primaryScreen()
#define SIZE (fit::fx(QSizeF(22, 80)).toSize())
#define ADJUST(x) ((x).adjusted(\
    fit::fx(1) + 0.5, fit::fx(1) + 0.5,\
    - fit::fx(1) - 0.5, - fit::fx(1) - 0.5\
))

FlatButton::FlatButton(QWidget* parent) : QPushButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
	setCursor(Qt::PointingHandCursor);

    resize(SIZE);

    /* Set size settings */
    _settings.topColor = "#f0f0f0";
    _settings.bottomColor = "#e0e4e7";
    _settings.textColor = "#2E3A41";
    _settings.iconButton = false;
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

    const auto& r = ADJUST(QRectF(rect()));

    if (_settings.iconButton) {
        QImage image(qCeil(width() * pS->devicePixelRatio()), qCeil(height() * pS->devicePixelRatio()), QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(pS->devicePixelRatio());
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
                    image.setPixelColor(i, j, (isDown() || isChecked()) ? c.darker(120) : c);
                } else {
                    const auto& g = qGray(image.pixelColor(i, j).rgb());
                    image.setPixelColor(i, j, QColor(g, g, g, image.pixelColor(i, j).alpha()));
                }
            }
        }
        painter.drawImage(QRectF(QPointF(), image.size() / pS->devicePixelRatio()), image, image.rect());
    } else {
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

        /* Draw background */
        QLinearGradient bg(r.topLeft(), r.bottomLeft());
        if (isEnabled()) {
            bg.setColorAt(0, (isDown() || isChecked()) ? _settings.topColor.darker(120) : _settings.topColor);
            bg.setColorAt(1, (isDown() || isChecked()) ? _settings.bottomColor.darker(120) : _settings.bottomColor);
        } else {
            const auto& t = qGray(_settings.topColor.rgb());
            const auto& b = qGray(_settings.bottomColor.rgb());
            bg.setColorAt(0, QColor(t, t, t, _settings.topColor.alpha()));
            bg.setColorAt(1, QColor(b, b, b, _settings.bottomColor.alpha()));
        }

        painter.setBrush(bg);
        painter.drawRect(r);

        /* Draw icon */
        QRectF ir;
        if (!icon().isNull()) {
            ir = QRectF(QPointF(), iconSize());
            ir.moveCenter(r.center());
            if (!text().isEmpty()) {
                int tw = QFontMetrics(QFont()).width(text());
                ir.moveTo((r.width() - ir.width() - tw - fit::fx(2)) / 2.0, ir.y());
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
                int tw = QFontMetrics(QFont()).width(text());
                QTextOption op(Qt::AlignVCenter | Qt::AlignLeft);
                op.setWrapMode(QTextOption::NoWrap);
                painter.drawText(QRectF(ir.right() + fit::fx(2), r.top(), tw, r.height()), text(), op);
            }
        }
    }
}

QSize FlatButton::sizeHint() const
{
    return SIZE;
}
