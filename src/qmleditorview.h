#ifndef QMLEDITORVIEW_H
#define QMLEDITORVIEW_H

#include <QWidget>
#include <QTextDocument>
#include <QTextCursor>

class QmlEditorViewPrivate;
class Control;

struct DocumentData {
        QTextDocument* document;
        QTextCursor cursor;
        QMetaObject::Connection modificationConnection;
};

struct EditorItem {
        Control* control;
        QString currentFileRelativePath;
        QMap<QString, DocumentData> documents;
        bool operator ==(const EditorItem& item) const {
            return item.control == this->control;
        }
};

class QmlEditorView : public QWidget
{
        Q_OBJECT
        friend class QmlEditorViewPrivate;

    public:
        enum Mode {
            CodeEditor,
            ImageEditor,
            HexEditor
        };

    public:
        explicit QmlEditorView(QWidget *parent = 0);
        Mode mode() const;
        void setMode(const Mode& mode);
        bool pinned() const;
        bool isOpen(Control* control) const;
        bool isOpen(const QString& controlPath) const;
        bool hasUnsavedDocs() const;
        bool hasChanges(Control* control) const;
        void addControl(Control* control);
        void addDocument(Control* control, const QString& documentPath);
        void setCurrentDocument(Control* control, const QString& documentPath);
        void openControl(Control* control);
        void closeControl(Control* control, const bool ask = true);
        void closeDocument(Control* control, const QString& documentPath, const bool ask = true);
        void saveControl(Control* control);
        void saveDocument(Control* control, const QString& documentPath);

    public slots:
        void clear();
        void saveAll();
        void raiseContainer();
        void refreshErrors();

    protected:
        virtual void paintEvent(QPaintEvent *event) override;

    signals:
        void modeChanged();

    private:
        QmlEditorViewPrivate* _d;
        Mode _mode;
        QList<EditorItem> _editorItems;
};

#endif // QMLEDITORVIEW_H
