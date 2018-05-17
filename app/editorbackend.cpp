#include <editorbackend.h>
#include <texteditor/texteditorsettings.h>

QList<QmlCodeDocument*> EditorBackend::m_documents;

EditorBackend::EditorBackend()
{
    m_modelManager.delayedInitialization();
    m_textEditorSettings = new TextEditor::TextEditorSettings;
}

EditorBackend::~EditorBackend()
{
    delete m_textEditorSettings;
}

EditorBackend* EditorBackend::instance()
{
    static EditorBackend m_instance;
    return &m_instance;
}

