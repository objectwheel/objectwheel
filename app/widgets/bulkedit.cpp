#include <bulkedit.h>
#include <focuslesslineedit.h>
#include <QPainter>
#include <QLineEdit>
#include <QVBoxLayout>

#define ADJUST(x) ((x).adjusted(0.5, 0.5, -0.5, -0.5))
#define SIZE (QSize(300, 0))

BulkEdit::BulkEdit(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);

    resize(SIZE);

    /* Set color settings */
    _settings.borderColor = "#18000000";
    _settings.backgroundColor = "#12000000";
    _settings.labelColor = "#40000000";

    /* Set size settings */
    _settings.cellHeight = 35;
    _settings.borderRadius = 8;
    _settings.leftMargin = 10;
    _settings.rightMargin = 10;

    _layout->setSpacing(0);
    _layout->setContentsMargins(_settings.leftMargin, 0, _settings.rightMargin, 0);
}

void BulkEdit::add(int id, const QString& label, QWidget* widget)
{
    LineElement element;
    element.id = id;
    element.text = label;
    element.edit = widget;
    element.edit->setAttribute(Qt::WA_MacShowFocusRect, false);
    element.edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    element.edit->setStyleSheet("QLineEdit { border: none; background: transparent; }");

    _elements << element;

    _layout->addWidget(element.edit);
    setFixedHeight(_settings.cellHeight * _elements.size());
}

BulkEdit::Settings& BulkEdit::settings()
{
    return _settings;
}

void BulkEdit::triggerSettings()
{
    setFixedHeight(_settings.cellHeight * _elements.size());
    _layout->setContentsMargins(_settings.leftMargin, 0, _settings.rightMargin, 0);
    update();
}

QSize BulkEdit::sizeHint() const
{
    return SIZE;
}

void BulkEdit::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const auto& r = ADJUST(QRectF(rect()));
    auto bc = _settings.backgroundColor;
    auto bbc = _settings.borderColor;
    auto lc = _settings.labelColor;

    /* Draw background */
    painter.setPen(bbc);
    painter.setBrush(bc);
    painter.drawRoundedRect(r, _settings.borderRadius, _settings.borderRadius);

    /* Draw seperator lines */
    for (int i = 0; i < _elements.size() - 1; i++) {
        painter.drawLine(
            r.left(),
            r.top() + _settings.cellHeight * (i + 1),
            r.right(),
            r.top() + _settings.cellHeight * (i + 1)
        );
    }

    /* Draw labels */
    painter.setPen(lc);
    for (int i = 0; i < _elements.size(); i++) {
        const auto& element = _elements.at(i);
        painter.drawText(
            QRectF(
                r.left() + 10,
                r.top() + _settings.cellHeight * i,
                r.width() - 20,
                _settings.cellHeight
            ),
            element.text,
            QTextOption(Qt::AlignVCenter | Qt::AlignLeft)
        );
    }
}
