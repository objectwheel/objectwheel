#ifndef OUTPUTPANE_H
#define OUTPUTPANE_H

#include <QWidget>
#include <QPointer>

class QSplitter;
class IssuesBox;
class SearchBox;
class ConsoleBox;
class FlatButton;
class QSplitterHandle;
class QVBoxLayout;
class QToolBar;

class OutputPane : public QWidget
{
    Q_OBJECT
    friend class OutputPanePrivate;

public:
    enum Box {
        Issues,
        Search,
        Console
    };

public:
    explicit OutputPane(QWidget *parent = nullptr);
    void setSplitter(QSplitter* splitter);
    void setSplitterHandle(QSplitterHandle* splitterHandle);

    FlatButton* button(Box type);
    IssuesBox* issuesBox();
    ConsoleBox* consoleBox();
    SearchBox* searchBox();
    Box activeBox() const;
    bool isCollapsed() const;

public slots:
    void sweep();
    void expand();
    void collapse();
    void updateLastHeight();
    void shine(Box type);

private slots:
    void handleHideButtonClicked();
    void handleIssuesButtonClicked(bool val);
    void handleSearchButtonClicked(bool val);
    void handleConsoleButtonClicked(bool val);

protected:
    QSize sizeHint() const override;

private:
    QVBoxLayout* m_layout;
    QToolBar* m_toolbar;
    FlatButton* m_hideButton;
    FlatButton* m_issuesButton;
    FlatButton* m_searchButton;
    FlatButton* m_consoleButton;
    QPointer<QSplitter> m_splitter;
    QPointer<QSplitterHandle> m_splitterHandle;
    IssuesBox* m_issuesBox;
    ConsoleBox* m_consoleBox;
    SearchBox* m_searchBox;
    int m_lastHeight;
    QWidget* m_activeBox;
    bool m_collapsed;
};

#endif // OUTPUTPANE_H
