#ifndef QMLCODEEDITORWIDGET_H
#define QMLCODEEDITORWIDGET_H

#include <QWidget>
#include <qmlcodedocument.h>
#include <QTextCursor>

class QmlCodeEditorWidgetPrivate;
class Control;
class QmlCodeEditor;

struct DocumentData {
    QmlCodeDocument* document;
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

class QmlCodeEditorWidget : public QWidget
{
    Q_OBJECT
    friend class QmlCodeEditorWidgetPrivate;

public:
    enum Mode {
        CodeEditor,
        ImageEditor,
        HexEditor
    };

public:
    explicit QmlCodeEditorWidget(QWidget *parent = 0);
    Mode mode() const;
    int openControlCount() const;

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
    QmlCodeEditor* editor() const;

public slots:
    void reset();
    void saveAll();
    void raiseContainer();
    void refreshErrors();
    void handleControlRemoval(Control* control);

protected:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void modeChanged();
    void documentSaved();
    void openControlCountChanged();

private:
    QmlCodeEditorWidgetPrivate* _d;
    Mode _mode;
    QList<EditorItem> _editorItems;
};

#endif // QMLCODEEDITORWIDGET_H
