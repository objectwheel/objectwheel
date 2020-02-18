#include <builddelegate.h>
#include <buildmodel.h>
#include <paintutils.h>

#include <QPainter>
#include <QAbstractItemView>

BuildDelegate::BuildDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

//case Qt::ForegroundRole:
//    return QApplication::palette().text();
//case Qt::TextAlignmentRole:
//    return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
//case Qt::FontRole: {
//    QFont font(QApplication::font());
//    font.setPixelSize(10);
//    font.setWeight(QFont::Light);
//    return font;
//}
//case Qt::SizeHintRole:
//    return QSize(0, 12 * 4 + 2 * 3 + 7 * 2);
//case Qt::DecorationRole:
//    return QImage::fromData(build->request().value(QLatin1String("icon")).toByteArray());
//case PaddingRole:
//    return 7;
//case PlatformRole:
//    return toPrettyPlatformName(build->request().value(QLatin1String("platform")).toString());
//case NameRole:
//    return build->request().value(QLatin1String("name")).toString() + packageSuffixFromRequest(build->request());
//case PlatformIconRole:
//    return platformIcon(build->request().value(QLatin1String("platform")).toString());
//case VersionRole:
//    return build->request().value(QLatin1String("versionName")).toString();
//case AbisRole: {
//    QStringList abis;
//    foreach (const QCborValue& abi, build->request().value(QLatin1String("abis")).toArray())
//        abis.append(abi.toString());
//    return abis.join(QLatin1String(", "));
//}
//case StatusRole:
//    return build->status();
//case SpeedRole:
//    return build->speed();
//case TimeLeftRole:
//    return build->timeLeft();
//case TotalDataSizeRole:
//    return build->totalDataSize();
//case ReceivedDataSizeRole:
//    return build->receivedDataSize();
#include <QDebug>
void BuildDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();
    painter->setFont(opt.font);

    // Limit drawing region to view's rect (with rounded corners)
    QPainterPath path;
    path.addRoundedRect(static_cast<const QAbstractItemView*>(opt.widget)->viewport()->rect(), 7, 7);
    painter->setClipPath(path);

    // Draw background
    painter->fillRect(opt.rect, opt.backgroundBrush);

    // Draw app icon
    const int padding = opt.rect.height() / 2.0 - opt.decorationSize.height() / 2.0;
    QRectF iconRect(QPointF(opt.rect.left() + padding, opt.rect.top() + padding), opt.decorationSize);
    if (opt.icon.isNull()) {
        painter->setPen(QPen(Qt::gray, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(iconRect);
        painter->setPen(QPen(Qt::darkGray));
        painter->drawText(iconRect, tr("Empty\nicon"), Qt::AlignVCenter | Qt::AlignHCenter);
    } else {
        const QPixmap& icon = PaintUtils::pixmap(opt.icon, opt.decorationSize, opt.widget);
        painter->drawPixmap(iconRect, icon, icon.rect());
    }

    // Draw platform icon
    iconRect.moveTopLeft(iconRect.center());
    iconRect.setSize(iconRect.size() / 2.0);
    const QPixmap& icon = PaintUtils::pixmap(index.data(BuildModel::PlatformIconRole).value<QIcon>(),
                                             opt.decorationSize / 2, opt.widget);
    painter->drawPixmap(iconRect, icon, icon.rect());

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
