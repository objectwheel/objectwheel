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
}

void DocumentManager::updateProjectInfo(const QString& projectDir)
{
    QmlJS::ModelManagerInterface::ProjectInfo projectInfo;
    projectInfo.qtVersionString = QLatin1String(qVersion());
    projectInfo.qtQmlPath = QFileInfo(QLibraryInfo::location(QLibraryInfo::Qml2ImportsPath)).canonicalFilePath();
    projectInfo.qtImportsPath = QFileInfo(QLibraryInfo::location(QLibraryInfo::ImportsPath)).canonicalFilePath();
    if (!projectDir.isEmpty())
        projectInfo.importPaths.maybeInsert(Utils::FileName::fromString(SaveUtils::toProjectImportsDir(projectDir)));
    s_instance->m_modelManager.setupProjectInfoQmlBundles(projectInfo);
    s_instance->m_modelManager.updateProjectInfo(projectInfo);
}
