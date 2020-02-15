#ifndef BUILDSPANE_H
#define BUILDSPANE_H

#include <QScrollArea>

class StackedLayout;
class DownloadWidget;
class PlatformSelectionWidget;
class AndroidPlatformWidget;

class BuildsPane final : public QScrollArea
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildsPane)

public:
    explicit BuildsPane(QWidget* parent = nullptr);

    StackedLayout* stackedLayout() const;
    DownloadWidget* downloadWidget() const;
    PlatformSelectionWidget* platformSelectionWidget() const;
    AndroidPlatformWidget* androidPlatformWidget() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    StackedLayout* m_stackedLayout;
    DownloadWidget* m_downloadWidget;
    PlatformSelectionWidget* m_platformSelectionWidget;
    AndroidPlatformWidget* m_androidPlatformWidget;
};

#endif // BUILDSPANE_H
