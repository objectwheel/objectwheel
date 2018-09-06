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
        QTextCursor cursor;
        QmlCodeDocument* document;
    };
    struct ExternalDocument : public Document { QString path; };
    struct InternalDocument : public Document { Control* control; };

public:
    explicit QmlCodeEditorWidget(QWidget *parent = nullptr);

public slots:
    void sweep();
    void save();
    void close();
    void setExplorerVisible(bool visible);

signals:
    void pinned(bool pinned);

private:
    QList<ExternalDocument*> m_regularDocuments;
    QList<ExternalDocument*> m_globalDocuments;
    QList<InternalDocument*> m_internalDocuments;
    QSplitter* m_splitter;
    QmlCodeEditor* m_codeEditor;
    FileExplorer* m_fileExplorer;
};

#endif // QMLCODEEDITORWIDGET_H