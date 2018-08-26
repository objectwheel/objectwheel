#include <issuesbox.h>
#include <control.h>
#include <outputpane.h>
#include <utilsicons.h>
#include <transparentstyle.h>

#include <QToolBar>
#include <QToolButton>
#include <QLabel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QListWidget>

class IssuesListDelegate: public QStyledItemDelegate
{
        Q_OBJECT

    public:
        IssuesListDelegate(QWidget* parent);
        void paint(QPainter* painter, const QStyleOptionViewItem &option,
          const QModelIndex &index) const override;
};

IssuesListDelegate::IssuesListDelegate(QWidget* parent)
    : QStyledItemDelegate(parent)
{
}

void IssuesListDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    const QAbstractItemModel* model = index.model();
    Q_ASSERT(model);
    painter->setRenderHint(QPainter::Antialiasing);

    QStyledItemDelegate::paint(painter, option, index);

    auto f = option.font;
    auto r = QRectF(option.rect).adjusted(0.5, 0.5, -0.5, -0.5);
    Error error = model->data(index, Qt::UserRole).value<Error>();
    painter->setPen("#a0a4a7");
    painter->drawLine(r.bottomLeft(), r.bottomRight());
    painter->setPen(option.palette.Text);
    f.setWeight(QFont::Medium);
    painter->setFont(f);
    painter->drawText(r.adjusted(26, 0, 0, 0),
      error.id + ":", Qt::AlignVCenter | Qt::AlignLeft);
    QFontMetrics fm(f);
    f.setWeight(QFont::Normal);
    painter->setFont(f);
    painter->drawText(r.adjusted(26.0 + fm.horizontalAdvance(error.id) + 8, 0, 0, 0),
      error.description, Qt::AlignVCenter | Qt::AlignLeft);
    painter->drawText(r, QString("Line: %1, Col: %2 ").
      arg(error.line).arg(error.column), Qt::AlignVCenter | Qt::AlignRight);
}

IssuesBox::IssuesBox(OutputPane* outputPane) : QWidget(outputPane)
  , m_layout(new QVBoxLayout(this))
  , m_toolbar(new QToolBar)
  , m_clearButton(new QToolButton)
  , m_title(new QLabel)
  , m_listWidget(new QListWidget)
  , m_outputPane(outputPane)
{
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_toolbar);
    m_layout->addWidget(m_listWidget);

    QPalette p1(m_listWidget->palette());
    p1.setColor(QPalette::Base, Qt::white);
    p1.setColor(QPalette::Highlight, QColor("#d4d4d4"));
    p1.setColor(QPalette::Text, Qt::black);
    m_listWidget->setPalette(p1);
    m_title->setText("Issues");

    TransparentStyle::attach(m_toolbar);

    m_title->setFixedHeight(22);
    m_clearButton->setFixedHeight(22);

    m_toolbar->setFixedHeight(24);
    auto spacing = new QWidget(this);
    spacing->setFixedSize(2, 24);
    auto spacing_2 = new QWidget(this);
    spacing_2->setFixedSize(5, 24);
    m_toolbar->addWidget(spacing);
    m_toolbar->addWidget(m_title);
    m_toolbar->addWidget(spacing_2);
    m_toolbar->addSeparator();
    m_toolbar->addWidget(m_clearButton);

    m_clearButton->setIcon(Utils::Icons::CLEAN_TOOLBAR.icon());
    m_clearButton->setToolTip(tr("Clean errors."));
    m_clearButton->setCursor(Qt::PointingHandCursor);
    connect(m_clearButton, SIGNAL(clicked(bool)), SLOT(clear()));

    m_listWidget->setIconSize({16, 16});
    m_listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setItemDelegate(new IssuesListDelegate(m_listWidget));

    connect(m_listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
      SLOT(handleDoubleClick(QListWidgetItem*)));
}

void IssuesBox::handleErrors(Control* control)
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
            if (m_buggyControls.contains(err))
                continue;
            auto item = new QListWidgetItem;
            item->setData(Qt::UserRole, QVariant::fromValue<Error>(err));
            item->setIcon(QIcon(":/images/error.png"));
            m_listWidget->addItem(item);
            m_buggyControls[err] = control;
            m_outputPane->shine(OutputPane::Issues);
        }
    }
}

void IssuesBox::handleDoubleClick(QListWidgetItem* item)
{
    const auto& error = item->data(Qt::UserRole).value<Error>();
    const auto& c = m_buggyControls.value(error);

    if (c == nullptr)
        return;
    emit entryDoubleClicked(c);
}

void IssuesBox::sweep()
{
    clear();
}

void IssuesBox::refresh()
{
    for (const auto& err : m_buggyControls.keys()) {
        auto control = m_buggyControls.value(err);
        if (control.isNull() || !control->hasErrors()) {
            for (int i = 0; i < m_listWidget->count(); i++) {
                auto item = m_listWidget->item(i);
                if (item->data(Qt::UserRole).value<Error>() == err)
                    delete m_listWidget->takeItem(i);
            }
            m_buggyControls.remove(err);
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
                    if (item->data(Qt::UserRole).value<Error>() == err)
                        delete m_listWidget->takeItem(i);
                }
                m_buggyControls.remove(err);
            }
        }
    }

    m_outputPane->button(OutputPane::Issues)->setText
            (QString("Issues [%1]").arg(m_listWidget->count()));
}

void IssuesBox::clear()
{
    m_buggyControls.clear();
    m_listWidget->clear();
}

bool operator<(const Error& e1, const Error& e2)
{
    return (e1.uid + e1.description +
      QString::number(e1.column) +
      QString::number(e1.line)) <
     (e2.uid + e2.description +
      QString::number(e2.column) +
      QString::number(e2.line));
}

#include "issuesbox.moc"
