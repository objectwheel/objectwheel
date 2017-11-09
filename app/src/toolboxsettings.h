#ifndef TOOLBOXSETTINGS_H
#define TOOLBOXSETTINGS_H

#include <QDialog>

namespace Ui {
    class ToolboxSettings;
}

class ToolboxSettings : public QDialog
{
        Q_OBJECT

    public:
        explicit ToolboxSettings(QWidget *parent = 0);
        ~ToolboxSettings();

    private:
        Ui::ToolboxSettings* ui;
};

#endif // TOOLBOXSETTINGS_H
