#ifndef QMLEDITORVIEW_H
#define QMLEDITORVIEW_H

#include <QWidget>
#include <QTextDocument>

class QmlEditorViewPrivate;
class Control;

class QmlEditorView : public QWidget
{
        Q_OBJECT
        friend class QmlEditorViewPrivate;

    public:
        struct EditorItem {
                Control* control;
                QString currentFileRelativePath;
                QMap<QString /* path */, QTextDocument* /* document */> documents;
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

        void addControl(Control* control);
        void addDocument(Control* control, const QString& documentPath);
        void setCurrentDocument(Control* control, const QString& documentPath);
        void openControl(Control* control);

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
