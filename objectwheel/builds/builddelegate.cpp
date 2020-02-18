#include <builddelegate.h>
#include <buildmodel.h>
#include <QPainter>

BuildDelegate::BuildDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

void BuildDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();

//    const QString& name = index.data(NameRole).toString();
//    const QString& description = index.data(DescriptionRole).toString();
//    const Availability availability = index.data(AvailabilityRole).value<Availability>();

//    // Limit drawing region to view's rect (with rounded corners)
//    QPainterPath path;
//    path.addRoundedRect(opt.widget->rect(), 8, 8);
//    painter->setClipPath(path);

//    // Draw highlighted background if selected
//    if (opt.state.testFlag(QStyle::State_Selected))
//        painter->fillRect(opt.rect, opt.palette.highlight());

//    // Draw icon
//    const int padding = opt.rect.height() / 2.0 - opt.decorationSize.height() / 2.0;
//    const QRectF iconRect(QPointF(opt.rect.left() + padding, opt.rect.top() + padding), opt.decorationSize);
//    const QPixmap& icon = PaintUtils::pixmap(opt.icon, opt.decorationSize, opt.widget);
//    painter->drawPixmap(iconRect, icon, icon.rect());

//    // Draw texts
//    QFont f;
//    f.setWeight(QFont::Medium);
//    painter->setFont(f);
//    const QRectF nameRect(QPointF(iconRect.right() + padding, iconRect.top()),
//                          QSizeF(opt.rect.width() - opt.decorationSize.width() - 3 * padding,
//                                 iconRect.height() / 2.0));
//    painter->setPen(opt.palette.text().color());
//    painter->drawText(nameRect, name, Qt::AlignVCenter | Qt::AlignLeft);

//    f.setWeight(QFont::Normal);
//    painter->setFont(f);
//    const QRectF descRect(QPointF(iconRect.right() + padding, iconRect.center().y()),
//                          QSizeF(opt.rect.width() - opt.decorationSize.width() - 3 * padding,
//                                 iconRect.height() / 2.0));
//    painter->drawText(descRect, description, Qt::AlignVCenter | Qt::AlignLeft);

//    // Draw bottom line
//    if (index.row() != index.model()->rowCount() - 1) {
//        painter->setPen(QPen(QColor("#28000000"), 0));
//        painter->drawLine(opt.rect.bottomLeft() + QPointF(padding, 0.5),
//                          opt.rect.bottomRight() + QPointF(-padding, 0.5));
//    }

//    // Draw availability label
//    if (availability != Available) {
//        f.setPixelSize(f.pixelSize() - 2);
//        const QString label = availability == Soon ? tr("SOON") : tr("IN FUTURE");
//        const QFontMetrics fm(f);
//        const int msgHeight = fm.height();
//        const int msgWidth = fm.horizontalAdvance(label) + msgHeight;
//        const QRectF msgRect(QPointF(opt.rect.left() + opt.rect.width() - msgWidth - padding,
//                                     opt.rect.top() + opt.rect.height() / 2.0 - msgHeight / 2.0),
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
