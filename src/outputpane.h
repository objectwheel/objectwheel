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
class OutputPanePrivate;

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

    public slots:
        void clear();
        void expand();
        void collapse();
        void updateLastHeight();
        void shine(Box type);

    protected:
        QSize sizeHint() const override;

    private:
        QPointer<QSplitter> _splitter;
        QPointer<QSplitterHandle> _splitterHandle;
        OutputPanePrivate* _d;
        IssuesBox* _issuesBox;
        ConsoleBox* _consoleBox;
        SearchBox* _searchBox;
        int _lastHeight;
        QWidget* _activeBox;
        bool _collapsed;
};

#endif // OUTPUTPANE_H
