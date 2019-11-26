#include <bulkedit.h>
#include <lineedit.h>
#include <QPainter>
#include <QLineEdit>
#include <QVBoxLayout>

#define ADJUST(x) ((x).adjusted(0.5, 0.5, -0.5, -0.5))

BulkEdit::BulkEdit(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    /* Set color settings */
    _settings.borderColor = "#18000000";
    _settings.backgroundColor = "#12000000";
    _settings.labelColor = "#40000000";

    /* Set size settings */
    _settings.cellHeight = 35;
    _settings.borderRadius = 8;
    _settings.leftMargin = 10;
    _settings.rightMargin = 10;

    _layout = new QVBoxLayout(this);
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
    element.edit->setStyleSheet(QStringLiteral("QLineEdit {"
                                               "  border: none;"
                                               "  background: transparent;"
                                               "  margin-left: %1;"
                                               "}").arg(fontMetrics().horizontalAdvance(label) + 15));

    _elements << element;

    _layout->addWidget(element.edit);
    updateGeometry();
    update();
}

BulkEdit::Settings& BulkEdit::settings()
{
    return _settings;
}

void BulkEdit::triggerSettings()
{
    _layout->setContentsMargins(_settings.leftMargin, 0, _settings.rightMargin, 0);
    adjustSize(); // In case we are not in a layout
    updateGeometry();
    update();
}

QSize BulkEdit::sizeHint() const
{
    return QSize(300, qMax(qreal(minimumSizeHint().height()), _elements.size() * _settings.cellHeight));
}

QSize BulkEdit::minimumSizeHint() const
{
    return QSize(300, qMax(_elements.size() * _settings.cellHeight, _settings.cellHeight));
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