#include <planwidget.h>
#include <utilityfunctions.h>

#include <QPainter>
#include <QGridLayout>
#include <QRadioButton>
#include <QEvent>

static const char g_planIdentifierProperty[] = "q_PlanWidget_planIdentifierProperty";

PlanWidget::PlanWidget(QWidget* parent) : QWidget(parent)
  , m_radius(4)
  , m_spacing(6)
  , m_padding(10)
{
    auto layout = new QGridLayout(this);
    layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(m_spacing);
    layout->addWidget(new QWidget(this), 0, 0);
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

PlanInfo PlanWidget::planInfo() const
{
    return m_planInfo;
}

void PlanWidget::setPlanInfo(const PlanInfo& planInfo)
{
    m_planInfo = planInfo;

    if (auto layout = static_cast<QGridLayout*>(this->layout())) {
        for (QRadioButton* button : findChildren<QRadioButton*>())
            delete button;
        layout->addWidget(layout->takeAt(layout->indexOf(layout->itemAtPosition(0, 0)))->widget(),
                          0, 0, 1, m_planInfo.columnCount());
        layout->setColumnMinimumWidth(0, blockWidth());
        layout->setRowMinimumHeight(0, headerHeight() + rowHeight() * (m_planInfo.rowCount() - 1) + 1);
        layout->setRowStretch(2, 1);
        layout->setColumnStretch(4, 1);
        int colWidth = columnWidth();
        for (int i = 0; i < m_planInfo.columnCount() - 1; i++) {
            qint64 planIdentifier = m_planInfo.identifier(i + 1);
            auto radio = new QRadioButton(this);
            radio->setFocusPolicy(Qt::NoFocus);
            radio->setCursor(Qt::PointingHandCursor);
            radio->setAutoExclusive(true);
            radio->setText(planInfo.at(0, i + 1));
            radio->setProperty(g_planIdentifierProperty, planIdentifier);
            if (m_planInfo.defaultPlan() == planIdentifier)
                radio->setChecked(true);
            layout->addWidget(radio, 1, i + 1, Qt::AlignHCenter);
            layout->setColumnMinimumWidth(i + 1, colWidth);
            UtilityFunctions::adjustFontPixelSize(radio, 1);
            connect(radio, &QRadioButton::toggled, radio, [=] (bool checked) {
                UtilityFunctions::adjustFontWeight(radio, checked ? QFont::DemiBold : QFont::Weight(font().weight()));
            });
        }
    }

    updateGeometry();
    update();
}

qint64 PlanWidget::selectedPlan() const
{
    for (QRadioButton* button : findChildren<QRadioButton*>()) {
        if (button->isChecked())
            return button->property(g_planIdentifierProperty).toLongLong();
    }
    return 0;
}

QSize PlanWidget::sizeHint() const
{
    return minimumSizeHint();
}

QSize PlanWidget::minimumSizeHint() const
{
    return QSize(blockWidth() + (m_planInfo.columnCount() - 1) * (columnWidth() + m_spacing)
                 + contentsMargins().left() + contentsMargins().right(),
                 headerHeight() + (m_planInfo.rowCount() - 1) * rowHeight() + 1
                 + contentsMargins().top() + contentsMargins().bottom()
                 + QRadioButton().sizeHint().height() + m_spacing);
}

int PlanWidget::rowHeight() const
{
    return fontMetrics().height() + 8;
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
    for (int i = 0; i < m_planInfo.rowCount() - 1; ++i) {
        int val = fm.horizontalAdvance(m_planInfo.at(i + 1, 0)) + 2;
        if (val > max)
            max = val;
    }
    return qMax(110, max + 2 * m_padding);
}

int PlanWidget::columnWidth() const
{
    int max = 0;
    for (int i = 0; i < m_planInfo.rowCount() - 1; ++i) {
        for (int j = 0; j < m_planInfo.columnCount() - 1; ++j) {
            int val = fontMetrics().horizontalAdvance(m_planInfo.at(i + 1, j + 1)) + 2;
            if (val > max)
                max = val;
        }
    }
    return qMax(150, max + 2 * m_padding);
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
            layout->setRowMinimumHeight(0, headerHeight() + rowHeight() * (m_planInfo.rowCount() - 1) + 1);
            const int colWidth = columnWidth();
            for (int i = 0; i < m_planInfo.columnCount() - 1; i++)
                layout->setColumnMinimumWidth(i + 1, colWidth);
        }
        updateGeometry();
        update();
    }
    QWidget::changeEvent(event);
}

void PlanWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int colWidth = columnWidth();
    const int blkWidth = blockWidth();
    const QRectF& cr = contentsRect();

    // Paint Block Background
    if (m_planInfo.rowCount() > 1) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#606060"));
        painter.drawRoundedRect(QRectF(cr.left(), cr.top() + headerHeight(), blkWidth,
                                       rowHeight() * (m_planInfo.rowCount() - 1) + 1),
                                m_radius, m_radius);
    }

    painter.setFont(UtilityFunctions::thickerFont(font()));
    painter.setRenderHint(QPainter::Antialiasing, false);
    for (int i = 0; i < m_planInfo.rowCount() - 1; i++) {
        // Paint Block Row Lines
        if (i != m_planInfo.rowCount() - 2) {
            painter.setPen(cosmeticPen(QColor(255, 255, 255, 45)));
            painter.drawLine(QPointF(cr.left() + m_padding, cr.top() + headerHeight()
                                     + rowHeight() * (i + 1)),
                             QPointF(cr.left() + blkWidth - m_padding, cr.top() + headerHeight()
                                     + rowHeight() * (i + 1)));
        }
        // Paint Block Row Texts
        painter.setPen(palette().brightText().color());
        painter.drawText(QRectF(cr.left(), cr.top() + headerHeight()
                                + rowHeight() * i, blkWidth - m_padding, rowHeight()),
                         m_planInfo.at(i + 1, 0), Qt::AlignVCenter | Qt::AlignRight);
    }
    painter.setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < m_planInfo.columnCount() - 1; i++) {
        // Paint Column Backgrounds
        const qreal left = cr.left() + blkWidth + m_spacing * (i + 1) + colWidth * i;
        const QRectF boundingRect(left + 0.5, cr.top() + 0.5, colWidth - 1, rowHeight() * (m_planInfo.rowCount() - 1) + headerHeight());
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_planInfo.headerColor(i + 1, isEnabled()).darker(120));
        painter.drawRoundedRect(boundingRect.adjusted(-0.5, -0.5, 0.5, 0.5), m_radius + 0.5, m_radius + 0.5);
        painter.setBrush(m_planInfo.columnColor(i + 1, isEnabled()));
        painter.drawRoundedRect(boundingRect, m_radius, m_radius);
        // Paint Header Backgrounds
        QRectF headerRect(boundingRect);
        headerRect.setBottom(headerRect.top() + headerHeight());
        painter.setClipRect(headerRect);
        QLinearGradient header(0, 0, 0, 1);
        header.setCoordinateMode(QGradient::ObjectMode);
        header.setColorAt(0, m_planInfo.headerColor(i + 1, isEnabled()).lighter(108));
        header.setColorAt(1, m_planInfo.headerColor(i + 1, isEnabled()).darker(105));
        painter.setBrush(header);
        painter.drawRoundedRect(headerRect.adjusted(0, 0, 0, m_radius + 1), m_radius, m_radius);
        painter.setClipRect(headerRect, Qt::NoClip);

        // Paint Header Row Lines
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setPen(cosmeticPen(m_planInfo.headerColor(i + 1, isEnabled()).darker(120)));
        painter.drawLine(QPointF(left, cr.top() + headerHeight()),
                         QPointF(left + colWidth, cr.top() + headerHeight()));
        // Paint Column Row Lines
        painter.setPen(cosmeticPen(QColor(0, 0, 0, 45)));
        for (int j = 0; j < m_planInfo.rowCount() - 2; j++) {
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
                         m_planInfo.at(0, i + 1), Qt::AlignVCenter | Qt::AlignHCenter);

        // Paint Column Row Texts
        painter.setFont(font());
        painter.setPen(palette().text().color());
        for (int j = 0; j < m_planInfo.rowCount() - 1; j++) {
            painter.drawText(QRectF(left, cr.top() + headerHeight() + rowHeight() * j, colWidth, rowHeight()),
                             m_planInfo.at(j + 1, i + 1), Qt::AlignVCenter | Qt::AlignHCenter);
        }

        const QString& badge = m_planInfo.badge(i + 1);
        if (!badge.isEmpty()) {
            static const QVector<QPointF> points = { {-24, 0}, {24, 0}, {0, 30} };
            const qreal angle = 20;
            QPainterPath path;
            QTransform transform;
            path.setFillRule(Qt::WindingFill);
            for (int i = 360; i >= 0; i -= angle)
                path.addPolygon(transform.rotate(angle).map(points));
            const QColor badgeColor(m_planInfo.badgeColor(i + 1, isEnabled()));
            QLinearGradient background(0, 0, 0, 1);
            background.setCoordinateMode(QGradient::ObjectMode);
            background.setColorAt(0, badgeColor);
            background.setColorAt(1, badgeColor.darker(125));
            QFont f(font());
            f.setPixelSize(f.pixelSize() - 2);
            painter.save();
            painter.setFont(UtilityFunctions::thickerFont(f));
            painter.translate(left + colWidth - path.boundingRect().width() / 2,
                              cr.top() + headerHeight() / 2);
            painter.setBrush(background);
            painter.setPen(cosmeticPen(badgeColor.darker(250)));
            painter.drawPath(path.simplified());
            painter.setPen(palette().brightText().color());
            painter.drawText(path.boundingRect(), badge, Qt::AlignVCenter | Qt::AlignHCenter);
            painter.restore();
        }
    }
}
