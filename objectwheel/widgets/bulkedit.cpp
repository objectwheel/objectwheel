#include <bulkedit.h>
#include <lineedit.h>
#include <QPainter>
#include <QBoxLayout>

BulkEdit::BulkEdit(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_settings.borderColor = "#18000000";
    m_settings.backgroundColor = "#15000000";
    m_settings.labelColor = "#60000000";
    m_settings.cellHeight = 35;
    m_settings.borderRadius = 8;
    m_settings.leftMargin = 10;
    m_settings.rightMargin = 10;

    m_layout->setSpacing(0);
    m_layout->setContentsMargins(m_settings.leftMargin, 0, m_settings.rightMargin, 0);
}

void BulkEdit::add(int id, const QString& label, QWidget* widget)
{
    QColor c(m_settings.labelColor);
    c.setAlpha(c.alpha() * 0.6);
    QPalette p(widget->palette());
    p.setColor(QPalette::PlaceholderText, c);

    LineElement element;
    element.id = id;
    element.text = label;
    element.edit = widget;
    element.edit->setPalette(p);
    element.edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    element.edit->setStyleSheet(QStringLiteral("QLineEdit {"
                                               "  border: none;"
                                               "  background: transparent;"
                                               "  margin-left: %1;"
                                               "}").arg(fontMetrics().horizontalAdvance(label) + 15));

    m_elements.append(element);
    m_layout->addWidget(element.edit);
    updateGeometry();
    update();
}

BulkEdit::Settings& BulkEdit::settings()
{
    return m_settings;
}

void BulkEdit::triggerSettings()
{
    for (const LineElement& element : qAsConst(m_elements)) {
        QColor c(m_settings.labelColor);
        c.setAlpha(c.alpha() * 0.6);
        QPalette p(element.edit->palette());
        p.setColor(QPalette::PlaceholderText, c);
        element.edit->setPalette(p);
    }
    m_layout->setContentsMargins(m_settings.leftMargin, 0, m_settings.rightMargin, 0);
    adjustSize(); // In case we are not in a layout
    updateGeometry();
    update();
}

QSize BulkEdit::sizeHint() const
{
    return QSize(300, qMax(qreal(minimumSizeHint().height()), m_elements.size() * m_settings.cellHeight));
}

QSize BulkEdit::minimumSizeHint() const
{
    return QSize(300, qMax(m_elements.size() * m_settings.cellHeight, m_settings.cellHeight));
}

void BulkEdit::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF& r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    // Draw background
    painter.setPen(m_settings.borderColor);
    painter.setBrush(m_settings.backgroundColor);
    painter.drawRoundedRect(r, m_settings.borderRadius, m_settings.borderRadius);

    // Draw seperator lines
    for (int i = 0; i < m_elements.size() - 1; i++) {
        painter.drawLine(
            r.left(),
            r.top() + m_settings.cellHeight * (i + 1),
            r.right(),
            r.top() + m_settings.cellHeight * (i + 1)
        );
    }

    // Draw labels
    painter.setPen(m_settings.labelColor);
    for (int i = 0; i < m_elements.size(); i++) {
        painter.drawText(
            QRectF(
                r.left() + 10,
                r.top() + m_settings.cellHeight * i,
                r.width() - 20,
                m_settings.cellHeight
            ),
            m_elements.at(i).text,
            QTextOption(Qt::AlignVCenter | Qt::AlignLeft)
        );
    }
}
