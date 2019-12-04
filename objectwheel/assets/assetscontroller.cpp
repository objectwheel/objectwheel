#include <assetscontroller.h>
#include <assetspane.h>
#include <assetstree.h>
#include <lineedit.h>
#include <filesystemcompletermodel.h>
#include <projectmanager.h>
#include <saveutils.h>
#include <interfacesettings.h>
#include <generalsettings.h>

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

    connect(m_assetsPane->modeComboBox(), qOverload<int>(&QComboBox::activated),
            this, &AssetsController::onModeComboBoxActivation);
    connect(m_assetsPane->searchEdit(), &LineEdit::returnPressed,
            this, &AssetsController::onSearchEditReturnPress);
    connect(m_assetsPane->assetsTree(), &AssetsTree::currentDirChanged,
            this, &AssetsController::onCurrentDirChange);
    connect(GeneralSettings::instance(), &GeneralSettings::designerStateReset,
            this, [=] {
        InterfaceSettings* settings = GeneralSettings::interfaceSettings();
        settings->begin();
        settings->setValue("AssetsPane.CurrentMode", AssetsTree::Viewer);
        settings->end();
        m_assetsPane->assetsTree()->setMode(AssetsTree::Viewer);
        m_assetsPane->modeComboBox()->setCurrentText(tr("Viewer"));
    });

    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    settings->begin();
    m_assetsPane->assetsTree()->setMode(settings->value<AssetsTree::Mode>("AssetsPane.CurrentMode", AssetsTree::Viewer));
    m_assetsPane->modeComboBox()->setCurrentText(m_assetsPane->assetsTree()->mode() == AssetsTree::Viewer ?
                                                     tr("Viewer") : tr("Explorer"));
    settings->end();
}

void AssetsController::charge()
{
    m_assetsPane->assetsTree()->setRootPath(SaveUtils::toProjectAssetsDir(ProjectManager::dir()));
}

void AssetsController::discharge()
{
    m_assetsPane->assetsTree()->clean();
    m_assetsPane->searchEdit()->clear();
}

void AssetsController::onModeComboBoxActivation()
{
    m_assetsPane->assetsTree()->setMode(m_assetsPane->modeComboBox()->currentData().value<AssetsTree::Mode>());

    if (GeneralSettings::interfaceSettings()->preserveDesignerState) {
        InterfaceSettings* settings = GeneralSettings::interfaceSettings();
        settings->begin();
        settings->setValue("AssetsPane.CurrentMode", m_assetsPane->assetsTree()->mode());
        settings->end();
    }
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
