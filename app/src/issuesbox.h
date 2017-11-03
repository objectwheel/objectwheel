#ifndef ISSUESBOX_H
#define ISSUESBOX_H

#include <flatbutton.h>
#include <global.h>
#include <QWidget>
#include <QListWidget>
#include <QToolBar>
#include <QVBoxLayout>
#include <QTimer>
#include <QPointer>
#include <QLabel>

class Control;
class OutputWidget;

class Error {
    public:
        Error() {}
        ~Error() {}
        Error(const Error& other)
        {
            uid = other.uid;
            id = other.id;
            mode = other.mode;
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
        DesignMode mode;
        int line, column;
        QString id, uid, description;
};
bool operator< (const Error&, const Error&);
Q_DECLARE_METATYPE(Error)

class IssuesBox : public QWidget
{
        Q_OBJECT
    public:
        explicit IssuesBox(OutputWidget* outputWidget);
        void setCurrentMode(const DesignMode& currentMode);

    public slots:
        void refresh();

    private slots:
        void handleErrors(Control* control);
        void handleDoubleClick(QListWidgetItem* item);

    signals:
        void entryDoubleClicked(Control*);

    private:
        QVBoxLayout _layout;
        QToolBar _toolbar;
        QLabel _title;
        QListWidget _listWidget;
        QMap<Error, QPointer<Control>> _buggyControls;
        OutputWidget* _outputWidget;
        DesignMode _currentMode;
};

#endif // ISSUESBOX_H
