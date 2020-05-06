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

    Q_ASSERT(m_view);
    Q_ASSERT(m_view->model());
    connect(box, &QDialogButtonBox::rejected, this, &BuildDetailsDialog::reject);
    connect(m_view->model(), &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &BuildDetailsDialog::onRowsAboutToBeRemoved);
    connect(m_view->model(), &QAbstractItemModel::modelReset,
            this, &BuildDetailsDialog::onModelReset);
    connect(m_view->model(), &QAbstractItemModel::dataChanged,
            this, &BuildDetailsDialog::onDataChange);
}

BuildDetailsDialog::~BuildDetailsDialog()
{
    setIdentifier(QString());
}

QString BuildDetailsDialog::identifier() const
{
    return m_identifier;
}

void BuildDetailsDialog::setIdentifier(const QString& identifier)
{
    if (m_identifier != identifier) {
        m_identifier = identifier;
        m_detailsTextEdit->clear();
        m_downloadDetailsWidget->setIdentifier(identifier);
        if (index().isValid()) {
            m_detailsTextEdit->setPlainText(index().data(BuildModel::StatusRole).toString());
            highlight(0);
            QTimer::singleShot(100, this, [=] { // FIXME: For some reason doesn't work without a timer
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

void BuildDetailsDialog::onModelReset()
{
    setIdentifier(QString());
    reject();
}

void BuildDetailsDialog::onRowsAboutToBeRemoved(const QModelIndex&, int first, int last)
{
    for (; first <= last; ++first) {
        if (first == index().row()) {
            setIdentifier(QString());
            reject();
            break;
        }
    }
}

void BuildDetailsDialog::onDataChange(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                                      const QVector<int>& roles)
{
    Q_ASSERT(topLeft == bottomRight);
    if (index() == topLeft && (roles.isEmpty() || roles.contains(BuildModel::StatusRole))) {
        QScrollBar* bar = m_detailsTextEdit->verticalScrollBar();
        bool scrollDown = bar->value() == bar->maximum();
        QTextCursor cursor(m_detailsTextEdit->textCursor());
        cursor.movePosition(QTextCursor::End);
        int begin = cursor.position();
        cursor.insertText(index().data(BuildModel::StatusRole).toString().mid(begin));
        highlight(begin);
        if (scrollDown)
            bar->setValue(bar->maximum());
    }
}

QModelIndex BuildDetailsDialog::index() const
{
    return m_view->model() ? static_cast<BuildModel*>(m_view->model())->indexFromIdentifier(m_identifier) : QModelIndex();
}

void BuildDetailsDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    // For some reason (probably a Qt bug) the text edit
    // sometimes stops updating its geometry and a blank
    // space shows up at the bottom, so we force a new
    // re-evaluation for the geometry of the text edit
    m_detailsTextEdit->resize(200, 200);
    layout()->invalidate();
}
