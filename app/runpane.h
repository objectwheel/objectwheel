#ifndef RUNPANE_H
#define RUNPANE_H

#include <QWidget>

class LoadingBar;
class FlatButton;
class QHBoxLayout;
class ConsoleBox;
class DevicesButton;

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
    FlatButton* m_runButton;
    FlatButton* m_stopButton;
    DevicesButton* m_devicesButton;
    FlatButton* m_projectsButton;
};

#endif // RUNPANE_H