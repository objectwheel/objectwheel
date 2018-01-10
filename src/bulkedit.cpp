#include <bulkedit.h>
#include <fit.h>
#include <focuslesslineedit.h>
#include <QPainter>
#include <QLineEdit>
#include <QVBoxLayout>

#define ADJUST(x) ((x).adjusted(0.5, 0.5, -0.5, -0.5))
#define SIZE (fit::fx(QSizeF(300, 0)).toSize())

BulkEdit::BulkEdit(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);

    resize(SIZE);

    /* Set color settings */
    _settings.borderColor = "#18000000";
    _settings.backgroundColor = "#12000000";
    _settings.labelColor = "#40000000";
    _settings.textColor = "#2E3A41";

    /* Set size settings */
    _settings.cellHeight = fit::fx(35);
    _settings.borderRadius = fit::fx(8);
    _settings.leftMargin = fit::fx(10);
    _settings.rightMargin = fit::fx(10);

    _layout->setSpacing(0);
    _layout->setContentsMargins(_settings.leftMargin, 0, _settings.rightMargin, 0);
}

void BulkEdit::add(int id, const QString& label)
{
    LineElement element;
    element.id = id;
    element.text = label;
    element.edit = new QLineEdit;
    element.edit->setFrame(false);
    element.edit->setAttribute(Qt::WA_MacShowFocusRect, false);
    element.edit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    element.edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QPalette p(element.edit->palette());
    p.setColor(QPalette::Text, "#2E3A41");
    p.setColor(QPalette::Base, Qt::transparent);
    p.setColor(QPalette::Window, Qt::transparent);
    element.edit->setPalette(p);

    _elements << element;

    _layout->addWidget(element.edit);
    setFixedHeight(_settings.cellHeight * _elements.size());
}

QLineEdit* BulkEdit::get(int id)
{
    for (const auto& e : _elements)
        if (e.id == id)
            return e.edit;
    return nullptr;
}

BulkEdit::Settings& BulkEdit::settings()
{
    return _settings;
}

void BulkEdit::triggerSettings()
{
    setFixedHeight(_settings.cellHeight * _elements.size());
    _layout->setContentsMargins(_settings.leftMargin, 0, _settings.rightMargin, 0);
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

    /* Draw background */
    painter.setPen(_settings.borderColor);
    painter.setBrush(_settings.backgroundColor);
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
    painter.setPen(_settings.labelColor);
    for (int i = 0; i < _elements.size(); i++) {
        const auto& element = _elements.at(i);
        painter.drawText(
            QRectF(
                r.left() + fit::fx(10),
                r.top() + _settings.cellHeight * i,
                r.width() - fit::fx(20),
                _settings.cellHeight
            ),
            element.text,
            QTextOption(Qt::AlignVCenter | Qt::AlignLeft)
        );
    }
}
