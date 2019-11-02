#ifndef OUTPUTPANE_H
#define OUTPUTPANE_H

#include <QWidget>

class OutputBar;
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
    OutputBar* outputBar() const;
    QAbstractButton* issuesButton() const;
    QAbstractButton* consoleButton() const;
    IssuesWidget* issuesWidget() const;
    ConsoleWidget* consoleWidget() const;

    QWidget* widgetForButton(const QAbstractButton* button) const;
    QAbstractButton* buttonForWidget(const QWidget* widget) const;

private slots:
    void updateIcons();

private:
    void changeEvent(QEvent* event);

private:
    QStackedWidget* m_stackedWidget;
    OutputBar* m_outputBar;
    QAbstractButton* m_issuesButton;
    QAbstractButton* m_consoleButton;
    IssuesWidget* m_issuesWidget;
    ConsoleWidget* m_consoleWidget;
};

#endif // OUTPUTPANE_H