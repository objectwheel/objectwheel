#ifndef ISSUESBOX_H
#define ISSUESBOX_H

#include <flatbutton.h>
#include <QWidget>
#include <QListWidget>
#include <QToolBar>
#include <QVBoxLayout>
#include <QTimer>
#include <QPointer>
#include <QLabel>
#include <QToolButton>

class Control;
class OutputPane;

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
        void clear();
        void refresh();

    private slots:
        void handleErrors(Control* control);
        void handleDoubleClick(QListWidgetItem* item);

    signals:
        void entryDoubleClicked(Control*);

    private:
        QVBoxLayout* m_layout;
        QToolBar* m_toolbar;
        QToolButton* m_clearButton;
        QLabel* m_title;
        QListWidget* m_listWidget;
        QMap<Error, QPointer<Control>> m_buggyControls;
        OutputPane* m_outputPane;
};

#endif // ISSUESBOX_H
