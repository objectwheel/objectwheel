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

    AssetsDocument* getAssets(const QString& relativePath) const;
    DesignsDocument* getDesigns(Control* control, const QString& relativePath) const;
    OthersDocument* getOthers(const QString& fullPath) const;

    AssetsDocument* addAssets(const QString& relativePath);
    DesignsDocument* addDesigns(Control* control, const QString& relativePath);
    OthersDocument* addOthers(const QString& fullPath);

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
    void onNewOthersFile();
    void onOpenOthersFile();
    void onModificationChange(Document* document);
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
    QList<AssetsDocument*> m_assetsDocuments;
    QList<DesignsDocument*> m_designsDocuments;
    QList<OthersDocument*> m_OthersDocuments;
};

Q_DECLARE_METATYPE(QmlCodeEditorWidget::Document*)
Q_DECLARE_METATYPE(QmlCodeEditorWidget::AssetsDocument*)
Q_DECLARE_METATYPE(QmlCodeEditorWidget::DesignsDocument*)
Q_DECLARE_METATYPE(QmlCodeEditorWidget::OthersDocument*)

#endif // QMLCODEEDITORWIDGET_H