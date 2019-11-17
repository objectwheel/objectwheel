#include <assetspane.h>
#include <assetstree.h>
#include <lineedit.h>
#include <paintutils.h>

#include <QBoxLayout>
#include <QComboBox>

AssetsPane::AssetsPane(QWidget* parent) : QWidget(parent)
  , m_assetsTree(new AssetsTree(this))
  , m_modeComboBox(new QComboBox(this))
  , m_searchEditCompleterModel(new FileSearchModel(this))
  , m_searchEditCompleter(new QCompleter(this))
  , m_searchEdit(new LineEdit(this))
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto completer = new QCompleter(this);
    completer->setModel(&m_searchCompleterModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    m_controlsPane->searchEdit()->setCompleter(completer);

    connect(m_controlsPane->searchEdit(), &LineEdit::returnPressed,
            this, &ControlsController::onSearchEditReturnPress);

    connect(m_searchEdit, qOverload<>(&LineEdit::returnPressed),
            this, &FileExplorer::filterList);

    m_searchEditCompleter->setFilterMode(Qt::MatchContains);
    m_searchEditCompleter->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    m_searchEditCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_searchEditCompleter->setModel(m_searchEditCompleterModel);

    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setCompleter(m_searchEditCompleter);
    m_searchEdit->setPlaceholderText(tr("Search"));
    m_searchEdit->addAction(PaintUtils::renderOverlaidPixmap(":/images/search.svg", "#595959", QSize(16, 16), this),
                            QLineEdit::LeadingPosition);

    m_modeComboBox->setFixedHeight(18);
    m_modeComboBox->setCursor(Qt::PointingHandCursor);
    m_modeComboBox->setToolTip(tr("Change view mode"));
    m_modeComboBox->addItem(tr("Viewer"), QVariant::fromValue(AssetsTree::Viewer));
    m_modeComboBox->addItem(tr("Explorer"), QVariant::fromValue(AssetsTree::Explorer));

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(m_assetsTree);
    layout->addWidget(m_searchEdit);
    // We are not adding the m_modeComboBox, so the main window
    // controller can add it to the pin bar of the dock window
}

AssetsTree* AssetsPane::assetsTree() const
{
    return m_assetsTree;
}

QComboBox* AssetsPane::modeComboBox() const
{
    return m_modeComboBox;
}

LineEdit* AssetsPane::searchEdit() const
{
    return m_searchEdit;
}

QSize AssetsPane::sizeHint() const
{
    return {300, 240};
}
