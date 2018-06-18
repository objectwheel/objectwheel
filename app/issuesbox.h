#ifndef ISSUESBOX_H
#define ISSUESBOX_H

#include <flatbutton.h>

#include <QPointer>
#include <QMap>

class Control;
class OutputPane;
class QVBoxLayout;
class ToolBar;
class ToolButton;
class QLabel;
class QListWidget;
class QListWidgetItem;

class Error {
public:
    Error() {}
    ~Error() {}
    Error(const Error& other)
    {
        uid = other.uid;
        id = other.id;
        description = other.description;
        line = other.line;
        column = other.column;
    }
    bool operator== (const Error& e) const {
        return (e.uid == uid &&
                e.description == description &&
                e.column == column &&
                e.line == line);
    }
    int line, column;
    QString id, uid, description;
};
bool operator< (const Error&, const Error&);
Q_DECLARE_METATYPE(Error)

class IssuesBox : public QWidget
{
    Q_OBJECT
public:
    explicit IssuesBox(OutputPane* outputPane);

public slots:
    void sweep();
    void refresh();

private slots:
    void clear();
    void handleErrors(Control* control);
    void handleDoubleClick(QListWidgetItem* item);

signals:
    void entryDoubleClicked(Control*);

private:
    QVBoxLayout* m_layout;
    ToolBar* m_toolbar;
    ToolButton* m_clearButton;
    QLabel* m_title;
    QListWidget* m_listWidget;
    QMap<Error, QPointer<Control>> m_buggyControls;
    OutputPane* m_outputPane;
};

#endif // ISSUESBOX_H
