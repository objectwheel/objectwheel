#include <documentmanager.h>
#include <saveutils.h>
#include <fileutils.h>
#include <texteditor/texteditorsettings.h>

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

void DocumentManager::removeActiveProjectInfo()
{
    s_instance->m_modelManager.removeActiveProjectInfo();
}

void DocumentManager::updateActiveProjectInfo(const QString& projectDir)
{
    QmlJS::ModelManagerInterface::ProjectInfo projectInfo;
    projectInfo.qtVersionString = QLatin1String(qVersion());
    projectInfo.importPaths.maybeInsert(Utils::FileName::fromString(SaveUtils::toProjectImportsDir(projectDir)));
    s_instance->m_modelManager.setupProjectInfoQmlBundles(projectInfo);
    s_instance->m_modelManager.updateActiveProjectInfo(projectInfo);
}
