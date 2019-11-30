#include <assetscontroller.h>
#include <assetspane.h>
#include <assetstree.h>
#include <lineedit.h>
#include <filesystemcompletermodel.h>

#include <QComboBox>
#include <QCompleter>
#include <QDir>

AssetsController::AssetsController(AssetsPane* assetsPane, QObject* parent) : QObject(parent)
  , m_assetsPane(assetsPane)
  , m_completer(new QCompleter(this))
  , m_fileSystemCompleterModel(new FileSystemCompleterModel(this))
{
    m_completer->setFilterMode(Qt::MatchContains);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    m_completer->setModel(m_fileSystemCompleterModel);
    m_assetsPane->searchEdit()->setCompleter(m_completer);
    // The following is unneeded since Up and Down shortcuts in the
    // DesignerPane are defined in WidgetWithChildrenShortcut context
    // UtilityFunctions::overrideShortcutFor(m_assetsPane->searchEdit(), [=] (QKeyEvent* event) {
    //     return event->key() == Qt::Key_Up || event->key() == Qt::Key_Down;
    // });

    connect(m_assetsPane->modeComboBox(), qOverload<int>(&QComboBox::activated),
            this, &AssetsController::onModeComboBoxActivation);
    connect(m_assetsPane->searchEdit(), &LineEdit::returnPressed,
            this, &AssetsController::onSearchEditReturnPress);
    connect(m_assetsPane->assetsTree(), &AssetsTree::currentDirChanged,
            this, &AssetsController::onCurrentDirChange);
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
    if (!m_completer->currentIndex().isValid())
        return;

    const QModelIndex& currentIndex = m_completer->popup()->currentIndex();

    if (!currentIndex.isValid())
        return;

    const QString& fileName = currentIndex.data(Qt::EditRole).toString();
    const QString& relativePath = currentIndex.data(Qt::DisplayRole).toString();

    if (relativePath.isEmpty() || fileName.isEmpty())
        return;

    if (QString::compare(m_assetsPane->searchEdit()->text(), fileName, Qt::CaseInsensitive) != 0)
        return;

    const QString& path = QDir(m_fileSystemCompleterModel->rootPath()).filePath(relativePath);

    m_assetsPane->assetsTree()->goToEntry(path);
}

void AssetsController::onCurrentDirChange(const QString& currentDir)
{
    m_fileSystemCompleterModel->setRootPath(currentDir);
}
