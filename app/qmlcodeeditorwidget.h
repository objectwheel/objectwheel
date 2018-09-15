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

public:
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
    void onModificationChange();
    void onScopeActivation(QmlCodeEditorToolBar::Scope);
    void onComboActivation(QmlCodeEditorToolBar::Combo);
    void onFileExplorerFileOpen(const QString& relativePath);

private:
    QmlCodeEditorToolBar* toolBar() const;
    bool documentExists(Document* document) const;

    void showNoDocumentsOpen();
    void openDocument(Document* document);
    void setupToolBar(Document* document);
    void setupCodeEditor(Document* document);
    void setupFileExplorer(Document* document);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    QSize sizeHint() const override;

signals:
    void opened();
    void pinned(bool pinned);

private:
    Document* m_openDocument;
    QSplitter* m_splitter;
    QmlCodeEditor* m_codeEditor;
    FileExplorer* m_fileExplorer;
    QList<GlobalDocument*> m_globalDocuments;
    QList<InternalDocument*> m_internalDocuments;
    QList<ExternalDocument*> m_externalDocuments;
};

Q_DECLARE_METATYPE(QmlCodeEditorWidget::Document*)
Q_DECLARE_METATYPE(QmlCodeEditorWidget::GlobalDocument*)
Q_DECLARE_METATYPE(QmlCodeEditorWidget::InternalDocument*)
Q_DECLARE_METATYPE(QmlCodeEditorWidget::ExternalDocument*)

#endif // QMLCODEEDITORWIDGET_H