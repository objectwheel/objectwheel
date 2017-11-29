#include <platformswidget.h>
#include <fit.h>
#include <css.h>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QScrollBar>

#define pS (QApplication::primaryScreen())

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
     fit::fx(5), 0, - option.rect.height() / 2.0);
   auto rltext = option.rect.adjusted(option.rect.height(),
     option.rect.height() / 2.0, 0, - fit::fx(5));
   auto ricon = option.rect.adjusted(fit::fx(5), fit::fx(5),
     - option.rect.width() + option.rect.height() - fit::fx(5), - fit::fx(5));
   auto icon = item->icon().pixmap(ricon.size() * pS->devicePixelRatio());
    painter->setRenderHint(QPainter::Antialiasing);

    QFont f;
    f.setWeight(QFont::DemiBold);

    if (item->isSelected())
        painter->fillRect(option.rect, option.palette.highlight());

    painter->drawPixmap(ricon, icon, icon.rect());

    painter->setPen("#21303c");
    painter->setFont(f);
    painter->drawText(rutext, utext, Qt::AlignVCenter | Qt::AlignLeft);

    f.setWeight(QFont::Light);
    f.setPixelSize(f.pixelSize() - 1);
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
    QPalette p(palette());
    p.setColor(backgroundRole(), "#e0e4e7");
    setPalette(p);
    setAutoFillBackground(true);

    _layout.setContentsMargins(0, fit::fx(20), 0, fit::fx(20));
    _layout.setSpacing(fit::fx(10));
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

    _lblLogo.setFixedSize(fit::fx(50), fit::fx(50));
    _lblLogo.setPixmap(QPixmap(":/resources/images/helmet.png"));
    _lblLogo.setScaledContents(true);

    QFont f;
    f.setPixelSize(fit::fx(28));
    f.setWeight(QFont::ExtraLight);
    QPalette p2(_lblTitle.palette());
    p2.setColor(_lblTitle.foregroundRole(), "#21303c");
    _lblTitle.setFont(f);
    _lblTitle.setPalette(p2);
    _lblTitle.setText("Objectwheel Builds");

    f.setPixelSize(fit::fx(17));
    _lblMsg.setFont(f);
    _lblMsg.setPalette(p2);
    _lblMsg.setText("Select your platform");

    _btnNext.setColor("#F4BA48");
    _btnNext.setTextColor(Qt::white);
    _btnNext.setFixedSize(fit::fx(200),fit::fx(28));
    _btnNext.setRadius(fit::fx(7.5));
    _btnNext.setIconSize(QSize(fit::fx(14),fit::fx(14)));
    _btnNext.setIcon(QIcon(":/resources/images/unload.png"));
    _btnNext.setText("Next");
    connect(&_btnNext, &FlatButton::clicked, [&]{
        if (_listWidget.currentItem())
            emit platformSelected(Platforms
              (_listWidget.currentItem()->data(Key).toInt()));
    });

    QPalette p3(_listWidget.viewport()->palette());
    p3.setColor(_listWidget.viewport()->backgroundRole(), "#D0D4D7");
    p3.setColor(QPalette::Highlight, "#C3C7CA");
    _listWidget.viewport()->setPalette(p3);
    _listWidget.setPalette(p3);
    _listWidget.setIconSize(fit::fx(QSize{42, 42}));
    _listWidget.setItemDelegate(new PlatformDelegate(&_listWidget, &_listWidget));
    _listWidget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _listWidget.verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _listWidget.setFocusPolicy(Qt::NoFocus);
    connect(&_listWidget, &QListWidget::itemSelectionChanged, [&] {
       _btnNext.setEnabled(_listWidget.currentItem());
    });

    auto android = new QListWidgetItem;
    android->setText("Android 4.2+");
    android->setData(Arch, "Architecture: armeabi-v7a");
    android->setData(Key, android_armeabi_v7a);
    android->setIcon(QIcon(":/resources/images/android.png"));
    _listWidget.addItem(android);

    auto ioss = new QListWidgetItem;
    ioss->setText("iOS 10+");
    ioss->setData(Arch, "Architectures: armv7s, arm64");
    ioss->setData(Key, ios);
    ioss->setIcon(QIcon(":/resources/images/ios.png"));
    _listWidget.addItem(ioss);

    auto mac = new QListWidgetItem;
    mac->setText("macOS 10.10+");
    mac->setData(Arch, "Architecture: x64");
    mac->setData(Key, macos);
    mac->setIcon(QIcon(":/resources/images/macos.png"));
    _listWidget.addItem(mac);

    auto win = new QListWidgetItem;
    win->setText("Windows 7+");
    win->setData(Arch, "Architecture: x86");
    win->setData(Key, windows_x86);
    win->setIcon(QIcon(":/resources/images/windows.png"));
    _listWidget.addItem(win);

    auto linux = new QListWidgetItem;
    linux->setText("Linux");
    linux->setData(Arch, "Architecture: x86");
    linux->setData(Key, linux_x86);
    linux->setIcon(QIcon(":/resources/images/linux.png"));
    _listWidget.addItem(linux);

    auto rasp = new QListWidgetItem;
    rasp->setText("Raspberry Pi");
    rasp->setData(Arch, "Version: Pi 2+");
    rasp->setData(Key, raspi);
    rasp->setIcon(QIcon(":/resources/images/raspi.png"));
    _listWidget.addItem(rasp);

    auto androidx = new QListWidgetItem;
    androidx->setText("Android 4.2+");
    androidx->setData(Arch, "Architecture: x86");
    androidx->setData(Key, android_x86);
    androidx->setIcon(QIcon(":/resources/images/android.png"));
    _listWidget.addItem(androidx);

    auto winx = new QListWidgetItem;
    winx->setText("Windows 7+");
    winx->setData(Arch, "Architecture: x64");
    winx->setData(Key, windows_x64);
    winx->setIcon(QIcon(":/resources/images/windows.png"));
    _listWidget.addItem(winx);

    auto linuxx = new QListWidgetItem;
    linuxx->setText("Linux");
    linuxx->setData(Arch, "Architecture: x64");
    linuxx->setData(Key, linux_x64);
    linuxx->setIcon(QIcon(":/resources/images/linux.png"));
    _listWidget.addItem(linuxx);
    _listWidget.setCurrentRow(0);
}

#include "platformswidget.moc"
