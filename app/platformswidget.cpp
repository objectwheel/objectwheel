#include <platformswidget.h>
#include <build.h>
#include <utilityfunctions.h>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QScrollBar>

enum {
    Arch = Qt::UserRole + 1,
    Key,
};

//!
//! *********************** [PlatformDelegate] ***********************
//!

class PlatformDelegate: public QStyledItemDelegate
{
        Q_OBJECT

    public:
        PlatformDelegate(QListWidget* view, QWidget* parent);

        void paint(QPainter* painter, const QStyleOptionViewItem &option,
          const QModelIndex &index) const override;

    private:
        QListWidget* m_view;
};

PlatformDelegate::PlatformDelegate(QListWidget* view, QWidget* parent)
    : QStyledItemDelegate(parent)
    , m_view(view)
{
}

void PlatformDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
   auto item = m_view->item(index.row());
   Q_ASSERT(item);

   auto utext = item->text();
   auto ltext = item->data(Arch).toString();
   auto rutext = option.rect.adjusted(option.rect.height(),
     7, 0, - option.rect.height() / 2.0);
   auto rltext = option.rect.adjusted(option.rect.height(),
     option.rect.height() / 2.0, 0, - 7);
   auto ricon = option.rect.adjusted(7, 7,
     - option.rect.width() + option.rect.height() - 7, - 7);
   auto icon = item->icon().pixmap(UtilityFunctions::window(m_view), ricon.size());
    painter->setRenderHint(QPainter::Antialiasing);

    QFont f;
    f.setWeight(QFont::DemiBold);

    if (item->isSelected())
        painter->fillRect(option.rect, option.palette.highlight());

    painter->drawPixmap(ricon, icon, icon.rect());

    painter->setPen(option.palette.text().color());
    painter->setFont(f);
    painter->drawText(rutext, utext, Qt::AlignVCenter | Qt::AlignLeft);

    f.setWeight(QFont::Normal);
    painter->setFont(f);
    painter->drawText(rltext, ltext, Qt::AlignVCenter | Qt::AlignLeft);
}

//!
//! *********************** [PlatformsWidget] ***********************
//!

PlatformsWidget::PlatformsWidget(QWidget *parent)
    : QWidget(parent)
    , _layout(this)
{
    _layout.setContentsMargins(0, 20, 0, 20);
    _layout.setSpacing(10);
    _layout.addWidget(&_lblLogo);
    _layout.addWidget(&_lblTitle);
    _layout.addWidget(&_lblMsg);
    _layout.addWidget(&_listWidget);
    _layout.addWidget(&_btnNext);
    _layout.setAlignment(&_lblLogo, Qt::AlignHCenter);
    _layout.setAlignment(&_lblTitle, Qt::AlignHCenter);
    _layout.setAlignment(&_lblMsg, Qt::AlignHCenter);
    _layout.setAlignment(&_listWidget, Qt::AlignHCenter);
    _layout.setAlignment(&_btnNext, Qt::AlignHCenter);

    _lblLogo.setFixedSize(50, 50);
    _lblLogo.setPixmap(QPixmap(":/images/helmet.png"));
    _lblLogo.setScaledContents(true);

    QFont f;
    f.setPixelSize(28);
    f.setWeight(QFont::ExtraLight);
    _lblTitle.setFont(f);
    _lblTitle.setText("Objectwheel Builds");

    f.setPixelSize(17);
    f.setWeight(QFont::Light);
    _lblMsg.setFont(f);
    _lblMsg.setText("Select your target platform");

    _btnNext.settings().topColor = "#F4BA48";
    _btnNext.settings().bottomColor = _btnNext.settings().topColor.darker(120);
    _btnNext.settings().borderRadius = 7.5;
    _btnNext.settings().textColor = Qt::white;
    _btnNext.setFixedSize(200,28);
    _btnNext.setIconSize(QSize(14,14));
    _btnNext.setIcon(QIcon(":/images/load.png"));
    _btnNext.setText("Next");
    connect(&_btnNext, SIGNAL(clicked(bool)),
      SLOT(handleBtnNextClicked()));

    _listWidget.setIconSize({52, 52});
    _listWidget.setMinimumWidth(400);
    _listWidget.setItemDelegate(new PlatformDelegate(&_listWidget, &_listWidget));
    _listWidget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _listWidget.setFocusPolicy(Qt::NoFocus);
    connect(&_listWidget, &QListWidget::itemSelectionChanged, [&] {
       _btnNext.setEnabled(_listWidget.currentItem());
    });

    auto android = new QListWidgetItem;
    android->setText("Android 4.0+");
    android->setData(Arch, "Architecture: armeabi-v7a");
    android->setData(Key, OTargets::android_armeabi_v7a);
    android->setIcon(QIcon(":/images/android.png"));
    _listWidget.addItem(android);

    auto ioss = new QListWidgetItem;
    ioss->setText("iOS 10+");
    ioss->setData(Arch, "Architectures: armv7s, arm64");
    ioss->setData(Key, OTargets::ios);
    ioss->setIcon(QIcon(":/images/ios.png"));
    _listWidget.addItem(ioss);

    auto mac = new QListWidgetItem;
    mac->setText("macOS 10.10+");
    mac->setData(Arch, "Architecture: x64");
    mac->setData(Key, OTargets::macos);
    mac->setIcon(QIcon(":/images/macos.png"));
    _listWidget.addItem(mac);

    auto win = new QListWidgetItem;
    win->setText("Windows 7+");
    win->setData(Arch, "Architecture: x86");
    win->setData(Key, OTargets::windows_x86);
    win->setIcon(QIcon(":/images/windows.png"));
    _listWidget.addItem(win);

    auto linx = new QListWidgetItem;
    linx->setText("Linux");
    linx->setData(Arch, "Architecture: x86");
    linx->setData(Key, OTargets::linux_x86);
    linx->setIcon(QIcon(":/images/linux.png"));
    _listWidget.addItem(linx);

    auto rasp = new QListWidgetItem;
    rasp->setText("Raspberry Pi");
    rasp->setData(Arch, "Version: Pi 2+");
    rasp->setData(Key, OTargets::raspi);
    rasp->setIcon(QIcon(":/images/raspi.png"));
    _listWidget.addItem(rasp);

    auto androidx = new QListWidgetItem;
    androidx->setText("Android 4.0+");
    androidx->setData(Arch, "Architecture: x86");
    androidx->setData(Key, OTargets::android_x86);
    androidx->setIcon(QIcon(":/images/android.png"));
    _listWidget.addItem(androidx);

    auto winx = new QListWidgetItem;
    winx->setText("Windows 7+");
    winx->setData(Arch, "Architecture: x64");
    winx->setData(Key, OTargets::windows_x64);
    winx->setIcon(QIcon(":/images/windows.png"));
    _listWidget.addItem(winx);

    auto linuxx = new QListWidgetItem;
    linuxx->setText("Linux");
    linuxx->setData(Arch, "Architecture: x64");
    linuxx->setData(Key, OTargets::linux_x64);
    linuxx->setIcon(QIcon(":/images/linux.png"));
    _listWidget.addItem(linuxx);
    _listWidget.setCurrentRow(0);
}

void PlatformsWidget::handleBtnNextClicked()
{
    if (_listWidget.currentItem()) {
        OTargets::Targets target = (OTargets::Targets)_listWidget.
          currentItem()->data(Key).toInt();
        Build::set(TAG_TARGET, target);
        emit platformSelected(target);
    }
}

#include "platformswidget.moc"
