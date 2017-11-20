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

    private slots:
        void on_btnReset_clicked();
        void on_btnRemove_clicked();
        void on_btnAdd_clicked();
        void on_btnImport_clicked();
        void on_btnExport_clicked();
        void on_btnFileDialog_clicked();

        void on_btnSave_clicked();

    private:
        Ui::ToolboxSettings* ui;
};

#endif // TOOLBOXSETTINGS_H
