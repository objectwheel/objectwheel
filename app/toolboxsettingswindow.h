#ifndef TOOLBOXSETTINGSWINDOW_H
#define TOOLBOXSETTINGSWINDOW_H

#include <QWidget>

namespace Ui {
class ToolboxSettingsWindow;
}

class ToolboxSettingsWindow : public QWidget
{
    Q_OBJECT

    friend class WindowManager;

public:
    explicit ToolboxSettingsWindow(QWidget *parent = 0);
    ~ToolboxSettingsWindow();

private slots:
    void on_btnReset_clicked();
    void on_btnRemove_clicked();
    void on_btnAdd_clicked();
    void on_btnImport_clicked();
    void on_btnExport_clicked();
    void on_btnFileDialog_clicked();
    void on_btnSave_clicked();

signals:
    void done();

private:
    QSize sizeHint() const override;

private:
    Ui::ToolboxSettingsWindow* ui;
};

#endif // TOOLBOXSETTINGSWINDOW_H
