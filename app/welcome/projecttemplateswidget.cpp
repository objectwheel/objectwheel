#include <projecttemplateswidget.h>
#include <buttonslice.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <delayer.h>
#include <filemanager.h>
#include <controlpreviewingmanager.h>
#include <progressbar.h>
#include <windowmanager.h>
#include <defaultfont.h>
#include <utilityfunctions.h>

#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QListWidget>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QDebug>
#include <QDateTime>
#include <QScreen>
#include <QFileDialog>

#define SIZE_LIST        (QSize(450, 315))
#define BUTTONS_WIDTH    (450)
#define SIZE_LOGO        (QSize(48, 48))
#define SIZE_TEMPLCON    (QSize(48, 48))
#define PATH_LOGO        (":/images/templates.png")
#define PATH_TEMPLATE    (":/images/template%1.png")
#define PATH_NICON       (":/images/load.png")
#define PATH_BICON       (":/images/unload.png")

namespace {
enum Buttons { Back, Next };
enum Roles { Name = Qt::UserRole + 1, Description };
extern const QStringList NAMES;
extern const QStringList DESCRIPTIONS;
} // Anonymous Namespace

class ProjectTemplatesDelegate: public QStyledItemDelegate
{
        Q_OBJECT

    public:
        ProjectTemplatesDelegate(QListWidget* listWidget, QWidget* parent);

        void paint(QPainter* painter, const QStyleOptionViewItem& option,
          const QModelIndex& index) const override;

    private:
        QListWidget* m_listWidget;
};

ProjectTemplatesDelegate::ProjectTemplatesDelegate(QListWidget* listWidget, QWidget* parent) : QStyledItemDelegate(parent)
  , m_listWidget(listWidget)
{
}

void ProjectTemplatesDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
   auto item = m_listWidget->item(index.row());
   Q_ASSERT(item);

   auto name = item->data(Name).toString();
   auto description = item->data(Description).toString();

   auto rn = QRectF(option.rect).adjusted(option.rect.height(),
     7, 0, - option.rect.height() / 2.0);
   auto rl = QRectF(option.rect).adjusted(option.rect.height(),
     option.rect.height() / 2.0, 0, - 7);
   auto ri = QRectF(option.rect).adjusted(7, 7,
     - option.rect.width() + option.rect.height() - 7, - 7);
   auto ra = ri.adjusted(3, -0.5, 0, 0);
   ra.setSize(QSize(10, 10));
   auto icon = item->icon().pixmap(UtilityFunctions::window(m_listWidget), ri.size().toSize());

   painter->setRenderHint(QPainter::Antialiasing);

   QPainterPath path;
   path.addRoundedRect(m_listWidget->rect(), 8, 8);
   painter->setClipPath(path);

    if (item->isSelected())
        painter->fillRect(option.rect, option.palette.highlight());

    painter->drawPixmap(ri, icon, icon.rect());

    QFont f(DefaultFont::font());
    f.setWeight(QFont::DemiBold);
    painter->setFont(f);
    painter->setPen(option.palette.text().color());
    painter->drawText(rn, name, Qt::AlignVCenter | Qt::AlignLeft);

    f.setPixelSize(f.pixelSize() - 2);
    f.setWeight(QFont::Normal);
    painter->setFont(f);
    painter->drawText(rl, description, Qt::AlignVCenter | Qt::AlignLeft);
}

ProjectTemplatesWidget::ProjectTemplatesWidget(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_iconLabel(new QLabel)
  , m_titleLabel(new QLabel)
  , m_informativeLabel(new QLabel)
  , m_templatesLabel(new QLabel)
  , m_listWidget(new QListWidget)
  , m_buttons(new ButtonSlice)
{
    m_layout->addStretch();
    m_layout->setSpacing(6);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    m_layout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    m_layout->addWidget(m_informativeLabel, 0, Qt::AlignCenter);
    m_layout->addSpacing(6);
    m_layout->addWidget(m_templatesLabel, 0, Qt::AlignCenter);
    m_layout->addWidget(m_listWidget, 0, Qt::AlignCenter);
    m_layout->addWidget(m_buttons, 0, Qt::AlignCenter);
    m_layout->addStretch();

    QPixmap p(PATH_LOGO);
    p.setDevicePixelRatio(devicePixelRatioF());

    m_iconLabel->setFixedSize(SIZE_LOGO);
    m_iconLabel->setPixmap(
        p.scaled(
            SIZE_LOGO * devicePixelRatioF(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    QFont f;
    f.setWeight(QFont::ExtraLight);
    f.setPixelSize(26);

    m_titleLabel->setFont(f);
    m_titleLabel->setText(tr("Project Templates"));

    f.setWeight(QFont::Light);
    f.setPixelSize(15);
    m_informativeLabel->setFont(f);
    m_informativeLabel->setText(tr("Choose appropriate template for your project"));
    m_informativeLabel->setAlignment(Qt::AlignCenter);

    m_templatesLabel->setText(tr("Templates"));

    QPalette p1;
    p1.setColor(QPalette::Highlight, "#15000000");
    m_listWidget->viewport()->installEventFilter(this);
    m_listWidget->setPalette(p1);
    m_listWidget->setIconSize(SIZE_TEMPLCON);
    m_listWidget->setMinimumWidth(400);
    m_listWidget->setItemDelegate(new ProjectTemplatesDelegate(m_listWidget, m_listWidget));
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    m_listWidget->setFixedSize(SIZE_LIST);
    m_listWidget->verticalScrollBar()->setStyleSheet(
        tr(
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
        ).
        arg(15).
        arg(6).
        arg(2.5)
    );
    m_listWidget->setStyleSheet(
        tr(
            "QListWidget {"
            "    background: #12000000;"
            "    border: 1px solid #22000000;"
            "    border-radius: %1px;"
            "}"
        )
        .arg(8)
    );

    for (int i = 0; i < NAMES.size(); i++) {
        auto item = new QListWidgetItem("Test");
        item->setIcon(QIcon(QString(PATH_TEMPLATE).arg(i)));
        item->setData(Name, NAMES[i]);
        item->setData(Description, DESCRIPTIONS[i]);
        m_listWidget->addItem(item);
    }

    m_listWidget->setCurrentRow(1);

    m_buttons->add(Back, "#5BC5F8", "#2592F9");
    m_buttons->add(Next, "#8BBB56", "#6EA045");
    m_buttons->get(Next)->setText(tr("Next"));
    m_buttons->get(Back)->setText(tr("Back"));
    m_buttons->get(Next)->setIcon(QIcon(PATH_NICON));
    m_buttons->get(Back)->setIcon(QIcon(PATH_BICON));
    m_buttons->get(Next)->setIconSize(QSize(16, 16));
    m_buttons->get(Back)->setIconSize(QSize(16, 16));
    m_buttons->get(Next)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    m_buttons->settings().cellWidth = BUTTONS_WIDTH / 2.0;
    m_buttons->triggerSettings();

    connect(m_buttons->get(Back), SIGNAL(clicked(bool)), SIGNAL(back()));
    connect(m_buttons->get(Next), SIGNAL(clicked(bool)), SLOT(onNextButtonClick()));
}

void ProjectTemplatesWidget::onNextButtonClick()
{
    if (!m_listWidget->currentItem()) {
        QMessageBox::warning(
            this,
            tr("Oops"),
            tr("Select a template first.")
        );
        return;
    }

    emit newProject(m_projectName, m_listWidget->currentRow());
}

void ProjectTemplatesWidget::onNewProject(const QString& projectName)
{
    m_projectName = projectName;
}

bool ProjectTemplatesWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_listWidget->viewport() && event->type() == QEvent::Paint && m_listWidget->count() == 0) {
        QPainter p(m_listWidget->viewport());
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen("#30000000");
        p.drawText(m_listWidget->viewport()->rect(), tr("No projects"), QTextOption(Qt::AlignCenter));
        return true;
    }

    return false;
}

namespace {
// TODO: Finish other templates
const QStringList NAMES = {
    QObject::tr("Blank Project"),
    QObject::tr("Plain Application Project"),
    QObject::tr("Hello World Application Project")/*,
    QObject::tr("Multi-page Application Project"),
    QObject::tr("Multi-window Application Project"),
    QObject::tr("Desktop Oriented Application Project"),
    QObject::tr("Simple Database Application Project"),
    QObject::tr("Advanced Application Project"),
    QObject::tr("Dummy Project (Quick Controls)"),
    QObject::tr("Dummy Project 2 (Quick Controls 2)")*/
};

const QStringList DESCRIPTIONS = {
    QObject::tr("A pure and clean project template with an empty 'Window'."),
    QObject::tr("A base project template for applications with an empty 'ApplicationWindow'."),
    QObject::tr("A simple application project template with a 'hello world' setup.")/*,
    QObject::tr("A base project template for multi-page applications."),
    QObject::tr("A base project template for multi-window applications."),
    QObject::tr("A base project template for desktop oriented applications."),
    QObject::tr("A simple application project template with a simple database setup."),
    QObject::tr("An advanced application project template with full of features loaded."),
    QObject::tr("A dummy project template to test and see Quick Controls v1"),
    QObject::tr("A dummy project template to test and see Quick Controls v2")*/
};
} // Anonymous Namespace

#include "projecttemplateswidget.moc"
