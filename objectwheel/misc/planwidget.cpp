#include <planwidget.h>
#include <utilityfunctions.h>

#include <QPainter>
#include <QGridLayout>
#include <QRadioButton>
#include <QEvent>

PlanWidget::PlanWidget(const QString& filePath, QWidget* parent) : QWidget(parent)
  , m_csvParser(filePath)
  , m_radius(4)
  , m_spacing(8)
  , m_padding(12)
  , m_columnColors {"#404040", "#A83C3F", "#61A53F", "#3566BC"}
{
    m_csvParser.parse();

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(m_spacing);
    layout->addWidget(new QWidget(this), 0, 0, 1, m_csvParser.columnCount());
    layout->setRowMinimumHeight(0, headerHeight() + rowHeight() * (m_csvParser.rowCount() - 1));
    layout->setColumnMinimumWidth(0, blockWidth());
    layout->setRowStretch(2, 1);
    layout->setColumnStretch(4, 1);

    int colWidth = columnWidth();
    for (int i = 0; i < m_csvParser.columnCount() - 1; i++) {
        auto radio = new QRadioButton(this);
        radio->setCursor(Qt::PointingHandCursor);
        radio->setAutoExclusive(true);
        radio->setText(m_csvParser.at(0, i + 1));
        layout->addWidget(radio, 1, i + 1, Qt::AlignHCenter);
        layout->setColumnMinimumWidth(i + 1, colWidth);
    }
}

int PlanWidget::radius() const
{
    return m_radius;
}

void PlanWidget::setRadius(int radius)
{
    if (m_radius != radius) {
        m_radius = radius;
        update();
    }
}

int PlanWidget::spacing() const
{
    return m_spacing;
}

void PlanWidget::setSpacing(int spacing)
{
    if (m_spacing != spacing) {
        m_spacing = spacing;
        layout()->setSpacing(m_spacing);
        updateGeometry();
        update();
    }
}

int PlanWidget::padding() const
{
    return m_padding;
}

void PlanWidget::setPadding(int padding)
{
    if (m_padding != padding) {
        m_padding = padding;
        update();
    }
}

QVector<QColor> PlanWidget::columnColors() const
{
    return m_columnColors;
}

void PlanWidget::setColumnColors(const QVector<QColor>& columnColors)
{
    m_columnColors = columnColors;
    update();
}

QString PlanWidget::selectedPlan() const
{
    for (QRadioButton* button : findChildren<QRadioButton*>()) {
        if (button->isChecked())
            return button->text();
    }
    return QString();
}

QSize PlanWidget::sizeHint() const
{
    return minimumSizeHint();
}

QSize PlanWidget::minimumSizeHint() const
{
    return QSize(blockWidth() + (m_csvParser.columnCount() - 1) * (columnWidth() + m_spacing)
                 + contentsMargins().left() + contentsMargins().right(),
                 headerHeight() + (m_csvParser.rowCount() - 1) * rowHeight()
                 + contentsMargins().top() + contentsMargins().bottom()
                 + QRadioButton().sizeHint().height() + m_spacing);
}

int PlanWidget::rowHeight() const
{
    return fontMetrics().height() + 10;
}

int PlanWidget::headerHeight() const
{
    return rowHeight() + 4;
}

int PlanWidget::blockWidth() const
{
    QFont f(font());
    f.setWeight(QFont::Medium);
    const QFontMetrics fm(f);
    int max = 0;
    for (int i = 0; i < m_csvParser.rowCount() - 1; ++i) {
        int val = fm.horizontalAdvance(m_csvParser.at(i + 1, 0)) + 2;
        if (val > max)
            max = val;
    }
    return max + 2 * m_padding;
}

int PlanWidget::columnWidth() const
{
    int max = 0;
    for (int i = 0; i < m_csvParser.rowCount() - 1; ++i) {
        for (int j = 0; j < m_csvParser.columnCount() - 1; ++j) {
            int val = fontMetrics().horizontalAdvance(m_csvParser.at(i + 1, j + 1)) + 2;
            if (val > max)
                max = val;
        }
    }
    return max + 2 * m_padding;
}

QPen PlanWidget::cosmeticPen(const QColor& color) const
{
    QPen pen(color, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    pen.setCosmetic(true);
    return pen;
}

void PlanWidget::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::FontChange
            || event->type() == QEvent::ApplicationFontChange
            || event->type() == QEvent::PaletteChange) {
        if (auto layout = static_cast<QGridLayout*>(this->layout())) {
            layout->setColumnMinimumWidth(0, blockWidth());
            layout->setRowMinimumHeight(0, headerHeight() + rowHeight() * (m_csvParser.rowCount() - 1));
            const int colWidth = columnWidth();
            for (int i = 0; i < m_csvParser.columnCount() - 1; i++)
                layout->setColumnMinimumWidth(i + 1, colWidth);
        }
        updateGeometry();
        update();
    }
    QWidget::changeEvent(event);
}

void PlanWidget::paintEvent(QPaintEvent*)
{
    const int colWidth = columnWidth();
    const int blkWidth = blockWidth();
    const QRectF& cr = contentsRect();

    // Paint Block Background
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_columnColors[0]);
    painter.drawRoundedRect(QRectF(cr.left(), cr.top() + headerHeight(), blkWidth,
                                   rowHeight() * (m_csvParser.rowCount() - 1) + 1),
                            m_radius, m_radius);

    painter.setFont(UtilityFunctions::thickerFont(font()));
    painter.setRenderHint(QPainter::Antialiasing, false);
    for (int i = 0; i < m_csvParser.rowCount() - 1; i++) {
        // Paint Block Row Lines
        if (i != m_csvParser.rowCount() - 2) {
            painter.setPen(cosmeticPen(m_columnColors[0].lighter(145)));
            painter.drawLine(QPointF(cr.left() + m_padding, cr.top() + headerHeight()
                                     + rowHeight() * (i + 1)),
                             QPointF(cr.left() + blkWidth - m_padding, cr.top() + headerHeight()
                                     + rowHeight() * (i + 1)));
        }
        // Paint Block Row Texts
        painter.setPen(palette().brightText().color());
        painter.drawText(QRectF(cr.left(), cr.top() + headerHeight()
                                + rowHeight() * i, blkWidth - m_padding, rowHeight()),
                         m_csvParser.at(i + 1, 0), Qt::AlignVCenter | Qt::AlignRight);
    }
    painter.setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < m_csvParser.columnCount() - 1; i++) {
        // Paint Column Backgrounds
        qreal left = cr.left() + blkWidth + m_spacing * (i + 1) + colWidth * i;
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_columnColors[i + 1]);
        painter.drawRoundedRect(QRectF(left, cr.top(), colWidth,
                                       rowHeight() * (m_csvParser.rowCount() - 1) + headerHeight() + 1),
                                m_radius, m_radius);
        painter.setBrush(m_columnColors[i + 1].lighter(225));
        painter.drawRoundedRect(QRectF(left + 0.5, cr.top() + headerHeight(), colWidth - 1,
                                       rowHeight() * (m_csvParser.rowCount() - 1) - 0.5 + 1),
                                m_radius - 0.5, m_radius - 0.5);
        painter.drawRect(QRectF(left + 0.5, cr.top() + headerHeight(), colWidth - 1, rowHeight()));

        // Paint Column Row Lines
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setPen(cosmeticPen(QColor(0, 0, 0, 50)));
        painter.drawLine(QPointF(left, cr.top() + headerHeight() - 0.5),
                         QPointF(left + colWidth, cr.top() + headerHeight() - 0.5));
        painter.setPen(cosmeticPen(m_columnColors[i + 1].lighter(225).darker(130)));
        for (int j = 0; j < m_csvParser.rowCount() - 2; j++) {
            painter.drawLine(QPointF(left + m_padding, cr.top() + headerHeight()
                                     + rowHeight() * (j + 1)),
                             QPointF(left + colWidth - m_padding, cr.top() + headerHeight()
                                     + rowHeight() * (j + 1)));
        }
        painter.setRenderHint(QPainter::Antialiasing, true);

        // Paint Header Text
        painter.setFont(UtilityFunctions::thickerFont(font()));
        painter.setPen(palette().brightText().color());
        painter.drawText(QRectF(left, cr.top(), colWidth, headerHeight()),
                         m_csvParser.at(0, i + 1), Qt::AlignVCenter | Qt::AlignHCenter);

        // Paint Column Row Texts
        painter.setFont(font());
        painter.setPen(palette().text().color());
        for (int j = 0; j < m_csvParser.rowCount() - 1; j++) {
            painter.drawText(QRectF(left, cr.top() + headerHeight() + rowHeight() * j, colWidth, rowHeight()),
                             m_csvParser.at(j + 1, i + 1), Qt::AlignVCenter | Qt::AlignHCenter);
        }
    }
}
