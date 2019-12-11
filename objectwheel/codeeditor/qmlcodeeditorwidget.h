#ifndef QMLCODEEDITORWIDGET_H
#define QMLCODEEDITORWIDGET_H

#include <QSplitter>
#include <QTextCursor>

#include <qmlcodeeditortoolbar.h>

class Control;
class QmlCodeDocument;
class QSplitter;
class QmlCodeEditor;
class FileExplorer;

class QmlCodeEditorWidget final : public QSplitter
{
    Q_OBJECT
    Q_DISABLE_COPY(QmlCodeEditorWidget)

public:
    struct Document {
        QTextCursor textCursor;
        QmlCodeEditorToolBar::Scope scope;
        QmlCodeDocument* document;
    };
    struct AssetsDocument : Document { QString relativePath; };
    struct DesignsDocument : AssetsDocument { Control* control; };
    struct OthersDocument : Document { QString fullPath; };

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

    void openAssets(const QString& relativePath);
    void openDesigns(Control* control, const QString& relativePath);
    void openOthers(const QString& fullPath);

    bool assetsExists(const QString& relativePath) const;
    bool designsExists(Control* control, const QString& relativePath) const;
    bool othersExists(const QString& fullPath) const;

    void closeDesigns(Control* control);

    AssetsDocument* getAssets(const QString& relativePath) const;
    DesignsDocument* getDesigns(Control* control, const QString& relativePath) const;
    OthersDocument* getOthers(const QString& fullPath) const;

    AssetsDocument* addAssets(const QString& relativePath);
    DesignsDocument* addDesigns(Control* control, const QString& relativePath);
    OthersDocument* addOthers(const QString& fullPath);

    QmlCodeEditor* codeEditor() const;
    QmlCodeEditorToolBar* toolBar() const;
    QSize sizeHint() const override;

    void addSaveFilter(SaveFilter* sf) { m_saveFilters.append(sf); }

public slots:
    void discharge();
    void saveAll();
    void saveOpen();
    void close();
    void close(Document* document);
    void save(Document* document);
    void rename(Document* document, const QString& newPath);
    void setFileExplorerVisible(bool visible);
    void setupLastOpenedDocs(QmlCodeEditorWidget::Document* document);
    void onAssetsFileExplorerFilesAboutToBeDeleted(const QSet<QString>& pathes);
    void onAssetsFileExplorerFileRenamed(const QString& path, const QString& oldName, const QString& newName);

private slots:
    void onNewOthersFile();
    void onAddOthersFile();
    void onModificationChange(Document* document);
    void onPinActivation(bool pinned);
    void onScopeActivation(QmlCodeEditorToolBar::Scope);
    void onComboActivation(QmlCodeEditorToolBar::Combo);
    void onFileExplorerFileOpen(const QString& relativePath);
    void onFileExplorerFilesAboutToBeDeleted(const QSet<QString>& pathes);
    void onFileExplorerFileRenamed(const QString& path, const QString& oldName, const QString& newName);

protected:
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dragMoveEvent(QDragMoveEvent* e) override;
    void dragLeaveEvent(QDragLeaveEvent* e) override;
    void dropEvent(QDropEvent* e) override;
    void closeEvent(QCloseEvent* e) override;

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
    bool m_fileExplorerHid;

    QmlCodeEditor* m_codeEditor;
    FileExplorer* m_fileExplorer;

    Document* m_openDocument;
    QmlCodeEditorWidget::AssetsDocument* m_lastAssetsDocument;
    QmlCodeEditorWidget::DesignsDocument* m_lastDesignsDocument;
    QmlCodeEditorWidget::OthersDocument* m_lastOthersDocument;

    QList<SaveFilter*> m_saveFilters;
    QList<AssetsDocument*> m_assetsDocuments;
    QList<DesignsDocument*> m_designsDocuments;
    QList<OthersDocument*> m_othersDocuments;
};

Q_DECLARE_METATYPE(QmlCodeEditorWidget::Document*)
Q_DECLARE_METATYPE(QmlCodeEditorWidget::AssetsDocument*)
Q_DECLARE_METATYPE(QmlCodeEditorWidget::DesignsDocument*)
Q_DECLARE_METATYPE(QmlCodeEditorWidget::OthersDocument*)

#endif // QMLCODEEDITORWIDGET_H