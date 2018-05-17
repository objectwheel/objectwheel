#ifndef EDITORBACKEND_H
#define EDITORBACKEND_H

#include <QObject>

#include <qmlcodeeditor.h>
#include <qmlcodedocument.h>

#include <qmljstools/qmljsmodelmanager.h>
#include <qmljstools/qmljsbundleprovider.h>
#include <qmljstools/qmljslocatordata.h>
#include <qmljstools/qmljsfunctionfilter.h>

class EditorBackend final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(EditorBackend)

    friend class BackendManager;

    EditorBackend();
    ~EditorBackend();

public:
    static EditorBackend* instance();

    static QList<QmlCodeDocument*> documents()
    { return m_documents; }

    static QmlCodeDocument* createDocument()
    { return (m_documents << new QmlCodeDocument(m_editors.last())).last(); }

    static QmlCodeEditor* createEditor()
    { return (m_editors << new QmlCodeEditor).last();  }

private:
    static QList<QmlCodeEditor*> m_editors;
    static QList<QmlCodeDocument*> m_documents;
    QmlJSTools::Internal::ModelManager m_modelManager;
    QmlJSTools::Internal::LocatorData m_locatorData;
    QmlJSTools::Internal::FunctionFilter m_functionFilter{&m_locatorData};
    QmlJSTools::BasicBundleProvider m_basicBundleProvider;
};

#endif // EDITORBACKEND_H