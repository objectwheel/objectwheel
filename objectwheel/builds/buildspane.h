#ifndef BUILDSPANE_H
#define BUILDSPANE_H

#include <QWidget>

class QLabel;
class SegmentedBar;
class QStackedWidget;
class GenericPlatformWidget;
class AndroidPlatformWidget;

class BuildsPane final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildsPane)

public:
    explicit BuildsPane(QWidget* parent = nullptr);

    QLabel* platformLabel() const;
    SegmentedBar* segmentedBar() const;
    QStackedWidget* stackedWidget() const;
    GenericPlatformWidget* genericWidget() const;
    AndroidPlatformWidget* androidWidget() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QLabel* m_platformLabel;
    SegmentedBar* m_segmentedBar;
    QStackedWidget* m_stackedWidget;
    GenericPlatformWidget* m_genericWidget;
    AndroidPlatformWidget* m_androidWidget;
};

#endif // BUILDSPANE_H
