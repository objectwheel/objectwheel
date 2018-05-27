#include <editorbackend.h>
#include <texteditor/texteditorsettings.h>

EditorBackend* EditorBackend::s_instance = nullptr;
QList<QmlCodeDocument*> EditorBackend::m_documents;

EditorBackend::EditorBackend(QObject* parent) : QObject(parent)
{
    s_instance = this;
    m_modelManager.delayedInitialization();
    m_textEditorSettings = new TextEditor::TextEditorSettings;
}

EditorBackend::~EditorBackend()
{
    delete m_textEditorSettings;
    s_instance = nullptr;
}

EditorBackend* EditorBackend::instance()
{
    return s_instance;
}

