#ifndef OUTPUTPANE_H
#define OUTPUTPANE_H

#include <QWidget>

class OutputBar;
class IssuesWidget;
class ConsoleWidget;

class OutputPane final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputPane)

public:
    explicit OutputPane(QWidget* parent = nullptr);

    OutputBar* outputBar() const;
    IssuesWidget* issuesWidget() const;
    ConsoleWidget* consoleWidget() const;

private:
    OutputBar* m_outputBar;
    IssuesWidget* m_issuesWidget;
    ConsoleWidget* m_consoleWidget;
};

#endif // OUTPUTPANE_H