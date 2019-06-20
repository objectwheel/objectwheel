#include <documentmanager.h>
#include <texteditor/texteditorsettings.h>

DocumentManager* DocumentManager::s_instance = nullptr;
QList<QmlCodeDocument*> DocumentManager::m_documents;

DocumentManager::DocumentManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    m_modelManager.delayedInitialization();
    m_textEditorSettings = new TextEditor::TextEditorSettings;
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

void DocumentManager::updateProjectInfo()
{
    QMetaObject::Connection conn;
    conn = connect(&instance()->m_modelManager, &QmlJSTools::Internal::ModelManager::idle, [=] {
        disconnect(conn);
        emit instance()->projectInfoUpdated();
    });
    instance()->m_modelManager.updateDefaultProjectInfo();
}
