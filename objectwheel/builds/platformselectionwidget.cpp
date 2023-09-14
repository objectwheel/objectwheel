#include <platformselectionwidget.h>
#include <paintutils.h>
#include <buttonslice.h>
#include <platform.h>
#include <styleditemdelegate.h>
#include <utilityfunctions.h>

#include <QBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QScrollBar>
#include <QApplication>
#include <QPainter>
#include <QPushButton>
#include <QPainterPath>

enum Availability {
    InFuture,
    Soon,
    Available,
};
Q_DECLARE_METATYPE(Availability)

enum ItemRoles {
    PlatformRole = Qt::UserRole + 1,
    NameRole,
    DescriptionRole,
    AvailabilityRole
};

class PlatformListDelegate final : public StyledItemDelegate
{
    Q_DISABLE_COPY(PlatformListDelegate)

public:
    explicit PlatformListDelegate(QObject* parent = nullptr) : StyledItemDelegate(parent)
    {}

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        return QSize(StyledItemDelegate::sizeHint(option, index).width(),
                     option.decorationSize.height() + 14);
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        StyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        painter->save();

        // Limit drawing region to view's rect (with rounded corners)
        if (opt.view) {
            QPainterPath path;
            path.addRoundedRect(opt.view->viewport()->rect(), 7, 7);
            painter->setClipPath(path);
        }

        // Draw highlighted background if selected
        if (opt.state.testFlag(QStyle::State_Selected))
            painter->fillRect(opt.rect, opt.palette.highlight());

        // Draw icon
        const int padding = opt.rect.height() / 2.0 - opt.decorationSize.height() / 2.0;
        const QRectF iconRect(QPointF(opt.rect.left() + padding, opt.rect.top() + padding), opt.decorationSize);
        const QPixmap& icon = PaintUtils::pixmap(opt.icon, opt.decorationSize, opt.view);
        painter->drawPixmap(iconRect, icon, icon.rect());

        // Draw texts
        const QRectF nameRect(QPointF(iconRect.right() + padding, iconRect.top()),
                              QSizeF(opt.rect.width() - opt.decorationSize.width() - 3 * padding,
                                     iconRect.height() / 2.0));
        painter->setPen(opt.palette.text().color());
        painter->drawText(nameRect, index.data(NameRole).toString(), Qt::AlignVCenter | Qt::AlignLeft);

        QFont f;
        f.setPixelSize(f.pixelSize() - 1);
        painter->setFont(f);
        painter->setPen(QColor(0, 0, 0, opt.state.testFlag(QStyle::State_Enabled) ? 160 : 100));
        const QRectF descRect(QPointF(iconRect.right() + padding, iconRect.center().y()),
                              QSizeF(opt.rect.width() - opt.decorationSize.width() - 3 * padding,
                                     iconRect.height() / 2.0));
        painter->drawText(descRect, index.data(DescriptionRole).toString(), Qt::AlignVCenter | Qt::AlignLeft);

        // Draw bottom line
        if (index.row() != index.model()->rowCount() - 1) {
            painter->setPen(QPen(QColor("#28000000"), 0));
            painter->drawLine(opt.rect.bottomLeft() + QPointF(padding, 0.5),
                              opt.rect.bottomRight() + QPointF(-padding, 0.5));
        }

        // Draw availability label
        const Availability availability = index.data(AvailabilityRole).value<Availability>();
        if (availability != Available) {
            f.setPixelSize(f.pixelSize() - 2);
            const QString label = availability == Soon ? tr("SOON") : tr("IN FUTURE");
            const QFontMetrics fm(f);
            const int msgHeight = fm.height();
            const int msgWidth = fm.horizontalAdvance(label) + msgHeight;
            const QRectF msgRect(QPointF(opt.rect.left() + opt.rect.width() - msgWidth - padding,
                                         opt.rect.top() + opt.rect.height() / 2.0 - msgHeight / 2.0),
                                 QSizeF(msgWidth, msgHeight));
            painter->setFont(f);
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(availability == Soon ? QColor("#b99a4e") : QColor("#607bb3"));
            painter->setBrush(availability == Soon ? QColor("#fff9df") : QColor("#f2faff"));
            painter->drawRoundedRect(msgRect, 2, 2);
            painter->drawText(msgRect, label, Qt::AlignVCenter | Qt::AlignHCenter);
        }

        painter->restore();
    }
};

PlatformSelectionWidget::PlatformSelectionWidget(QWidget* parent) : QWidget(parent)
  , m_platformList(new QListWidget(this))
  , m_buttonSlice(new ButtonSlice(this))
{
    auto updatePalette = [=] {
        QPalette p(QApplication::palette());
        p.setColor(QPalette::Highlight, QStringLiteral("#16000000"));
        m_platformList->setPalette(p);
    };
//    connect(qApp, &QApplication::paletteChanged, this, updatePalette);
    updatePalette();

    auto androidItem = new QListWidgetItem;
    androidItem->setIcon(QIcon(":/images/builds/android.svg"));
    androidItem->setData(PlatformRole, Android);
    androidItem->setData(NameRole, QLatin1String("Android 5.0+"));
    androidItem->setData(DescriptionRole, tr("Supported ABIs: armeabi-v7a, arm64-v8a, x86, x86_64"));
    androidItem->setData(AvailabilityRole, Available);
    m_platformList->addItem(androidItem);

    auto iOSItem = new QListWidgetItem;
    iOSItem->setIcon(QIcon(":/images/builds/ios.svg"));
    iOSItem->setData(PlatformRole, iOS);
    iOSItem->setData(NameRole, QLatin1String("iOS 12+"));
    iOSItem->setData(DescriptionRole, tr("Supported ABIs: arm64-v8a, x86_64 (simulator)"));
    iOSItem->setData(AvailabilityRole, Soon);
    iOSItem->setFlags(iOSItem->flags() & ~Qt::ItemIsSelectable);
    m_platformList->addItem(iOSItem);

    auto macOSItem = new QListWidgetItem;
    macOSItem->setIcon(QIcon(":/images/builds/macos.svg"));
    macOSItem->setData(PlatformRole, macOS);
    macOSItem->setData(NameRole, QLatin1String("macOS 10.13+"));
    macOSItem->setData(DescriptionRole, tr("Supported ABIs: x86_64"));
    macOSItem->setData(AvailabilityRole, Soon);
    macOSItem->setFlags(macOSItem->flags() & ~Qt::ItemIsSelectable);
    m_platformList->addItem(macOSItem);

    auto windowsItem = new QListWidgetItem;
    windowsItem->setIcon(QIcon(":/images/builds/windows.svg"));
    windowsItem->setData(PlatformRole, Windows);
    windowsItem->setData(NameRole, QLatin1String("Windows 7+"));
    windowsItem->setData(DescriptionRole, tr("Supported ABIs: x86, x86_64"));
    windowsItem->setData(AvailabilityRole, Soon);
    windowsItem->setFlags(windowsItem->flags() & ~Qt::ItemIsSelectable);
    m_platformList->addItem(windowsItem);

    auto linuxItem = new QListWidgetItem;
    linuxItem->setIcon(QIcon(":/images/builds/linux.svg"));
    linuxItem->setData(PlatformRole, Linux);
    linuxItem->setData(NameRole, QLatin1String("Linux (X11)"));
    linuxItem->setData(DescriptionRole, tr("Supported ABIs: x86, x86_64"));
    linuxItem->setData(AvailabilityRole, Soon);
    linuxItem->setFlags(linuxItem->flags() & ~Qt::ItemIsSelectable);
    m_platformList->addItem(linuxItem);

    auto raspberryPiItem = new QListWidgetItem;
    raspberryPiItem->setIcon(QIcon(":/images/builds/raspberry.svg"));
    raspberryPiItem->setData(PlatformRole, RaspberryPi);
    raspberryPiItem->setData(NameRole, QLatin1String("Raspberry Pi 2+"));
    raspberryPiItem->setData(DescriptionRole, tr("Supported ABIs: armeabi-v7a, arm64-v8a"));
    raspberryPiItem->setData(AvailabilityRole, InFuture);
    raspberryPiItem->setFlags(raspberryPiItem->flags() & ~Qt::ItemIsSelectable);
    m_platformList->addItem(raspberryPiItem);

    m_platformList->setUniformItemSizes(true);
    m_platformList->setIconSize(QSize(40, 40));
    m_platformList->setFixedSize(QSize(450, 326));
    m_platformList->setFocusPolicy(Qt::NoFocus);
    m_platformList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_platformList->setItemDelegate(new PlatformListDelegate(m_platformList));
    m_platformList->verticalScrollBar()->setStyleSheet(
                QStringLiteral(
                    "QScrollBar:vertical {"
                    "    background: transparent;"
                    "    width: %2px;"
                    "} QScrollBar::handle:vertical {"
                    "    background: #909497;"
                    "    min-height: %1px;"
                    "    border-radius: %3px;"
                    "} QScrollBar::add-line:vertical {"
                    "    background: none;"
                    "} QScrollBar::sub-line:vertical {"
                    "    background: none;"
                    "} QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {"
                    "    background: none;"
                    "} QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
                    "    background: none;"
                    "}").arg(15).arg(6).arg(2.5));
    m_platformList->setStyleSheet(
                QStringLiteral(
                    "QListWidget {"
                    "    background: #12000000;"
                    "    border: 1px solid #22000000;"
                    "    border-radius: %1px;"
                    "}").arg(8));

    m_buttonSlice->add(Back, "#5BC5F8", "#2592F9");
    m_buttonSlice->add(Next, "#8BBB56", "#6EA045");
    m_buttonSlice->get(Back)->setText(tr("Back"));
    m_buttonSlice->get(Next)->setText(tr("Next"));
    m_buttonSlice->get(Back)->setIcon(QIcon(":/images/welcome/unload.png"));
    m_buttonSlice->get(Next)->setIcon(QIcon(":/images/welcome/load.png"));
    m_buttonSlice->get(Back)->setCursor(Qt::PointingHandCursor);
    m_buttonSlice->get(Next)->setCursor(Qt::PointingHandCursor);
    m_buttonSlice->settings().cellWidth = 150;
    m_buttonSlice->triggerSettings();

    auto iconLabel = new QLabel(this);
    auto titleLabel = new QLabel(tr("Platform Selection"), this);
    auto descriptionLabel = new QLabel(tr("Select your target platform to start"), this);
    auto platformsLabel = new QLabel(tr("Platforms"));

    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/builds/oses.svg"),
                                            QSize(60, 60), this));
    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(24);
    titleLabel->setFont(f);

    f.setWeight(QFont::Light);
    f.setPixelSize(16);
    descriptionLabel->setFont(f);

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->setRowStretch(0, 1);
    layout->setRowMinimumHeight(0, 12);
    layout->addWidget(iconLabel, 1, 1, Qt::AlignHCenter);
    layout->addWidget(titleLabel, 2, 1, Qt::AlignHCenter);
    layout->addWidget(descriptionLabel, 3, 1, Qt::AlignHCenter);
    layout->setRowMinimumHeight(4, 8);
    layout->addWidget(platformsLabel, 5, 1, Qt::AlignHCenter);
    layout->addWidget(m_platformList, 6, 1, Qt::AlignHCenter);
    layout->addWidget(m_buttonSlice, 7, 1, Qt::AlignHCenter);
    layout->addWidget(m_buttonSlice, 8, 1, Qt::AlignHCenter);
    layout->setRowStretch(9, 1);
    layout->setRowMinimumHeight(9, 12);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
}

QListWidget* PlatformSelectionWidget::platformList() const
{
    return m_platformList;
}

ButtonSlice* PlatformSelectionWidget::buttonSlice() const
{
    return m_buttonSlice;
}

Platform PlatformSelectionWidget::currentPlatform() const
{
    const QList<QListWidgetItem*>& selectedItems = m_platformList->selectedItems();
    if (selectedItems.isEmpty())
        return Invalid;
    return selectedItems.first()->data(PlatformRole).value<Platform>();
}
