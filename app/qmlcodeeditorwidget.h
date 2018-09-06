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
    enum DocumentType { Internal, Global, Regular };
    struct Document {
        DocumentType type;
        QTextCursor cursor;
        QmlCodeDocument* document;
    };
    struct ExternalDocument : public Document { QString path; };
    struct InternalDocument : public Document { Control* control; };

public slots:
    void sweep();

public:
    explicit QmlCodeEditorWidget(QWidget *parent = nullptr);
    void openDocument(Control* control);
    void openDocument(const QString& path, DocumentType type);

private:
    void openDocument(Document* document) const;
    Document* createNewDocument(Control* control) const;
    Document* createNewDocument(const QString& path, DocumentType type) const;

private:
    QList<ExternalDocument*> m_regularDocuments;
    QList<ExternalDocument*> m_globalDocuments;
    QList<InternalDocument*> m_internalDocuments;
    QSplitter* m_splitter;
    QmlCodeEditor* m_codeEditor;
    FileExplorer* m_fileExplorer;
};

#endif // QMLCODEEDITORWIDGET_H