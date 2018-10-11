#include <issuesbox.h>
#include <control.h>
#include <utilsicons.h>
#include <transparentstyle.h>
#include <utilityfunctions.h>

#include <QToolBar>
#include <QToolButton>
#include <QLabel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QListWidget>
#include <QTimer>
#include <QQmlError>

enum Roles
{
    ErrorRole = ErrorRole + 1,
    ControlRole
};

class IssuesListDelegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    IssuesListDelegate(QWidget* parent) : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter* painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const
    {
        const QAbstractItemModel* model = index.model();
        Q_ASSERT(model);
        painter->setRenderHint(QPainter::Antialiasing);

        QStyledItemDelegate::paint(painter, option, index);

        auto f = option.font;
        auto r = QRectF(option.rect).adjusted(0.5, 0.5, -0.5, -0.5);
        const QQmlError& error = model->data(index, ErrorRole).value<QQmlError>();
        const QPointer<Control>& control = model->data(index, ControlRole).value<QPointer<Control>>();

        if (control.isNull())
            return;

        painter->setPen("#a0a4a7");
        painter->drawLine(r.bottomLeft(), r.bottomRight());
        painter->setPen(option.palette.text().color());
        f.setWeight(QFont::Medium);
        painter->setFont(f);
        painter->drawText(r.adjusted(26, 0, 0, 0),
                          control->id() + ":", Qt::AlignVCenter | Qt::AlignLeft);
        QFontMetrics fm(f);
        f.setWeight(QFont::Normal);
        painter->setFont(f);
        painter->drawText(r.adjusted(26.0 + fm.horizontalAdvance(control->id()) + 8, 0, 0, 0),
                          error.description, Qt::AlignVCenter | Qt::AlignLeft);
        painter->drawText(r, QString("Line: %1, Col: %2 ").
                          arg(error.line).arg(error.column), Qt::AlignVCenter | Qt::AlignRight);
    }
};

IssuesBox::IssuesBox(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_listWidget(new QListWidget)
  , m_toolBar(new QToolBar(this))
  , m_titleLabel(new QLabel(this))
  , m_clearButton(new QToolButton(this))
  , m_fontSizeUpButton(new QToolButton(this))
  , m_fontSizeDownButton(new QToolButton(this))
  , m_minimizeButton(new QToolButton(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_listWidget);

    m_titleLabel->setText("   " + tr("Issues") + "   ");
    m_titleLabel->setFixedHeight(22);

    m_toolBar->addWidget(m_titleLabel);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(m_clearButton);
    m_toolBar->addWidget(m_fontSizeUpButton);
    m_toolBar->addWidget(m_fontSizeDownButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacerWidget(Qt::Horizontal));
    m_toolBar->addWidget(m_minimizeButton);
    m_toolBar->setFixedHeight(24);

    m_clearButton->setFixedHeight(22);
    m_clearButton->setIcon(Utils::Icons::CLEAN_TOOLBAR.icon());
    m_clearButton->setToolTip(tr("Clean issues list"));
    m_clearButton->setCursor(Qt::PointingHandCursor);
    connect(m_clearButton, &QToolButton::clicked,
            m_listWidget, &QListWidget::clear);

    m_fontSizeUpButton->setFixedHeight(22);
    m_fontSizeUpButton->setIcon(Utils::Icons::PLUS_TOOLBAR.icon());
    m_fontSizeUpButton->setToolTip(tr("Increase font size"));
    m_fontSizeUpButton->setCursor(Qt::PointingHandCursor);
    connect(m_fontSizeUpButton, &QToolButton::clicked,
            this, [=] { // TODO: Change this with zoomIn
        UtilityFunctions::adjustFontPixelSize(m_listWidget, 1);
    });

    m_fontSizeDownButton->setFixedHeight(22);
    m_fontSizeDownButton->setIcon(Utils::Icons::MINUS.icon());
    m_fontSizeDownButton->setToolTip(tr("Decrease font size"));
    m_fontSizeDownButton->setCursor(Qt::PointingHandCursor);
    connect(m_fontSizeDownButton, &QToolButton::clicked,
            this, [=] { // TODO: Change this with zoomOut
        UtilityFunctions::adjustFontPixelSize(m_listWidget, -1);
    });

    m_minimizeButton->setFixedHeight(22);
    m_minimizeButton->setIcon(Utils::Icons::CLOSE_SPLIT_BOTTOM.icon());
    m_minimizeButton->setToolTip(tr("Minimize the pane"));
    m_minimizeButton->setCursor(Qt::PointingHandCursor);
    connect(m_minimizeButton, &QToolButton::clicked,
            this, &IssuesBox::minimized);

    m_listWidget->setObjectName("m_listWidget");
    m_listWidget->setStyleSheet("#m_listWidget { border: 1px solid #c4c4c4;"
                                   "border-top: none; border-bottom: none;}");
    m_listWidget->setIconSize({16, 16});
    m_listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setItemDelegate(new IssuesListDelegate(m_listWidget));
    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &IssuesBox::handleDoubleClick);

    TransparentStyle::attach(m_toolBar);
    QTimer::singleShot(200, [=] { // Workaround for QToolBarLayout's obsolote serMargin function usage
        m_toolBar->setContentsMargins(0, 0, 0, 0);
        m_toolBar->layout()->setContentsMargins(0, 0, 0, 0); // They must be all same
        m_toolBar->layout()->setSpacing(0);
    });
}

void IssuesBox::sweep()
{
    clear();
}

void IssuesBox::refresh()
{
    for (const auto& err : m_defectiveControls.keys()) {
        auto control = m_defectiveControls.value(err);
        if (control.isNull() || !control->hasErrors()) {
            for (int i = 0; i < m_listWidget->count(); i++) {
                auto item = m_listWidget->item(i);
                if (item->data(ErrorRole).value<Error>() == err)
                    delete m_listWidget->takeItem(i);
            }
            m_defectiveControls.remove(err);
        } else {
            QList<Error> es;
            for (const auto& error : control->errors()) {
                Error e;
                e.id = control->id();
                e.uid = control->uid();
                e.description = error.description();
                e.line = error.line();
                e.column = error.column();
                es << e;
            }
            if (!es.contains(err)) {
                for (int i = 0; i < m_listWidget->count(); i++) {
                    auto item = m_listWidget->item(i);
                    if (item->data(ErrorRole).value<Error>() == err)
                        delete m_listWidget->takeItem(i);
                }
                m_defectiveControls.remove(err);
            }
        }
    }

    emit titleChanged(QString::fromUtf8("Issues [%1]").arg(m_listWidget->count()));
}

void IssuesBox::process(Control* control)
{
    refresh();
    if (control->hasErrors()) {
        for (const auto& error : control->errors()) {
            Error err;
            err.id = control->id();
            err.uid = control->uid();
            err.description = error.description();
            err.line = error.line();
            err.column = error.column();
            if (m_defectiveControls.contains(err))
                continue;
            auto item = new QListWidgetItem;
            item->setData(ErrorRole, QVariant::fromValue<Error>(err));
            item->setIcon(QIcon(":/images/error.png"));
            m_listWidget->addItem(item);
            m_defectiveControls[err] = control;
            emit flash();
        }
    }
}

void IssuesBox::clear()
{
    m_defectiveControls.clear();
    m_listWidget->clear();
}

void IssuesBox::onItemDoubleClick(QListWidgetItem* item)
{
    const auto& error = item->data(ErrorRole).value<Error>();
    const auto& c = m_defectiveControls.value(error);

    if (c == nullptr)
        return;
    emit itemDoubleClicked(c);
}

QSize IssuesBox::minimumSizeHint() const
{
    return {100, 100};
}

QSize IssuesBox::sizeHint() const
{
    return {100, 100};
}

#include "issuesbox.moc"
