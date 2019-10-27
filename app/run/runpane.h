#ifndef RUNPANE_H
#define RUNPANE_H

#include <QToolBar>

class QPushButton;
class RunProgressBar;
class RunDevicesButton;
class SegmentedBar;

class RunPane final : public QToolBar
{
    Q_OBJECT
    Q_DISABLE_COPY(RunPane)

public:
    explicit RunPane(QWidget* parent = nullptr);

    QPushButton* runButton() const;
    QPushButton* stopButton() const;
    QPushButton* projectsButton() const;
    QPushButton* preferencesButton() const;
    SegmentedBar* segmentedBar() const;
    RunProgressBar* runProgressBar() const;
    RunDevicesButton* runDevicesButton() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void updateIcons();
    void changeEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QPushButton* m_runButton;
    QPushButton* m_stopButton;
    QPushButton* m_projectsButton;
    QPushButton* m_preferencesButton;
    SegmentedBar* m_segmentedBar;
    RunProgressBar* m_runProgressBar;
    RunDevicesButton* m_runDevicesButton;
};

#endif // RUNPANE_H