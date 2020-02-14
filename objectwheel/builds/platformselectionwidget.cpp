#include <platformselectionwidget.h>
#include <paintutils.h>
#include <buttonslice.h>

#include <QBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QStyledItemDelegate>
#include <QScrollBar>
#include <QApplication>
#include <QPainter>
#include <QPushButton>

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

class PlatformListDelegate final : public QStyledItemDelegate
{
    Q_DISABLE_COPY(PlatformListDelegate)

public:
    PlatformListDelegate(QListWidget* listWidget, QWidget* parent) : QStyledItemDelegate(parent)
      , m_listWidget(listWidget)
    {
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        return QSize(QStyledItemDelegate::sizeHint(option, index).width(),
                     m_listWidget->iconSize().height() + 14);
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        const QListWidgetItem* item = m_listWidget->item(index.row());

        if (item == 0)
            return;

        if (item->listWidget() == 0)
            return;

        painter->save();

        const QRectF r = option.rect;
        const QString& name = item->data(NameRole).toString();
        const QString& description = item->data(DescriptionRole).toString();
        const Availability availability = item->data(AvailabilityRole).value<Availability>();

        // Limit drawing region to view's rect (with rounded corners)
        QPainterPath path;
        path.addRoundedRect(item->listWidget()->rect(), 8, 8);
        painter->setClipPath(path);

        // Draw highlighted background if selected
        if (item->isSelected())
            painter->fillRect(r, option.palette.highlight());

        // Draw icon
        const QSize& iconSize = item->listWidget()->iconSize();
        const int padding = r.height() / 2.0 - iconSize.height() / 2.0;
        const QRectF iconRect(QPointF(r.left() + padding, r.top() + padding), iconSize);
        const QPixmap& icon = PaintUtils::pixmap(item->icon(), iconSize, item->listWidget());
        painter->drawPixmap(iconRect, icon, icon.rect());

        // Draw texts
        QFont f;
        f.setWeight(QFont::Medium);
        painter->setFont(f);
        const QRectF nameRect(QPointF(iconRect.right() + padding, iconRect.top()),
                              QSizeF(r.width() - iconSize.width() - 3 * padding, iconRect.height() / 2.0));
        painter->setPen(option.palette.text().color());
        painter->drawText(nameRect, name, Qt::AlignVCenter | Qt::AlignLeft);

        f.setWeight(QFont::Normal);
        painter->setFont(f);
        const QRectF descRect(QPointF(iconRect.right() + padding, iconRect.center().y()),
                              QSizeF(r.width() - iconSize.width() - 3 * padding, iconRect.height() / 2.0));
        painter->drawText(descRect, description, Qt::AlignVCenter | Qt::AlignLeft);

        // Draw bottom line
        if (index.row() != item->listWidget()->count() - 1) {
            painter->setPen(QPen(QColor("#28000000"), 0));
            painter->drawLine(r.bottomLeft() + QPointF(padding, -0.5),
                              r.bottomRight() + QPointF(-padding, -0.5));
        }

        // Draw availability label
        if (availability != Available) {
            f.setPixelSize(f.pixelSize() - 2);
            const QString label = availability == Soon ? tr("SOON") : tr("IN FUTURE");
            const QFontMetrics fm(f);
            const int msgHeight = fm.height();
            const int msgWidth = fm.horizontalAdvance(label) + msgHeight;
            const QRectF msgRect(QPointF(r.left() + r.width() - msgWidth - padding,
                                         r.top() + r.height() / 2.0 - msgHeight / 2.0),
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

private:
    const QListWidget* m_listWidget;
};

PlatformSelectionWidget::PlatformSelectionWidget(QWidget* parent) : QWidget(parent)
  , m_platformList(new QListWidget(this))
  , m_buttonSlice(new ButtonSlice(this))
{
    auto updatePalette = [=] {
        QPalette p(m_platformList->palette());
        p.setColor(QPalette::Highlight, QStringLiteral("#16000000"));
        m_platformList->setPalette(p);
    };
    connect(qApp, &QApplication::paletteChanged, this, updatePalette);
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

    auto macOSItem = new QListWidgetItem();
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

    m_platformList->setIconSize(QSize(40, 40));
    m_platformList->setFixedSize(QSize(450, 326));
    m_platformList->setFocusPolicy(Qt::NoFocus);
    m_platformList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_platformList->setItemDelegate(new PlatformListDelegate(m_platformList, m_platformList));
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
    f.setWeight(QFont::ExtraLight);
    f.setPixelSize(26);
    titleLabel->setFont(f);

    f.setWeight(QFont::Light);
    f.setPixelSize(16);
    descriptionLabel->setFont(f);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    layout->addWidget(descriptionLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(8);
    layout->addWidget(platformsLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_platformList, 0, Qt::AlignHCenter);
    layout->addWidget(m_buttonSlice, 0, Qt::AlignHCenter);
}

QListWidget* PlatformSelectionWidget::platformList() const
{
    return m_platformList;
}

ButtonSlice* PlatformSelectionWidget::buttonSlice() const
{
    return m_buttonSlice;
}

PlatformSelectionWidget::Platform PlatformSelectionWidget::currentPlatform() const
{
    const QList<QListWidgetItem*>& selectedItems = m_platformList->selectedItems();
    if (selectedItems.isEmpty())
        return Invalid;
    return selectedItems.first()->data(PlatformRole).value<Platform>();
}