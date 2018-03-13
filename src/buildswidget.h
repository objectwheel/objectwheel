#ifndef BUILDSWINDOW_H
#define BUILDSWINDOW_H

#include <QWidget>
#include <global.h>

class View;
class AndroidWidget;
class DownloadWidget;
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
        AndroidWidget* _androidWidget;
        DownloadWidget* _downloadWidget;
};

#endif // BUILDSWINDOW_H
