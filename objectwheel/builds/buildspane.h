#ifndef BUILDSPANE_H
#define BUILDSPANE_H

#include <QScrollArea>

class QLabel;
class SegmentedBar;
class QStackedWidget;
class StartWidget;
class AndroidPlatformWidget;
class AndroidPlatformController;

class BuildsPane final : public QScrollArea
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildsPane)

public:
    explicit BuildsPane(QWidget* parent = nullptr);

    QLabel* platformLabel() const;
    SegmentedBar* segmentedBar() const;
    QStackedWidget* stackedWidget() const;
    StartWidget* startWidget() const;
    AndroidPlatformWidget* androidPlatformWidget() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void charge() const;
    void discharge() const;

private:
    QLabel* m_platformLabel;
    SegmentedBar* m_segmentedBar;
    QStackedWidget* m_stackedWidget;
    StartWidget* m_startWidget;
    AndroidPlatformWidget* m_androidPlatformWidget;
    AndroidPlatformController* m_androidPlatformController;
};

#endif // BUILDSPANE_H
