#ifndef RUNPANE_H
#define RUNPANE_H

#include <QToolBar>

class RunDevicesButton;
class PushButton;
class RunProgressBar;

class RunPane : public QToolBar
{
    Q_OBJECT

public:
    explicit RunPane(QWidget *parent = nullptr);

public slots:
    void discharge();

private slots:
    void onRunButtonClick();

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

signals:
    void runButtonClicked();
    void projectsButtonClicked();
    void preferencesButtonClicked();

private:
    PushButton* m_runButton;
    PushButton* m_stopButton;
    RunDevicesButton* m_runDevicesButton;
    PushButton* m_preferencesButton;
    PushButton* m_projectsButton;
    RunProgressBar* m_runProgressBar;
};

#endif // RUNPANE_H