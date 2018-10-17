#ifndef RUNPANE_H
#define RUNPANE_H

#include <QToolBar>

class DevicesButton;
class PushButton;
class RunPaneLoadingBar;

class RunPane : public QToolBar
{
    Q_OBJECT

public:
    explicit RunPane(QWidget *parent = nullptr);

public slots:
    void sweep();
    void busy(int progress, const QString& message);
    void done(const QString& message);
    void error(const QString& message);
    void setMessage(const QString& message);

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
    DevicesButton* m_devicesButton;
    PushButton* m_preferencesButton;
    PushButton* m_projectsButton;
    RunPaneLoadingBar* m_loadingBar;
};

#endif // RUNPANE_H