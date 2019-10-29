#include <formsdelegate.h>
#include <formstree.h>
#include <paintutils.h>
#include <control.h>

#include <QPointer>
#include <QPainter>

FormsDelegate::FormsDelegate(FormsTree* formsTree) : QStyledItemDelegate(formsTree)
  , m_formsTree(formsTree)
{
}

void FormsDelegate::paintBackground(QPainter* painter, const QStyleOptionViewItem& option,
                                    int rowNumber) const
{
    painter->save();

    bool isSelected = option.state & QStyle::State_Selected;
    const QPalette& pal = option.palette;
    const QRectF& rect = option.rect;

    painter->setClipRect(rect);

    // Fill background
    if (isSelected) {
        painter->fillRect(rect, pal.highlight());
    } else {
        if (rowNumber % 2)
            painter->fillRect(rect, pal.alternateBase());
        else
            painter->fillRect(rect, pal.base());
    }

    // Draw top and bottom lines
    QColor lineColor(pal.dark().color());
    lineColor.setAlpha(50);
    painter->setPen(lineColor);
    painter->drawLine(rect.topLeft() + QPointF(0.5, 0.0),
                      rect.topRight() - QPointF(0.5, 0.0));
    painter->drawLine(rect.bottomLeft() + QPointF(0.5, 0.0),
                      rect.bottomRight() - QPointF(0.5, 0.0));

    painter->restore();
}

void FormsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const
{
    painter->save();

    const bool isSelected = option.state & QStyle::State_Selected;
    const QAbstractItemModel* model = index.model();
    const QRectF r = option.rect;
    const auto& control = model->data(index, ControlRole).value<QPointer<Control>>();
    Q_ASSERT(control);

    paintBackground(painter, option, index.row());

    // Draw icon
    QRectF iconRect(0, 0, -5, 0);
    QPixmap pixmap(PaintUtils::pixmap(control->icon(), option.decorationSize, m_formsTree,
                                      isSelected ? QIcon::Selected : QIcon::Normal));
    iconRect = QRectF({}, pixmap.size() / pixmap.devicePixelRatioF());
    iconRect.moveCenter(r.center());
    iconRect.moveLeft(r.left() + 5);
    painter->drawPixmap(iconRect, pixmap, pixmap.rect());

    if (isSelected)
        painter->setPen(option.palette.highlightedText().color());
    else
        painter->setPen(option.palette.text().color());

    // Draw text
    if (control) {
        const QRectF& textRect = r.adjusted(iconRect.width() + 10, 0, 0, 0);
        const QString& text = control->id();
        painter->drawText(textRect, option.fontMetrics.elidedText(text, Qt::ElideMiddle, textRect.width()),
                          QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
    }

    painter->restore();
}
