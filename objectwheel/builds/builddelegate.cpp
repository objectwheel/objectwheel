#include <builddelegate.h>
#include <buildmodel.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QPainter>
#include <QTime>
#include <QApplication>

BuildDelegate::BuildDelegate(QObject* parent) : StyledItemDelegate(parent)
{
}

void BuildDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const
{
    StyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();
    painter->setFont(opt.font);

    // Limit drawing region to view's rect (with rounded corners)
    QPainterPath path;
    path.addRoundedRect(opt.widget->viewport()->rect(), 7, 7);
    painter->setClipPath(path);

    // Draw background
    painter->fillRect(opt.rect, opt.backgroundBrush);

    // Draw app icon
    const int padding = opt.rect.height() / 2.0 - opt.decorationSize.height() / 2.0;
    QRect iconRect(QPoint(opt.rect.left() + padding, opt.rect.top() + padding), opt.decorationSize);
    if (opt.icon.isNull()) {
        painter->setPen(QPen(Qt::gray, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(iconRect, 4, 4);
        painter->setPen(QPen(Qt::darkGray));
        painter->drawText(iconRect, tr("Empty\nicon"), Qt::AlignVCenter | Qt::AlignHCenter);
    } else {
        const QPixmap& icon = PaintUtils::pixmap(opt.icon, opt.decorationSize, opt.widget);
        painter->drawPixmap(iconRect, icon, icon.rect());
    }

    // Draw platform icon
    iconRect.moveTopLeft(iconRect.center() + QPoint(2, 2));
    iconRect.setSize(iconRect.size() / 2);
    const QPixmap& icon = PaintUtils::pixmap(index.data(BuildModel::PlatformIconRole).value<QIcon>(),
                                             iconRect.size(), opt.widget);
    painter->drawPixmap(iconRect, icon, icon.rect());

    // Draw texts
    QFont labelFont(opt.font);
    labelFont.setWeight(QFont::Medium);
    const int spacing = 2;
    const QFontMetrics fmLabel(labelFont);
    const QSize& buttonSize = index.data(BuildModel::ButtonSize).toSize();
    const int leftLabelLength = fmLabel.horizontalAdvance(tr("Version:"));
    const int rightLabelLength = fmLabel.horizontalAdvance(tr("Time left:"));
    const qreal textHeight = (opt.rect.height() - 2 * padding) / 4.0; // Divided by row count
    const qreal textWidth = (opt.rect.width() - iconRect.right() - 3 * padding - buttonSize.width() - 2 * spacing) / 2.0;
    painter->setPen(opt.palette.text().color());

    QRectF labelRect(iconRect.right() + padding, opt.rect.top() + padding, leftLabelLength, textHeight);
    QRectF textRect(labelRect.right() + spacing, labelRect.top(), textWidth - leftLabelLength - spacing, textHeight);
    const QString& nameStr = opt.fontMetrics.elidedText(index.data(BuildModel::NameRole).toString(),
                                                        Qt::ElideMiddle, textRect.width() + 1);
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("Name:"), Qt::AlignRight | Qt::AlignTop);
    painter->setFont(opt.font);
    painter->drawText(textRect, nameStr, Qt::AlignLeft | Qt::AlignTop);

    labelRect.moveTop(labelRect.top() + textHeight);
    textRect.moveTop(textRect.top() + textHeight);
    const QString& versionStr = opt.fontMetrics.elidedText(index.data(BuildModel::VersionRole).toString(),
                                                           Qt::ElideMiddle, textRect.width() + 1);
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("Version:"), Qt::AlignRight | Qt::AlignVCenter);
    painter->setFont(opt.font);
    painter->drawText(textRect, versionStr, Qt::AlignLeft | Qt::AlignVCenter);

    labelRect.moveTop(labelRect.top() + textHeight);
    textRect.moveTop(textRect.top() + textHeight);
    const QString& abisStr = opt.fontMetrics.elidedText(index.data(BuildModel::AbisRole).toString(),
                                                        Qt::ElideMiddle, textRect.width() + 1);
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("ABIs:"), Qt::AlignRight | Qt::AlignVCenter);
    painter->setFont(opt.font);
    painter->drawText(textRect, abisStr, Qt::AlignLeft | Qt::AlignVCenter);

    labelRect.moveTop(labelRect.top() + textHeight);
    textRect.moveTop(textRect.top() + textHeight);
    const QString& statusStr = opt.fontMetrics.elidedText(index.data(BuildModel::StatusRole).toString(),
                                                          Qt::ElideMiddle, textRect.width() + 1);
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("Status:"), Qt::AlignRight | Qt::AlignBottom);
    painter->setFont(opt.font);
    painter->drawText(textRect, statusStr, Qt::AlignLeft | Qt::AlignBottom);

    const qreal speed = index.data(BuildModel::SpeedRole).toReal();
    const QTime& timeLeft = index.data(BuildModel::TimeLeftRole).toTime();
    const int receivedBytes = index.data(BuildModel::ReceivedBytesRole).toInt();
    const int totalBytes = index.data(BuildModel::TotalBytesRole).toInt();
    const qreal progress = 100.0 * receivedBytes / totalBytes;
    const QString& speedStr = UtilityFunctions::toPrettyBytesString(speed) + QLatin1String("/sec");
    const QString& timeLeftStr = timeLeft.isValid() ? timeLeft.toString(QLatin1String("hh:mm:ss")) : QLatin1String("-");
    const QString& sizeStr = totalBytes > 0
            ? UtilityFunctions::toPrettyBytesString(receivedBytes)
              + QLatin1String(" / ")
              + UtilityFunctions::toPrettyBytesString(totalBytes)
              + QLatin1String(" ( % %1 )").arg(QString::number(progress, 'f', 2))
            : QLatin1String("-");

    labelRect = QRectF(labelRect.left() + textWidth + 2 * spacing, opt.rect.top() + padding, rightLabelLength, textHeight);
    textRect = QRectF(labelRect.right() + spacing, labelRect.top(), textWidth - rightLabelLength - spacing, textHeight);
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("Speed:"), Qt::AlignRight | Qt::AlignTop);
    painter->setFont(opt.font);
    painter->drawText(textRect, opt.fontMetrics.elidedText(speedStr, Qt::ElideMiddle, textRect.width() + 1),
                      Qt::AlignLeft | Qt::AlignTop);

    labelRect.moveTop(labelRect.top() + textHeight);
    textRect.moveTop(textRect.top() + textHeight);
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("Time left:"), Qt::AlignRight | Qt::AlignVCenter);
    painter->setFont(opt.font);
    painter->drawText(textRect, opt.fontMetrics.elidedText(timeLeftStr, Qt::ElideMiddle, textRect.width() + 1),
                      Qt::AlignLeft | Qt::AlignVCenter);

    labelRect.moveTop(labelRect.top() + textHeight);
    textRect.moveTop(textRect.top() + textHeight);
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("Size:"), Qt::AlignRight | Qt::AlignVCenter);
    painter->setFont(opt.font);
    painter->drawText(textRect, opt.fontMetrics.elidedText(sizeStr, Qt::ElideMiddle, textRect.width() + 1),
                      Qt::AlignLeft | Qt::AlignVCenter);

    labelRect.moveTop(labelRect.top() + textHeight);
    textRect.moveTop(textRect.top() + textHeight);
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("Progress:"), Qt::AlignRight | Qt::AlignBottom);
    painter->setFont(opt.font);
    if (totalBytes > 0) {
        QRectF progressRect(textRect.left(), 0, textRect.width(), 7);
        progressRect.moveBottom(textRect.bottom() - 2);
        QStyleOptionProgressBar bar;
        bar.initFrom(opt.widget);
        bar.rect = progressRect.toRect();
        bar.maximum = 100;
        bar.progress = progress; // Due to stylesheet on QListView, we don't use opt->widget->style()
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &bar, painter, opt.widget);
    } else {
        painter->drawText(textRect, QLatin1String("-"), Qt::AlignLeft | Qt::AlignBottom);
    }

    // Draw bottom line
    if (index.row() != index.model()->rowCount() - 1) {
        painter->setPen(QPen(QColor("#28000000"), 0));
        painter->drawLine(opt.rect.bottomLeft() + QPointF(padding, 0.5),
                          opt.rect.bottomRight() + QPointF(-padding, 0.5));
    }

    painter->restore();
}
