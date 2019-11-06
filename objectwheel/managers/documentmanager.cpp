#include <documentmanager.h>
#include <saveutils.h>
#include <fileutils.h>
#include <texteditor/texteditorsettings.h>

#include <QLibraryInfo>
#include <QFileInfo>

DocumentManager* DocumentManager::s_instance = nullptr;
QList<QmlCodeDocument*> DocumentManager::m_documents;

DocumentManager::DocumentManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    m_modelManager.delayedInitialization();
    m_textEditorSettings = new TextEditor::TextEditorSettings;

    auto conn = new QMetaObject::Connection;
    *conn = connect(&m_modelManager, &QmlJSTools::Internal::ModelManager::idle, [=] {
        disconnect(*conn);
        delete conn;
        emit initialized();
    });

    // NOTE: Filling up tools into the Toolbar needs basic type
    // information available, so we are initializing the basis
    updateProjectInfo(QString());
}

DocumentManager::~DocumentManager()
{
    delete m_textEditorSettings;
    s_instance = nullptr;
}

DocumentManager* DocumentManager::instance()
{
    return s_instance;
}

void DocumentManager::removeProjectInfo()
{
    s_instance->m_modelManager.removeProjectInfo();
    s_instance->m_modelManager.activateScan();
}

void DocumentManager::updateProjectInfo(const QString& projectDir)
{
    const QString& importsDir = projectDir.isEmpty() ? QString() : SaveUtils::toProjectImportsDir(projectDir);
    QmlJS::ModelManagerInterface::ProjectInfo projectInfo;
    projectInfo.qtVersionString = QLatin1String(qVersion());
    projectInfo.qtQmlPath = QFileInfo(QLibraryInfo::location(QLibraryInfo::Qml2ImportsPath)).canonicalFilePath();
    projectInfo.qtImportsPath = QFileInfo(QLibraryInfo::location(QLibraryInfo::ImportsPath)).canonicalFilePath();
    if (!importsDir.isEmpty()) {
        projectInfo.importPaths.maybeInsert(Utils::FileName::fromString(importsDir));
        // projectInfo.tryQmlDump = true;
        // projectInfo.qmlDumpPath = importsDir;
        // projectInfo.qmlDumpEnvironment = Utils::Environment::systemEnvironment();
        // projectInfo.sourceFiles.append(FileSystemUtils::searchFilesBySuffix({"qml", "js", "qmldir"}, importsDir));
    }
    s_instance->m_modelManager.setupProjectInfoQmlBundles(projectInfo);
    s_instance->m_modelManager.updateProjectInfo(projectInfo);
    s_instance->m_modelManager.activateScan();
}
