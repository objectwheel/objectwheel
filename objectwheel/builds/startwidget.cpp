#include <startwidget.h>
#include <paintutils.h>

#include <QBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QStyledItemDelegate>
#include <QScrollBar>
#include <QApplication>
#include <QPainter>

enum {
    Name = Qt::UserRole + 1,
    Decription,
    Availability
};

class PlatformListDelegate final : public QStyledItemDelegate
{
    Q_DISABLE_COPY(PlatformListDelegate)

public:
    PlatformListDelegate(QListWidget* listWidget, QWidget* parent) : QStyledItemDelegate(parent)
      , m_platformList(listWidget)
    {
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        auto item = m_platformList->item(index.row());
        if (item == 0)
            return;

        auto name = item->data(Name).toString();
        auto lastEdit = item->data(Decription).toString();

        auto rn = QRectF(option.rect).adjusted(option.rect.height(),
                                               7, 0, - option.rect.height() / 2.0);
        auto rl = QRectF(option.rect).adjusted(option.rect.height(),
                                               option.rect.height() / 2.0, 0, - 7);
        auto ri = QRectF(option.rect).adjusted(7, 7,
                                               - option.rect.width() + option.rect.height() - 7, - 7);
        auto ra = ri.adjusted(3, -0.5, 0, 0);
        ra.setSize(QSize(10, 10));
        auto icon = PaintUtils::pixmap(item->icon(), ri.size().toSize(), m_platformList,
                                       m_platformList->isEnabled() ? QIcon::Normal : QIcon::Disabled);

        painter->setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.addRoundedRect(m_platformList->rect(), 8, 8);
        painter->setClipPath(path);

        if (item->isSelected())
            painter->fillRect(option.rect, option.palette.highlight());

        painter->drawPixmap(ri, icon, icon.rect());

//        if (item->data(Active).toBool()) {
//            QLinearGradient g(ri.topLeft(), ri.bottomLeft());
//            g.setColorAt(0, "#6BCB36");
//            g.setColorAt(0.5, "#4db025");
//            painter->setBrush(g);
//            painter->setPen("#6BCB36");
//            painter->drawRoundedRect(ra, ra.width(), ra.height());
//        }

        QFont f;
        f.setWeight(QFont::Medium);
        painter->setFont(f);
        painter->setPen(option.palette.text().color());
        painter->drawText(rn, name, Qt::AlignVCenter | Qt::AlignLeft);

        f.setWeight(QFont::Normal);
        painter->setFont(f);
        painter->drawText(rl, lastEdit, Qt::AlignVCenter | Qt::AlignLeft);
    }

private:
    QListWidget* m_platformList;
};

StartWidget::StartWidget(QWidget* parent) : QWidget(parent)
  , m_platformList(new QListWidget(this))
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
    androidItem->setData(Name, QLatin1String("Android 5.0+"));
    androidItem->setData(Decription, tr("Supported ABIs: armeabi-v7a, arm64-v8a, x86, x86_64"));
    androidItem->setData(Availability, true);
    m_platformList->addItem(androidItem);

    auto iOSItem = new QListWidgetItem;
    iOSItem->setIcon(QIcon(":/images/builds/ios.svg"));
    iOSItem->setData(Name, QLatin1String("iOS 12+"));
    iOSItem->setData(Decription, tr("Supported ABIs: arm64-v8a, x86_64 (simulator)"));
    iOSItem->setData(Availability, false);
    m_platformList->addItem(iOSItem);

    auto macOSItem = new QListWidgetItem();
    macOSItem->setIcon(QIcon(":/images/builds/macos.svg"));
    macOSItem->setData(Name, QLatin1String("macOS 10.13+"));
    macOSItem->setData(Decription, tr("Supported ABIs: x86_64"));
    macOSItem->setData(Availability, false);
    m_platformList->addItem(macOSItem);

    auto windowsItem = new QListWidgetItem;
    windowsItem->setIcon(QIcon(":/images/builds/windows.svg"));
    windowsItem->setData(Name, QLatin1String("Windows 7+"));
    windowsItem->setData(Decription, tr("Supported ABIs: x86, x86_64"));
    windowsItem->setData(Availability, false);
    m_platformList->addItem(windowsItem);

    auto linuxItem = new QListWidgetItem;
    linuxItem->setIcon(QIcon(":/images/builds/linux.svg"));
    linuxItem->setData(Name, QLatin1String("Linux (X11)"));
    linuxItem->setData(Decription, tr("Supported ABIs: x86, x86_64"));
    linuxItem->setData(Availability, false);
    m_platformList->addItem(linuxItem);

    auto raspberryPiItem = new QListWidgetItem;
    raspberryPiItem->setIcon(QIcon(":/images/builds/raspberry.svg"));
    raspberryPiItem->setData(Name, QLatin1String("Raspberry Pi 2+"));
    raspberryPiItem->setData(Decription, tr("Supported ABIs: armeabi-v7a, arm64-v8a"));
    raspberryPiItem->setData(Availability, false);
    m_platformList->addItem(raspberryPiItem);

    m_platformList->viewport()->installEventFilter(this);
    m_platformList->setIconSize(QSize(48, 48));
    m_platformList->setMinimumWidth(400);
    m_platformList->setItemDelegate(new PlatformListDelegate(m_platformList, m_platformList));
    m_platformList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_platformList->setFocusPolicy(Qt::NoFocus);
    m_platformList->setFixedSize(QSize(450, 314));
    m_platformList->verticalScrollBar()->setStyleSheet(
                QString {
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
                    "}"
                }
                .arg(15)
                .arg(6)
                .arg(2.5));
    m_platformList->setStyleSheet(
                QString {
                    "QListWidget {"
                    "    background: #12000000;"
                    "    border: 1px solid #22000000;"
                    "    border-radius: %1px;"
                    "}"
                }
                .arg(8));

    auto iconLabel = new QLabel(this);
    auto titleLabel = new QLabel(tr("Objectwheel Cloud Builds"), this);
    auto descriptionLabel = new QLabel(tr("Select your target platform to start"), this);
    auto platformsLabel = new QLabel(tr("Platforms:"));

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
    layout->addWidget(iconLabel, 1, 1, Qt::AlignHCenter);
    layout->addWidget(titleLabel, 2, 1, Qt::AlignHCenter);
    layout->addWidget(descriptionLabel, 3, 1, Qt::AlignHCenter);
    layout->setRowMinimumHeight(4, 8);
    layout->addWidget(platformsLabel, 5, 1, Qt::AlignHCenter);
    layout->addWidget(m_platformList, 6, 1, Qt::AlignHCenter);
    layout->setRowStretch(7, 1);

    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/builds/gift.svg"),
                                            QSize(60, 60), this));

    QFont f;
    f.setWeight(QFont::ExtraLight);
    f.setPixelSize(26);
    titleLabel->setFont(f);

    f.setWeight(QFont::Light);
    f.setPixelSize(16);
    descriptionLabel->setFont(f);
}
