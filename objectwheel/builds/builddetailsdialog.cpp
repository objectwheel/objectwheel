#include <builddetailsdialog.h>
#include <downloaddetailswidget.h>
#include <buildmodel.h>
#include <utilityfunctions.h>
#include <fontcolorssettings.h>

#include <QPlainTextEdit>
#include <QBoxLayout>
#include <QScrollBar>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTimer>

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

    m_detailsTextEdit->setReadOnly(true);
    m_detailsTextEdit->setFont(FontColorsSettings().toFont());
    UtilityFunctions::adjustFontPixelSize(m_detailsTextEdit, -2);

    connect(box, &QDialogButtonBox::rejected, this, &BuildDetailsDialog::reject);
    connect(m_view->model(), &QAbstractItemModel::rowsAboutToBeRemoved,
            this, [=] (const QModelIndex& parent, int first, int last) {
        for (; first <= last; ++first) {
            if (m_index == m_view->model()->index(first, 0, parent)) {
                setIndex(QModelIndex());
                reject();
                break;
            }
        }
    });
    connect(m_view->model(), &QAbstractItemModel::modelReset,
            this, [=] {
        setIndex(QModelIndex());
        reject();
    });
    connect(m_view->model(), &QAbstractItemModel::dataChanged,
            this, [=] (const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
        Q_ASSERT(topLeft == bottomRight);
        if (m_index == topLeft && (roles.isEmpty() || roles.contains(BuildModel::StatusRole))) {
            QScrollBar* bar = m_detailsTextEdit->verticalScrollBar();
            bool scrollDown = bar->value() == bar->maximum();
            QTextCursor cursor(m_detailsTextEdit->textCursor());
            cursor.movePosition(QTextCursor::End);
            int begin = cursor.position();
            cursor.insertText(m_index.data(BuildModel::StatusRole).toString().mid(begin));
            highlight(begin);
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
        if (index.isValid()) {
            m_detailsTextEdit->setPlainText(index.data(BuildModel::StatusRole).toString());
            highlight(0);
            QTimer::singleShot(100, this, [=] {
                QScrollBar* bar = m_detailsTextEdit->verticalScrollBar();
                bar->setValue(bar->maximum());
            });
        }
    }
}

void BuildDetailsDialog::highlight(int begin)
{
    // Highlight (thicken) hh:mm:ss at the start of each line
    QTextCursor cursor(m_detailsTextEdit->textCursor());
    cursor.setPosition(begin);
    QTextCharFormat format(cursor.charFormat());
    format.setFontWeight(QFont::Bold);

    if (!cursor.atBlockStart())
        cursor.movePosition(QTextCursor::NextBlock);

    while (!cursor.atEnd()) {
        cursor.setPosition(cursor.position() + 8, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(format);
        if (!cursor.atBlockStart())
            cursor.movePosition(QTextCursor::NextBlock);
    }
}
