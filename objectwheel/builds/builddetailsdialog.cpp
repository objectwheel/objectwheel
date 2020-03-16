#include <builddetailsdialog.h>
#include <downloaddetailswidget.h>
#include <buildmodel.h>
#include <utilityfunctions.h>

#include <QPlainTextEdit>
#include <QBoxLayout>
#include <QScrollBar>
#include <QDialogButtonBox>
#include <QPushButton>

BuildDetailsDialog::BuildDetailsDialog(const QAbstractItemView* view, QWidget* parent) : QDialog(parent)
  , m_view(view)
  , m_downloadDetailsWidget(new DownloadDetailsWidget(view, this))
  , m_detailsTextEdit(new QPlainTextEdit(this))
{
    setWindowTitle(tr("Build Details"));

    auto box = new QDialogButtonBox(this);
    box->addButton(QDialogButtonBox::Close)->setCursor(Qt::PointingHandCursor);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(8);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(m_downloadDetailsWidget);
    layout->addWidget(m_detailsTextEdit);
    layout->addWidget(box);

    UtilityFunctions::adjustFontPixelSize(m_detailsTextEdit, -1);
    m_detailsTextEdit->setReadOnly(true);

    connect(box, &QDialogButtonBox::rejected, this, &BuildDetailsDialog::reject);
    connect(m_view->model(), &QAbstractItemModel::rowsAboutToBeRemoved,
            this, [=] (const QModelIndex& parent, int first, int last) {
        for (; first <= last; ++first) {
            if (m_index == m_view->model()->index(first, 0, parent)) {
                setIndex(QModelIndex());
                break;
            }
        }
    });
    connect(m_view->model(), &QAbstractItemModel::modelReset,
            this, [=] {
        setIndex(QModelIndex());
    });
    connect(m_view->model(), &QAbstractItemModel::dataChanged,
            this, [=] (const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>&) {
        Q_ASSERT(topLeft == bottomRight);
        if (m_index == topLeft) {
            QScrollBar* bar = m_detailsTextEdit->verticalScrollBar();
            const bool scrollDown = bar->value() == bar->maximum();
            m_detailsTextEdit->setPlainText(m_index.data(BuildModel::DetailsRole).toString());
            if (scrollDown)
                bar->setValue(bar->maximum());
        }
    });
}

BuildDetailsDialog::~BuildDetailsDialog()
{
    setIndex(QModelIndex());
}

const QModelIndex& BuildDetailsDialog::index() const
{
    return m_index;
}

void BuildDetailsDialog::setIndex(const QModelIndex& index)
{
    if (m_index != index) {
        m_index = index;
        m_detailsTextEdit->clear();
        m_downloadDetailsWidget->setIndex(index);
        if (m_index.isValid()) {
            QScrollBar* bar = m_detailsTextEdit->verticalScrollBar();
            const bool scrollDown = bar->value() == bar->maximum();
            m_detailsTextEdit->setPlainText(m_index.data(BuildModel::DetailsRole).toString());
            if (scrollDown)
                bar->setValue(bar->maximum());
        }
    }
}
