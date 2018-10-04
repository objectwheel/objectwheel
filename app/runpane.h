#ifndef RUNPANE_H
#define RUNPANE_H

#include <QWidget>

class LoadingBar;
class QHBoxLayout;
class ConsoleBox;
class DevicesButton;
class RunPaneButton;

class RunPane : public QWidget
{
    Q_OBJECT

public:
    explicit RunPane(ConsoleBox* consoleBox, QWidget *parent = nullptr);

public slots:
    void sweep();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onRunButtonClick();
    void onStopButtonClick();
    void onStopButtonDoubleClick();
    void onProjectsButtonClick();

private:
    ConsoleBox* m_consoleBox;
    QHBoxLayout* m_layout;
    LoadingBar* m_loadingBar;
    RunPaneButton* m_runButton;
    RunPaneButton* m_stopButton;
    DevicesButton* m_devicesButton;
    RunPaneButton* m_projectsButton;
};

#endif // RUNPANE_H