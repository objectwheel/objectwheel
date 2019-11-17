#include <assetscontroller.h>
#include <assetspane.h>
#include <assetstree.h>
#include <lineedit.h>
#include <filesystemcompletermodel.h>

#include <QComboBox>
#include <QCompleter>

AssetsController::AssetsController(AssetsPane* assetsPane, QObject* parent) : QObject(parent)
  , m_assetsPane(assetsPane)
  , m_fileSystemCompleterModel(new FileSystemCompleterModel(this))
{
    auto completer = new QCompleter(this);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    completer->setModel(m_fileSystemCompleterModel);
    m_assetsPane->searchEdit()->setCompleter(completer);

    connect(m_assetsPane->modeComboBox(), qOverload<int>(&QComboBox::activated),
            this, &AssetsController::onModeComboBoxActivation);
    connect(m_assetsPane->searchEdit(), &LineEdit::returnPressed,
            this, &AssetsController::onSearchEditReturnPress);
}

void AssetsController::discharge()
{
    m_assetsPane->assetsTree()->discharge();
    m_assetsPane->searchEdit()->clear();
    onSearchEditReturnPress();
}

void AssetsController::onModeComboBoxActivation()
{
    m_assetsPane->assetsTree()->setMode(m_assetsPane->modeComboBox()->currentData().value<AssetsTree::Mode>());
}

void AssetsController::onSearchEditReturnPress()
{
//    if (!m_searchEditCompleter->currentIndex().isValid())
//        return;

//    const QModelIndex& currentIndex = m_searchEditCompleter->popup()->currentIndex();

//    if (!currentIndex.isValid())
//        return;

//    const QString& fileName = currentIndex.data(Qt::EditRole).toString();
//    const QString& relativePath = currentIndex.data(Qt::DisplayRole).toString();

//    if (relativePath.isEmpty() || fileName.isEmpty())
//        return;

//    if (m_searchEdit->text() != fileName)
//        return;

//    const QString& path = QDir(m_fileSystemModel->filePath(mt(rootIndex()))).filePath(relativePath);
}


