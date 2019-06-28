#ifndef OUTPUTPANE_H
#define OUTPUTPANE_H

#include <QWidget>

class QStackedWidget;
class IssuesWidget;
class ConsoleWidget;
class QAbstractButton;

class OutputPane final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputPane)

public:
    explicit OutputPane(QWidget* parent = nullptr);

    QStackedWidget* stackedWidget() const;
    QAbstractButton* issuesButton() const;
    QAbstractButton* consoleButton() const;
    IssuesWidget* issuesWidget() const;
    ConsoleWidget* consoleWidget() const;
    QWidget* widgetForButton(QAbstractButton* button) const;

private:
    QStackedWidget* m_stackedWidget;
    QAbstractButton* m_issuesButton;
    QAbstractButton* m_consoleButton;
    IssuesWidget* m_issuesWidget;
    ConsoleWidget* m_consoleWidget;
};

#endif // OUTPUTPANE_H