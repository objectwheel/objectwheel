#ifndef RUNPANE_H
#define RUNPANE_H

#include <QToolBar>

class PushButton;
class RunProgressBar;
class RunDevicesButton;

class RunPane final : public QToolBar
{
    Q_OBJECT
    Q_DISABLE_COPY(RunPane)

public:
    explicit RunPane(QWidget* parent = nullptr);

    PushButton* runButton() const;
    PushButton* stopButton() const;
    PushButton* projectsButton() const;
    PushButton* preferencesButton() const;
    RunProgressBar* runProgressBar() const;
    RunDevicesButton* runDevicesButton() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    bool event(QEvent* event) override;
    void paintEvent(QPaintEvent*) override;

private:
    PushButton* m_runButton;
    PushButton* m_stopButton;
    PushButton* m_projectsButton;
    PushButton* m_preferencesButton;
    RunProgressBar* m_runProgressBar;
    RunDevicesButton* m_runDevicesButton;
};

#endif // RUNPANE_H