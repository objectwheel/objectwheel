#include <formspane.h>
#include <formstree.h>
#include <lineedit.h>
#include <paintutils.h>
#include <applicationstyle.h>

#include <QBoxLayout>
#include <QPushButton>

FormsPane::FormsPane(QWidget* parent) : QWidget(parent)
  , m_formsTree(new FormsTree(this))
  , m_searchEdit(new LineEdit(this))
  , m_addButton(new QPushButton(this))
  , m_removeButton(new QPushButton(this))
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setPlaceholderText(tr("Search"));
    m_searchEdit->addAction(PaintUtils::renderOverlaidPixmap(":/images/search.svg", "#595959", QSize(16, 16), this),
                            QLineEdit::LeadingPosition);

    ApplicationStyle::setButtonStyle(m_addButton, ApplicationStyle::Help);
    m_addButton->setFixedSize(18, 18);
    m_addButton->setCursor(Qt::PointingHandCursor);
    m_addButton->setToolTip(tr("Add new form to the project"));
    m_addButton->setIcon(QIcon(":/images/designer/plus.svg"));

    ApplicationStyle::setButtonStyle(m_removeButton, ApplicationStyle::Help);
    m_removeButton->setFixedSize(18, 18);
    m_removeButton->setCursor(Qt::PointingHandCursor);
    m_removeButton->setToolTip(tr("Remove selected form from the project"));
    m_removeButton->setIcon(QIcon(":/images/designer/minus.svg"));

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(m_formsTree);
    layout->addWidget(m_searchEdit);
    // Do not add buttons, since they will be put
    // under pinbar of the pane by the MainWindow
}

FormsTree* FormsPane::formsTree() const
{
    return m_formsTree;
}

LineEdit* FormsPane::searchEdit() const
{
    return m_searchEdit;
}

QPushButton* FormsPane::addButton() const
{
    return m_addButton;
}

QPushButton* FormsPane::removeButton() const
{
    return m_removeButton;
}

QSize FormsPane::sizeHint() const
{
    return QSize(190, 200);
}
