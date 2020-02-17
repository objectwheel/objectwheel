#include <builddelegate.h>
#include <buildmodel.h>

#include <QListView>
#include <QPainter>

BuildDelegate::BuildDelegate(QListView* listView) : QStyledItemDelegate(listView)
  , m_listView(listView)
{
}

QSize BuildDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QSize(QStyledItemDelegate::sizeHint(option, index).width(),
                 m_listView->iconSize().height() + 14);
}

void BuildDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const
{
    if (m_listView == 0)
        return;

    painter->save();

    painter->fillRect(option.rect, Qt::white);
    painter->drawText(option.rect, index.data(BuildModel::StatusRole).toString(), Qt::AlignVCenter | Qt::AlignHCenter);
//    const QRectF r = option.rect;
//    const QString& name = item->data(NameRole).toString();
//    const QString& description = item->data(DescriptionRole).toString();
//    const Availability availability = item->data(AvailabilityRole).value<Availability>();

//    // Limit drawing region to view's rect (with rounded corners)
//    QPainterPath path;
//    path.addRoundedRect(item->listWidget()->rect(), 8, 8);
//    painter->setClipPath(path);

//    // Draw highlighted background if selected
//    if (item->isSelected())
//        painter->fillRect(r, option.palette.highlight());

//    // Draw icon
//    const QSize& iconSize = item->listWidget()->iconSize();
//    const int padding = r.height() / 2.0 - iconSize.height() / 2.0;
//    const QRectF iconRect(QPointF(r.left() + padding, r.top() + padding), iconSize);
//    const QPixmap& icon = PaintUtils::pixmap(item->icon(), iconSize, item->listWidget());
//    painter->drawPixmap(iconRect, icon, icon.rect());

//    // Draw texts
//    QFont f;
//    f.setWeight(QFont::Medium);
//    painter->setFont(f);
//    const QRectF nameRect(QPointF(iconRect.right() + padding, iconRect.top()),
//                          QSizeF(r.width() - iconSize.width() - 3 * padding, iconRect.height() / 2.0));
//    painter->setPen(option.palette.text().color());
//    painter->drawText(nameRect, name, Qt::AlignVCenter | Qt::AlignLeft);

//    f.setWeight(QFont::Normal);
//    painter->setFont(f);
//    const QRectF descRect(QPointF(iconRect.right() + padding, iconRect.center().y()),
//                          QSizeF(r.width() - iconSize.width() - 3 * padding, iconRect.height() / 2.0));
//    painter->drawText(descRect, description, Qt::AlignVCenter | Qt::AlignLeft);

//    // Draw bottom line
//    if (index.row() != item->listWidget()->count() - 1) {
//        painter->setPen(QPen(QColor("#28000000"), 0));
//        painter->drawLine(r.bottomLeft() + QPointF(padding, -0.5),
//                          r.bottomRight() + QPointF(-padding, -0.5));
//    }

//    // Draw availability label
//    if (availability != Available) {
//        f.setPixelSize(f.pixelSize() - 2);
//        const QString label = availability == Soon ? tr("SOON") : tr("IN FUTURE");
//        const QFontMetrics fm(f);
//        const int msgHeight = fm.height();
//        const int msgWidth = fm.horizontalAdvance(label) + msgHeight;
//        const QRectF msgRect(QPointF(r.left() + r.width() - msgWidth - padding,
//                                     r.top() + r.height() / 2.0 - msgHeight / 2.0),
//                             QSizeF(msgWidth, msgHeight));
//        painter->setFont(f);
//        painter->setRenderHint(QPainter::Antialiasing);
//        painter->setPen(availability == Soon ? QColor("#b99a4e") : QColor("#607bb3"));
//        painter->setBrush(availability == Soon ? QColor("#fff9df") : QColor("#f2faff"));
//        painter->drawRoundedRect(msgRect, 2, 2);
//        painter->drawText(msgRect, label, Qt::AlignVCenter | Qt::AlignHCenter);
//    }

    painter->restore();
}
