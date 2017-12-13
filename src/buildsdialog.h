#ifndef BUILDSDIALOG_H
#define BUILDSDIALOG_H

#include <view.h>
#include <platformswidget.h>
#include <androidwidget.h>
#include <moduleselectionwidget.h>
#include <downloadwidget.h>
#include <QDialog>

class BuildsDialog : public QDialog
{
        Q_OBJECT
    public:
        explicit BuildsDialog(QWidget *parent = nullptr);

    protected:
        virtual void resizeEvent(QResizeEvent *event) override;

    public slots:
        void showModules();
        void showPlatforms();
        void handleModuleSelection();
        void handlePlatformSelection(OTargets::Targets);
        void handleDownload();

    private:
        View _view;
        OTargets::Targets _target;
        PlatformsWidget _platformsWidget;
        ModuleSelectionWidget _modulesWidget;
        AndroidWidget _androidWidget;
        DownloadWidget _downloadWidget;
};

#endif // BUILDSDIALOG_H
