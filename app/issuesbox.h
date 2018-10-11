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

class IssuesBox : public QWidget
{
    Q_OBJECT
public:
    explicit IssuesBox(QWidget* parent = nullptr);

public slots:
    void sweep();
    void refresh();
    void process(Control* control);

private slots:
    void clear();
    void onItemDoubleClick(QListWidgetItem* item);

protected:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void flash();
    void minimized();
    void titleChanged(const QString& title);
    void itemDoubleClicked(Control*);

private:
    QVBoxLayout* m_layout;
    QListWidget* m_listWidget;
    QMap<Error, QPointer<Control>> m_defectiveControls;
    QToolBar* m_toolBar;
    QLabel* m_titleLabel;
    QToolButton* m_clearButton;
    QToolButton* m_fontSizeUpButton;
    QToolButton* m_fontSizeDownButton;
    QToolButton* m_minimizeButton;
};

#endif // ISSUESBOX_H
