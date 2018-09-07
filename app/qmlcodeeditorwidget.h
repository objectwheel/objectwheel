#ifndef QMLCODEEDITORWIDGET_H
#define QMLCODEEDITORWIDGET_H

#include <QWidget>
#include <QTextCursor>

class Control;
class QmlCodeDocument;
class QSplitter;
class QmlCodeEditor;
class FileExplorer;

class QmlCodeEditorWidget : public QWidget
{
    Q_OBJECT

public:
    enum DocumentType { Global, Internal, External };
    struct Document {
        DocumentType type;
        QTextCursor textCursor;
        QmlCodeDocument* document;
    };
    struct GlobalDocument : public Document { QString relativePath; };
    struct InternalDocument : public GlobalDocument { Control* control; };
    struct ExternalDocument : public Document { QString fullPath; };

public:
    explicit QmlCodeEditorWidget(QWidget *parent = nullptr);

    void openGlobal(const QString& relativePath);
    void openInternal(Control* control);
    void openExternal(const QString& fullPath);

    bool globalExists(const QString& relativePath);
    bool internalExists(Control* control, const QString& relativePath);
    bool externalExists(const QString& fullPath);

    GlobalDocument* getGlobal(const QString& relativePath);
    InternalDocument* getInternal(Control* control, const QString& relativePath);
    ExternalDocument* getExternal(const QString& fullPath);

    GlobalDocument* createGlobal(const QString& relativePath);
    InternalDocument* createInternal(Control* control, const QString& relativePath);
    ExternalDocument* createExternal(const QString& fullPath);

public slots:
    void sweep();
    void save();
    void close();
    void setFileExplorerVisible(bool visible);

private:
    bool documentExists(Document* document);
    void openDocument(Document* document);
    void newDocument(Document* document);

signals:
    void pinned(bool pinned);

private:
    QList<GlobalDocument*> m_globalDocuments;
    QList<InternalDocument*> m_internalDocuments;
    QList<ExternalDocument*> m_externalDocuments;
    Document* m_openDocument;
    QSplitter* m_splitter;
    QmlCodeEditor* m_codeEditor;
    FileExplorer* m_fileExplorer;
};

#endif // QMLCODEEDITORWIDGET_H