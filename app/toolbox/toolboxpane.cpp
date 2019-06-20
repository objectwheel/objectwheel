#include <toolboxpane.h>
#include <toolboxtree.h>
#include <lineedit.h>
#include <paintutils.h>
#include <QBoxLayout>

ToolboxPane::ToolboxPane(QWidget* parent) : QWidget(parent)
  , m_searchEdit(new LineEdit(this))
  , m_toolboxTree(new ToolboxTree(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setPlaceholderText(tr("Search"));
    m_searchEdit->addAction(PaintUtils::renderOverlaidPixmap(":/images/search.svg", "#595959", m_searchEdit),
                            QLineEdit::LeadingPosition);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(3, 3, 3, 3);
    layout->addWidget(m_searchEdit);
    layout->addWidget(m_toolboxTree);
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
    return {140, 710};
}
