#ifndef ISSUESBOX_H
#define ISSUESBOX_H

#include <flatbutton.h>

#include <QPointer>
#include <QMap>

class Control;
class QVBoxLayout;
class QToolBar;
class QToolButton;
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
    explicit IssuesBox(QWidget* parent = nullptr);

public slots:
    void sweep();
    void refresh();
    void handleErrors(Control* control);

private slots:
    void clear();
    void handleDoubleClick(QListWidgetItem* item);

protected:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void flashMe();
    void updateTitle(const QString& title);
    void entryDoubleClicked(Control*);

private:
    QVBoxLayout* m_layout;
    QToolBar* m_toolBar;
    QToolButton* m_clearButton;
    QLabel* m_title;
    QListWidget* m_listWidget;
    QMap<Error, QPointer<Control>> m_buggyControls;
};

#endif // ISSUESBOX_H
