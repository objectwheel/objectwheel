#ifndef MODULESELECTIONWIDGET_H
#define MODULESELECTIONWIDGET_H

#include <QtWidgets>
#include <flatbutton.h>

class ModuleSelectionWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit ModuleSelectionWidget(QWidget *parent = nullptr);

    signals:
        void backClicked();
};

#endif // MODULESELECTIONWIDGET_H
