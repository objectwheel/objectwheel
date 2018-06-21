#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include <QObject>

#include <qmljstools/qmljsmodelmanager.h>
#include <qmljstools/qmljsbundleprovider.h>
#include <qmljstools/qmljslocatordata.h>
#include <qmljstools/qmljsfunctionfilter.h>

class QmlCodeDocument;

namespace TextEditor { class TextEditorSettings; }

class DocumentManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DocumentManager)

    friend class ApplicationCore;

public:
    static DocumentManager* instance();

    static QList<QmlCodeDocument*> documents()
    { return m_documents; }

    static void addDocument(QmlCodeDocument* document)
    { m_documents.append(document); }

    static void removeDocument(QmlCodeDocument* document)
    { m_documents.removeAll(document); }

private:
    explicit DocumentManager(QObject* parent = nullptr);
    ~DocumentManager();

private:
    static DocumentManager* s_instance;
    static QList<QmlCodeDocument*> m_documents;
    QmlJSTools::Internal::ModelManager m_modelManager;
    QmlJSTools::Internal::LocatorData m_locatorData;
    QmlJSTools::Internal::FunctionFilter m_functionFilter{&m_locatorData};
    QmlJSTools::BasicBundleProvider m_basicBundleProvider;
    TextEditor::TextEditorSettings* m_textEditorSettings;
};

#endif // DOCUMENTMANAGER_H