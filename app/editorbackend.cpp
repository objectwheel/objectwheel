#include <editorbackend.h>

QList<QmlCodeEditor*> EditorBackend::m_editors;
QList<QmlCodeDocument*> EditorBackend::m_documents;

EditorBackend::EditorBackend()
{
    m_modelManager.delayedInitialization();
    Core::HelpManager::setupHelpManager();
    Utils::setCreatorTheme(Core::Internal::ThemeEntry::createTheme("flat"));
    connect(qApp, &QCoreApplication::aboutToQuit,
            &m_helpManager, &Core::HelpManager::aboutToShutdown);
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

