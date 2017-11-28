#ifndef BUILDSDIALOG_H
#define BUILDSDIALOG_H

#include <view.h>
#include <platformswidget.h>
#include <QDialog>

class BuildsDialog : public QDialog
{
        Q_OBJECT
    public:
        explicit BuildsDialog(QWidget *parent = nullptr);

    protected:
        virtual void resizeEvent(QResizeEvent *event) override;

    private:
        View _view;
        PlatformsWidget _platformsWidget;
};

#endif // BUILDSDIALOG_H
