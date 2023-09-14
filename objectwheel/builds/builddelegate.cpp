#include <builddelegate.h>
#include <buildmodel.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QPainter>
#include <QTime>
#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPainterPath>

enum { PADDING = 7 };

BuildDelegate::BuildDelegate(QObject* parent) : StyledItemDelegate(parent)
{
}

void BuildDelegate::setEditorData(QWidget*, const QModelIndex&) const
{
    // do nothing
}

void BuildDelegate::setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&) const
{
    // do nothing
}

void BuildDelegate::updateEditorGeometry(QWidget* widget, const QStyleOptionViewItem& option,
                                         const QModelIndex& index) const
{
    StyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    widget->setGeometry(opt.rect.left() + opt.rect.width() - PADDING - widget->width(),
                        opt.rect.top() + PADDING,
                        widget->width(), opt.rect.height() - 2 * PADDING);
}

QWidget* BuildDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const
{
    if (!index.isValid())
        return nullptr;

    StyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const QSize& buttonSize = index.data(BuildModel::ButtonSize).toSize();
    auto model = static_cast<const BuildModel*>(index.model());
    const QByteArray& identifier = index.data(BuildModel::Identifier).toByteArray();

    QPushButton* infoButton = nullptr;
    if (opt.widget == opt.view) {
        infoButton = new QPushButton;
        infoButton->setFocusPolicy(Qt::StrongFocus);
        infoButton->setCursor(Qt::PointingHandCursor);
        infoButton->setFlat(true);
        infoButton->setIcon(QIcon(":/images/output/info.svg"));
        infoButton->setFixedSize(buttonSize);
        infoButton->setToolTip(tr("Show details"));
        connect(infoButton, &QPushButton::clicked,
                this, [=] { emit infoButtonClicked(model->indexFromIdentifier(identifier)); });
    }

    auto deleteButton = new QPushButton;
    deleteButton->setFocusPolicy(Qt::StrongFocus);
    deleteButton->setCursor(Qt::PointingHandCursor);
    deleteButton->setFlat(true);
    deleteButton->setIcon(QIcon(":/images/builds/trash.svg"));
    deleteButton->setFixedSize(buttonSize);
    deleteButton->setToolTip(tr("Abort"));
    connect(deleteButton, &QPushButton::clicked, this,
            [=] { emit deleteButtonClicked(model->indexFromIdentifier(identifier)); });

    auto openFolderButton = new QPushButton;
    openFolderButton->setFocusPolicy(Qt::StrongFocus);
    openFolderButton->setCursor(Qt::PointingHandCursor);
    openFolderButton->setFlat(true);
    openFolderButton->setIcon(QIcon(":/images/builds/open.svg"));
    openFolderButton->setFixedSize(buttonSize);
    openFolderButton->setEnabled(false);
    openFolderButton->setToolTip(tr("Show in folder"));
    connect(openFolderButton, &QPushButton::clicked, this,
            [=] { emit openFolderButtonClicked(model->indexFromIdentifier(identifier)); });
    connect(model, &BuildModel::downloadFinished,
            openFolderButton, [=] (const QModelIndex& i) {
        if (i.data(BuildModel::Identifier).toByteArray() == identifier
                && !i.data(BuildModel::ErrorRole).toBool()) {
            openFolderButton->setEnabled(true);
        }
    });

    auto widget = new QWidget(parent);
    auto layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    if (infoButton) {
        layout->addWidget(infoButton);
        layout->addStretch();
    }
    layout->addWidget(deleteButton);
    layout->addStretch();
    layout->addWidget(openFolderButton);
    widget->setFixedWidth(buttonSize.width());

    return widget;
}

QSize BuildDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    return QSize(550, option.decorationSize.height() + 2 * PADDING);
}

void BuildDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const
{
    StyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();
    painter->setFont(opt.font);

    // Limit drawing region to view's rect (with rounded corners)
    if (opt.view) {
        QPainterPath path;
        path.addRoundedRect(opt.view->viewport()->rect(), PADDING, PADDING);
        painter->setClipPath(path);
    }

    // Draw background
    painter->fillRect(opt.rect, opt.backgroundBrush);

    // Draw app icon
    QRect iconRect(QPoint(opt.rect.left() + PADDING, opt.rect.top() + PADDING), opt.decorationSize);
    if (opt.icon.isNull()) {
        painter->setPen(QPen(Qt::gray, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawRoundedRect(iconRect, 4, 4);
        painter->setRenderHint(QPainter::Antialiasing, false);
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
    labelFont.setWeight(QFont::Normal);
    const int spacing = 2;
    const QFontMetrics fmLabel(labelFont);
    const QSize& buttonSize = index.data(BuildModel::ButtonSize).toSize();
    const qreal textHeight = (opt.rect.height() - 2 * PADDING) / 4.0; // Divided by row count
    const int leftLabelLength = fmLabel.horizontalAdvance(tr("Version:"));
    const int rightLabelLength = fmLabel.horizontalAdvance(tr("Time left:"));
    const int rightLength = rightLabelLength + spacing
            + opt.fontMetrics.horizontalAdvance("999.99 MB / 999.99 MB ( % 100.00 )") + 1;
    const int leftLength = opt.rect.width() - iconRect.right() - 3 * PADDING - buttonSize.width()
            - 2 * spacing - rightLength;
    painter->setPen(opt.palette.text().color());

    QRectF labelRect(iconRect.right() + PADDING, opt.rect.top() + PADDING, leftLabelLength, textHeight);
    QRectF textRect(labelRect.right() + spacing, labelRect.top(), leftLength - leftLabelLength
                    - spacing, textHeight);
    const QString& nameStr = opt.fontMetrics.elidedText(index.data(BuildModel::NameRole).toString(),
                                                        Qt::ElideRight, textRect.width());
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("Name:"), Qt::AlignRight | Qt::AlignTop);
    painter->setFont(opt.font);
    painter->drawText(textRect, nameStr, Qt::AlignLeft | Qt::AlignTop);

    labelRect.moveTop(labelRect.top() + textHeight);
    textRect.moveTop(textRect.top() + textHeight);
    const QString& versionStr = opt.fontMetrics.elidedText(index.data(BuildModel::VersionRole).toString(),
                                                           Qt::ElideRight, textRect.width());
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("Version:"), Qt::AlignRight | Qt::AlignVCenter);
    painter->setFont(opt.font);
    painter->drawText(textRect, versionStr, Qt::AlignLeft | Qt::AlignVCenter);

    labelRect.moveTop(labelRect.top() + textHeight);
    textRect.moveTop(textRect.top() + textHeight);
    const QString& abisStr = opt.fontMetrics.elidedText(index.data(BuildModel::AbisRole).toString(),
                                                        Qt::ElideRight, textRect.width());
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("ABIs:"), Qt::AlignRight | Qt::AlignVCenter);
    painter->setFont(opt.font);
    painter->drawText(textRect, abisStr, Qt::AlignLeft | Qt::AlignVCenter);

    labelRect.moveTop(labelRect.top() + textHeight);
    textRect.moveTop(textRect.top() + textHeight);
    const QString& statusStr = opt.fontMetrics.elidedText(index.data(Qt::StatusTipRole).toString(),
                                                          Qt::ElideRight, textRect.width());
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("Status:"), Qt::AlignRight | Qt::AlignBottom);
    painter->setFont(opt.font);
    painter->drawText(textRect, statusStr, Qt::AlignLeft | Qt::AlignBottom);

    const bool hasError = index.data(BuildModel::ErrorRole).toBool();
    const bool isFinished = index.data(BuildModel::StateRole).toInt() == BuildModel::Finished;
    const qreal speed = index.data(BuildModel::SpeedRole).toReal();
    const QTime& timeLeft = index.data(BuildModel::TimeLeftRole).toTime();
    const int transferredBytes = index.data(BuildModel::TransferredBytesRole).toInt();
    const int totalBytes = index.data(BuildModel::TotalBytesRole).toInt();
    const qreal progress = totalBytes > 0 ? 100.0 * transferredBytes / totalBytes : 0;
    const QString& speedStr = UtilityFunctions::toPrettyBytesString(speed) + QLatin1String("/s");
    const QString& timeLeftStr = timeLeft.isValid() ? timeLeft.toString(QLatin1String("hh:mm:ss"))
                                                    : QLatin1String("00:00:00");
    const QString& sizeStr = UtilityFunctions::toPrettyBytesString(transferredBytes)
            + QLatin1String(" / ")
            + UtilityFunctions::toPrettyBytesString(totalBytes)
            + QStringLiteral(" ( % %1 )").arg(QString::number(progress, 'f', 2));

    labelRect = QRectF(labelRect.left() + leftLength + 2 * spacing, opt.rect.top() + PADDING,
                       rightLabelLength, textHeight);
    textRect = QRectF(labelRect.right() + spacing, labelRect.top(), rightLength - rightLabelLength
                      - spacing, textHeight);
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("Speed:"), Qt::AlignRight | Qt::AlignTop);
    painter->setFont(opt.font);
    painter->drawText(textRect, speedStr, Qt::AlignLeft | Qt::AlignTop);

    labelRect.moveTop(labelRect.top() + textHeight);
    textRect.moveTop(textRect.top() + textHeight);
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("Time left:"), Qt::AlignRight | Qt::AlignVCenter);
    painter->setFont(opt.font);
    painter->drawText(textRect, timeLeftStr, Qt::AlignLeft | Qt::AlignVCenter);

    labelRect.moveTop(labelRect.top() + textHeight);
    textRect.moveTop(textRect.top() + textHeight);
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("Size:"), Qt::AlignRight | Qt::AlignVCenter);
    painter->setFont(opt.font);
    painter->drawText(textRect, sizeStr, Qt::AlignLeft | Qt::AlignVCenter);

    labelRect.moveTop(labelRect.top() + textHeight);
    textRect.moveTop(textRect.top() + textHeight);
    painter->setFont(labelFont);
    painter->drawText(labelRect, tr("Progress:"), Qt::AlignRight | Qt::AlignBottom);
    painter->setFont(opt.font);
    QRectF progressRect(textRect.left(), 0, textRect.width(), 7);
    progressRect.moveBottom(textRect.bottom() - 2);
    QStyleOptionProgressBar bar;
    bar.initFrom(opt.widget);
    bar.rect = progressRect.toRect();
    bar.invertedAppearance = isFinished ? true : false;
    bar.maximum = 100;
    bar.progress = isFinished ? 100 : progress;
    bar.palette.setColor(QPalette::Button, hasError ? QColor("#dd5959") : (isFinished ? QColor("#65b84b") : QColor()));
    // Due to stylesheet on QListView, we don't use opt->widget->style()
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &bar, painter, opt.widget);

    // Draw bottom line
    if (index.row() != index.model()->rowCount() - 1) {
        painter->setPen(QPen(QColor("#28000000"), 0));
        painter->drawLine(opt.rect.bottomLeft() + QPointF(PADDING, 0.5),
                          opt.rect.bottomRight() + QPointF(-PADDING, 0.5));
    }

    painter->restore();
}
