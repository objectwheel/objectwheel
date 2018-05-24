#ifndef EDITORBACKEND_H
#define EDITORBACKEND_H

#include <QObject>

#include <qmljstools/qmljsmodelmanager.h>
#include <qmljstools/qmljsbundleprovider.h>
#include <qmljstools/qmljslocatordata.h>
#include <qmljstools/qmljsfunctionfilter.h>

class QmlCodeDocument;

namespace TextEditor { class TextEditorSettings; }

class EditorBackend final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(EditorBackend)

    friend class BackendManager;

public:
    static EditorBackend* instance();

    static QList<QmlCodeDocument*> documents()
    { return m_documents; }

    static void addDocument(QmlCodeDocument* document)
    { m_documents.append(document); }

    static void removeDocument(QmlCodeDocument* document)
    { m_documents.removeAll(document); }

private:
    explicit EditorBackend(QObject* parent = nullptr);
    ~EditorBackend();

private:
    static QList<QmlCodeDocument*> m_documents;
    QmlJSTools::Internal::ModelManager m_modelManager;
    QmlJSTools::Internal::LocatorData m_locatorData;
    QmlJSTools::Internal::FunctionFilter m_functionFilter{&m_locatorData};
    QmlJSTools::BasicBundleProvider m_basicBundleProvider;
    TextEditor::TextEditorSettings* m_textEditorSettings;
};

#endif // EDITORBACKEND_H