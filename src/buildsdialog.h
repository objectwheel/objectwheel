#ifndef BUILDSDIALOG_H
#define BUILDSDIALOG_H

#include <view.h>
#include <platformswidget.h>
#include <androidwidget.h>
#include <moduleselectionwidget.h>
#include <QDialog>

class BuildsDialog : public QDialog
{
        Q_OBJECT
    public:
        explicit BuildsDialog(QWidget *parent = nullptr);

    protected:
        virtual void resizeEvent(QResizeEvent *event) override;

    public slots:
        void showPlatforms();
        void handlePlatformSelection(Targets);

    private:
        View _view;
        PlatformsWidget _platformsWidget;
        ModuleSelectionWidget _modulesWidget;
        AndroidWidget _androidWidget;
};

#endif // BUILDSDIALOG_H
