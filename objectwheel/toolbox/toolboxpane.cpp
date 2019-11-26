#include <toolboxpane.h>
#include <toolboxtree.h>
#include <lineedit.h>
#include <paintutils.h>

#include <QBoxLayout>
#include <QDragEnterEvent>
#include <QMimeData>

ToolboxPane::ToolboxPane(QWidget* parent) : QWidget(parent)
  , m_searchEdit(new LineEdit(this))
  , m_toolboxTree(new ToolboxTree(this))
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAcceptDrops(true);

    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setPlaceholderText(tr("Search"));
    m_searchEdit->addAction(PaintUtils::renderOverlaidPixmap(":/images/search.svg", "#595959", QSize(16, 16), this),
                            QLineEdit::LeadingPosition);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(m_toolboxTree);
    layout->addWidget(m_searchEdit);
}

LineEdit* ToolboxPane::searchEdit() const
{
    return m_searchEdit;
}

ToolboxTree* ToolboxPane::toolboxTree() const
{
    return m_toolboxTree;
}

QSize ToolboxPane::sizeHint() const
{
    return {190, 470};
}

QSize ToolboxPane::minimumSizeHint() const
{
    return {0, 0};
}

void ToolboxPane::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat(QStringLiteral("application/x-objectwheel-tool")))
        event->accept();
}
