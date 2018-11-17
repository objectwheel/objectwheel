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
    struct GlobalDocument : Document { QString relativePath; };
    struct InternalDocument : GlobalDocument { Control* control; };
    struct ExternalDocument : Document { QString fullPath; };

    class SaveFilter {
    public:
        ~SaveFilter() {}
        virtual void beforeSave(Document*) = 0;
        virtual void afterSave(Document*) = 0;
    };

public:
    explicit QmlCodeEditorWidget(QWidget* parent = nullptr);

    int count() const;

    bool controlExists(const Control* control);
    bool controlModified(const Control* control);

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

    QmlCodeEditor* codeEditor() const;
    QmlCodeEditorToolBar* toolBar() const;

    void addSaveFilter(SaveFilter* sf) { m_saveFilters.append(sf); }

public slots:
    void discharge();
    void saveAll();
    void saveOpen();
    void save(Document* document);
    void close();
    void setFileExplorerVisible(bool visible);

private slots:
    void onNewExternalFile();
    void onOpenExternalFile();
    void onModificationChange();
    void onPinActivation(bool pinned);
    void onScopeActivation(QmlCodeEditorToolBar::Scope);
    void onComboActivation(QmlCodeEditorToolBar::Combo);
    void onFileExplorerFileOpen(const QString& relativePath);

protected:
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dragMoveEvent(QDragMoveEvent* e) override;
    void dragLeaveEvent(QDragLeaveEvent* e) override;
    void dropEvent(QDropEvent* e) override;
    void closeEvent(QCloseEvent* e) override;
    QSize sizeHint() const override;

private:
    void showNoDocumentsOpen();
    void openDocument(Document* document);
    void setupToolBar(Document* document);
    void setupCodeEditor(Document* document);
    void setupFileExplorer(Document* document);
    bool documentExists(Document* document) const;

signals:
    void opened();

private:
    QSplitter* m_splitter;
    QmlCodeEditor* m_codeEditor;
    FileExplorer* m_fileExplorer;

    Document* m_openDocument;
    QList<SaveFilter*> m_saveFilters;
    QList<GlobalDocument*> m_globalDocuments;
    QList<InternalDocument*> m_internalDocuments;
    QList<ExternalDocument*> m_externalDocuments;
};

Q_DECLARE_METATYPE(QmlCodeEditorWidget::Document*)
Q_DECLARE_METATYPE(QmlCodeEditorWidget::GlobalDocument*)
Q_DECLARE_METATYPE(QmlCodeEditorWidget::InternalDocument*)
Q_DECLARE_METATYPE(QmlCodeEditorWidget::ExternalDocument*)

#endif // QMLCODEEDITORWIDGET_H