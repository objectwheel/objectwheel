#ifndef BUILDSPANE_H
#define BUILDSPANE_H

#include <QWidget>

class View;
class AndroidPlatformWidget;
//class DownloadWidget;
class PlatformsWidget;
class ModuleSelectionWidget;

class BuildsPane final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildsPane)

public:
    explicit BuildsPane(QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QStackedWidget* m_stackedWidget;
    OTargets::Targets _target;
    PlatformsWidget* _platformsWidget;
    ModuleSelectionWidget* _modulesWidget;
    AndroidPlatformWidget* _androidWidget;
//    DownloadWidget* _downloadWidget;
};

#endif // BUILDSPANE_H
