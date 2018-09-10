#ifndef QMLCODEEDITORWIDGET_H
#define QMLCODEEDITORWIDGET_H

#include <QWidget>
#include <QTextCursor>

#include <qmlcodeeditortoolbar.h>

class Control;
class QmlCodeDocument;
class QSplitter;
class QmlCodeEditor;
class FileExplorer;

class QmlCodeEditorWidget : public QWidget
{
    Q_OBJECT

    struct Document {
        QTextCursor textCursor;
        QmlCodeDocument* document;
        QmlCodeEditorToolBar::Scope scope;
    };
    struct GlobalDocument : public Document { QString relativePath; };
    struct InternalDocument : public GlobalDocument { Control* control; };
    struct ExternalDocument : public Document { QString fullPath; };

public:
    explicit QmlCodeEditorWidget(QWidget *parent = nullptr);

    int count() const;

    void openGlobal(const QString& relativePath);
    void openInternal(Control* control, const QString& relativePath);
    void openExternal(const QString& fullPath);

    bool globalExists(const QString& relativePath) const;
    bool internalExists(Control* control, const QString& relativePath) const;
    bool externalExists(const QString& fullPath) const;

    GlobalDocument* getGlobal(const QString& relativePath) const;
    InternalDocument* getInternal(Control* control, const QString& relativePath) const;
    ExternalDocument* getExternal(const QString& fullPath) const;

    GlobalDocument* addGlobal(const QString& relativePath);
    InternalDocument* addInternal(Control* control, const QString& relativePath);
    ExternalDocument* addExternal(const QString& fullPath);

public slots:
    void sweep();
    void save();
    void close();
    void setFileExplorerVisible(bool visible);

private slots:
    void onScopeActivation(QmlCodeEditorToolBar::Scope);
    void onFileExplorerFileOpen(const QString& relativePath);

private:
    bool warnIfNotATextFile(const QString& filePath) const;
    void activateGlobalScope();
    void activateInternalScope();
    void activateExternalScope();
    void openDocument(Document* document);
    void setupToolBar(Document* document);
    void setupCodeEditor(Document* document);
    void setupFileExplorer(Document* document);

    QSize sizeHint() const override;
    QmlCodeEditorToolBar* toolBar() const;

signals:
    void opened();
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