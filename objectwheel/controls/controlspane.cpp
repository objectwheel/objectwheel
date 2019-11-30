#include <controlspane.h>
#include <controlstree.h>
#include <lineedit.h>
#include <paintutils.h>
#include <QBoxLayout>

ControlsPane::ControlsPane(QWidget* parent) : QWidget(parent)
  , m_controlsTree(new ControlsTree(this))
  , m_searchEdit(new LineEdit(this))
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setPlaceholderText(tr("Search"));
    m_searchEdit->addAction(PaintUtils::renderOverlaidPixmap(":/images/search.svg", "#595959", QSize(16, 16), this),
                            QLineEdit::LeadingPosition);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(m_controlsTree);
    layout->addWidget(m_searchEdit);
}

ControlsTree* ControlsPane::controlsTree() const
{
    return m_controlsTree;
}

LineEdit* ControlsPane::searchEdit() const
{
    return m_searchEdit;
}

QSize ControlsPane::sizeHint() const
{
    return {300, 155};
}

QSize ControlsPane::minimumSizeHint() const
{
    return {0, 0};
}
