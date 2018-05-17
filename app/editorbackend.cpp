#include <editorbackend.h>

QList<QmlCodeEditor*> EditorBackend::m_editors;
QList<QmlCodeDocument*> EditorBackend::m_documents;

EditorBackend::EditorBackend()
{
    m_modelManager.delayedInitialization();
}

EditorBackend::~EditorBackend()
{
}

EditorBackend* EditorBackend::instance()
{
    static EditorBackend m_instance;
    return &m_instance;
}

