#ifndef RUNCONTROLLER_H
#define RUNCONTROLLER_H

#include <QToolBar>

class RunDevicesButton;
class PushButton;
class RunProgressBar;

class RunController : public QToolBar
{
    Q_OBJECT

public:
    explicit RunController(QWidget *parent = nullptr);

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

#endif // RUNCONTROLLER_H