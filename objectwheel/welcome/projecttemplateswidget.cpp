#include <projecttemplateswidget.h>
#include <buttonslice.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <controlrenderingmanager.h>
#include <windowmanager.h>
#include <utilityfunctions.h>
#include <paintutils.h>
#include <styleditemdelegate.h>

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
#include <QApplication>

#define SIZE_LIST        (QSize(450, 315))
#define BUTTONS_WIDTH    (450)
#define PATH_NICON       (":/images/welcome/load.png")
#define PATH_BICON       (":/images/welcome/unload.png")

namespace {
enum Buttons { Back, Next };
enum Roles { NameRole = Qt::UserRole + 1, DescriptionRole };
extern const QStringList NAMES;
extern const QStringList DESCRIPTIONS;
} // Anonymous Namespace

class ProjectTemplatesListDelegate final : public StyledItemDelegate
{
    Q_DISABLE_COPY(ProjectTemplatesListDelegate)

public:
    explicit ProjectTemplatesListDelegate(QObject* parent = nullptr) : StyledItemDelegate(parent)
    {}

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        return QSize(StyledItemDelegate::sizeHint(option, index).width(),
                     option.decorationSize.height() + 14);
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        painter->save();

        StyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

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
                              QSizeF(opt.rect.width() - opt.decorationSize.width() - 3 * padding, iconRect.height() / 2.0));
        painter->setPen(opt.palette.text().color());
        painter->drawText(nameRect, index.data(NameRole).toString(), Qt::AlignVCenter | Qt::AlignLeft);

        QFont f;
        f.setPixelSize(f.pixelSize() - 2);
        painter->setFont(f);
        painter->setPen(QColor(0, 0, 0, opt.state.testFlag(QStyle::State_Enabled) ? 160 : 100));
        const QRectF descriptionRect(QPointF(iconRect.right() + padding, iconRect.center().y()),
                                     QSizeF(opt.rect.width() - opt.decorationSize.width() - 3 * padding,
                                            iconRect.height() / 2.0));
        painter->drawText(descriptionRect, index.data(DescriptionRole).toString(), Qt::AlignVCenter | Qt::AlignLeft);

        // Draw bottom line
        if (index.row() != index.model()->rowCount() - 1) {
            painter->setPen(QPen(QColor("#28000000"), 0));
            painter->drawLine(opt.rect.bottomLeft() + QPointF(padding, 0.5),
                              opt.rect.bottomRight() + QPointF(-padding, 0.5));
        }

        painter->restore();
    }
};

ProjectTemplatesWidget::ProjectTemplatesWidget(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_iconLabel(new QLabel)
  , m_titleLabel(new QLabel)
  , m_informativeLabel(new QLabel)
  , m_templatesLabel(new QLabel)
  , m_listWidget(new QListWidget)
  , m_buttons(new ButtonSlice)
{
    setFocusPolicy(Qt::NoFocus);

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

    m_iconLabel->setFixedSize(QSize(60, 60));
    m_iconLabel->setPixmap(PaintUtils::pixmap(":/images/welcome/templates.svg", QSize(60, 60), this));

    QFont f = UtilityFunctions::systemTitleFont();
    f.setWeight(QFont::Light);
    f.setPixelSize(24);

    m_titleLabel->setFont(f);
    m_titleLabel->setText(tr("Project Templates"));

    f.setWeight(QFont::Light);
    f.setPixelSize(16);
    m_informativeLabel->setFont(f);
    m_informativeLabel->setText(tr("Choose appropriate template for your project"));
    m_informativeLabel->setAlignment(Qt::AlignCenter);

    m_templatesLabel->setText(tr("Templates"));

    auto updatePalette = [=] {
        QPalette p(QApplication::palette());
        p.setColor(QPalette::Highlight, "#16000000");
        m_listWidget->setPalette(p);
    };
    connect(qApp, &QApplication::paletteChanged, this, updatePalette);
    updatePalette();
    m_listWidget->setUniformItemSizes(true);
    m_listWidget->viewport()->installEventFilter(this);
    m_listWidget->setIconSize(QSize(40, 40));
    m_listWidget->setItemDelegate(new ProjectTemplatesListDelegate(m_listWidget));
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    m_listWidget->setFixedSize(SIZE_LIST);
    m_listWidget->verticalScrollBar()->setStyleSheet(
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
    m_listWidget->setStyleSheet(
                QString {
                    "QListWidget {"
                    "    background: #12000000;"
                    "    border: 1px solid #22000000;"
                    "    border-radius: %1px;"
                    "}"
                }
                .arg(8));


    for (int i = 0; i < NAMES.size(); i++) {
        auto item = new QListWidgetItem("Test");
        item->setIcon(QIcon(QStringLiteral(":/images/welcome/template%1.svg").arg(i)));
        item->setData(NameRole, NAMES[i]);
        item->setData(DescriptionRole, DESCRIPTIONS[i]);
        m_listWidget->addItem(item);
    }

    m_listWidget->setCurrentRow(1);

    m_buttons->add(Back, "#5BC5F8", "#2592F9");
    m_buttons->add(Next, "#8BBB56", "#6EA045");
    m_buttons->get(Next)->setText(tr("Next"));
    m_buttons->get(Back)->setText(tr("Back"));
    m_buttons->get(Next)->setIcon(QIcon(PATH_NICON));
    m_buttons->get(Back)->setIcon(QIcon(PATH_BICON));
    m_buttons->get(Next)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    m_buttons->settings().cellWidth = BUTTONS_WIDTH / 2.0;
    m_buttons->triggerSettings();

    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &ProjectTemplatesWidget::onNextButtonClick);
    connect(m_buttons->get(Back), &QPushButton::clicked,
            this, &ProjectTemplatesWidget::back);
    connect(m_buttons->get(Next), &QPushButton::clicked,
            this, &ProjectTemplatesWidget::onNextButtonClick);
}

void ProjectTemplatesWidget::onNextButtonClick()
{
    if (!m_listWidget->currentItem()) {
        UtilityFunctions::showMessage(
                    this, tr("Oops"),
                    tr("Please select a template project first before moving forward."));
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
    QObject::tr("Application Project v2"),
    QObject::tr("Application Project v1")/*,
        QObject::tr("Full-featured Application Project v2"),
        QObject::tr("Full-featured Application Project v1")*/
};

const QStringList DESCRIPTIONS = {
    QObject::tr("A blank project with an empty Window"),
    QObject::tr("A plain application project with an ApplicationWindow from Quick Controls 2"),
    QObject::tr("A plain application project with an ApplicationWindow from Quick Controls 1")/*,
        QObject::tr("A full-featured template project with bunch of controls from Quick Controls 2"),
        QObject::tr("A full-featured template project with bunch of controls from Quick Controls 1")*/
};
} // Anonymous Namespace
