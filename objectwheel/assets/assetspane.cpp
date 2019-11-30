#include <assetspane.h>
#include <assetstree.h>
#include <lineedit.h>
#include <paintutils.h>

#include <QBoxLayout>
#include <QComboBox>

AssetsPane::AssetsPane(QWidget* parent) : QWidget(parent)
  , m_assetsTree(new AssetsTree(this))
  , m_modeComboBox(new QComboBox(this))
  , m_searchEdit(new LineEdit(this))
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_searchEdit->setClearButtonEnabled(true);
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
    return {300, 165};
}

QSize AssetsPane::minimumSizeHint() const
{
    return {0, 0};
}
