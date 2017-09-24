#ifndef QMLEDITORVIEW_H
#define QMLEDITORVIEW_H

#include <QWidget>
#include <QTextDocument>
#include <QTextCursor>

class QmlEditorViewPrivate;
class Control;

class QmlEditorView : public QWidget
{
        Q_OBJECT
        friend class QmlEditorViewPrivate;

    public:
        struct DocumentData {
                QTextDocument* document;
                QTextCursor cursor;
                QMetaObject::Connection modificationConnection;
        };

        struct EditorItem {
                Control* control;
                QString currentFileRelativePath;
                QMap<QString /* path */, DocumentData /* documentData */> documents;
        };

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
        void addControl(Control* control);
        void addDocument(Control* control, const QString& documentPath);
        void setCurrentDocument(Control* control, const QString& documentPath);
        void openControl(Control* control);

    public slots:
        void raiseContainer();

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
