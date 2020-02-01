#ifndef BUILDSWINDOW_H
#define BUILDSWINDOW_H

#include <QWidget>
#include <global.h>

class View;
class AndroidPlatformWidget;
//class DownloadWidget;
class PlatformsWidget;
class ModuleSelectionWidget;

class BuildsWidget : public QWidget
{
    Q_OBJECT

public:
    enum Screens {
        Platforms,
        Modules,
        Android,
        Download
    };

public:
    explicit BuildsWidget(QWidget *parent = nullptr);

protected:
    QSize sizeHint() const override;

private slots:
    void showModules();
    void showPlatforms();
    void handleDownload();
    void handleModuleSelection();
    void handlePlatformSelection(OTargets::Targets);

private:
    View* _view;
    OTargets::Targets _target;
    PlatformsWidget* _platformsWidget;
    ModuleSelectionWidget* _modulesWidget;
    AndroidPlatformWidget* _androidWidget;
//    DownloadWidget* _downloadWidget;
};

#endif // BUILDSWINDOW_H
