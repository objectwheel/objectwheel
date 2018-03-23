#ifndef MODULESELECTIONWIDGET_H
#define MODULESELECTIONWIDGET_H

#include <QtWidgets>
#include <flatbutton.h>

class ModuleSelectionWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit ModuleSelectionWidget(QWidget *parent = nullptr);

    private slots:
        void handleBtnNextClicked();

    signals:
        void done();
        void backClicked();
};

#endif // MODULESELECTIONWIDGET_H
