#include <propertiesdelegate.h>

PropertiesDelegate::PropertiesDelegate(PropertiesTree* propertiesTree) : QStyledItemDelegate(propertiesTree)
  , m_propertiesTree(propertiesTree)
{}

void PropertiesDelegate::paintBackground(QPainter* painter, const QStyleOptionViewItem& option,
                                         int rowNumber, bool isClassRow, bool hasVerticalLine)
{
    painter->save();

    const QRectF& rect = option.rect;

    QPainterPath path;
    path.addRect(rect);
    painter->setClipPath(path);
    painter->setClipping(true);

    // Fill background
    if (isClassRow)
        painter->fillRect(rect, option.palette.light());
    else if (rowNumber % 2)
        painter->fillRect(rect, option.palette.alternateBase());
    else
        painter->fillRect(rect, option.palette.base());

    // Draw top and bottom lines
    QColor lineColor(option.palette.dark().color());
    lineColor.setAlpha(50);
    painter->setPen(lineColor);
    painter->drawLine(rect.topLeft() + QPointF(0.5, 0.0),
                      rect.topRight() - QPointF{0.5, 0.0});
    painter->drawLine(rect.bottomLeft() + QPointF(0.5, 0.0),
                      rect.bottomRight() - QPointF{0.5, 0.0});

    // Draw vertical line
    if (hasVerticalLine) {
        painter->drawLine(rect.topRight() + QPointF(-0.5, 0.5),
                          rect.bottomRight() + QPointF(-0.5, -0.5));
    }

    painter->restore();
}

void PropertiesDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const QAbstractItemModel* model = index.model();
    const bool isClassRow = !model->parent(index).isValid() && index.row() > 3;

    fillBackground(painter, option,
                   calculateVisibleRow(m_propertiesPane->itemFromIndex(index)),
                   isClassRow, index.column() == 0 && !isClassRow);

    // Draw text
    if (isClassRow) {
        painter->setPen(option.palette.highlightedText().color());
    } else {
        if (index.column() == 0 && index.data(Qt::DecorationRole).toBool()) {
            QFont font (option.font);
            font.setWeight(QFont::Medium);
            painter->setFont(font);
            painter->setPen(option.palette.link().color());
        } else {
            painter->setPen(option.palette.text().color());
        }
    }

    const QRectF& textRect = option.rect.adjusted(5, 0, 0, 0);
    const QString& text = index.data(Qt::DisplayRole).toString();
    painter->drawText(textRect,
                      option.fontMetrics.elidedText(text, Qt::ElideMiddle, textRect.width()),
                      QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
    painter->restore();
}

QSize PropertiesDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QSize& size = QStyledItemDelegate::sizeHint(option, index);
    return QSize(size.width(), ROW_HEIGHT);
}
