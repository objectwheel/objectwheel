#include <navigatorpane.h>
#include <navigatortree.h>
#include <lineedit.h>
#include <paintutils.h>
#include <QBoxLayout>

NavigatorPane::NavigatorPane(QWidget* parent) : QWidget(parent)
  , m_navigatorTree(new NavigatorTree(this))
  , m_searchEdit(new LineEdit(this))
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setPlaceholderText(tr("Search"));
    m_searchEdit->addAction(PaintUtils::renderOverlaidPixmap(":/images/search.svg", "#595959",
                                                             m_searchEdit->devicePixelRatioF()),
                            QLineEdit::LeadingPosition);
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(3, 3, 3, 3);
    layout->addWidget(m_navigatorTree);
    layout->addWidget(m_searchEdit);
}

NavigatorTree* NavigatorPane::navigatorTree() const
{
    return m_navigatorTree;
}

LineEdit* NavigatorPane::searchEdit() const
{
    return m_searchEdit;
}

QSize NavigatorPane::sizeHint() const
{
    return {310, 220};
}
