#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QtWidgets>

#include <flatbutton.h>
#include <toolboxtree.h>
#include <fit.h>
#include <filterlineedit.h>
#include <toolboxsettings.h>

class ToolBox : public QWidget
{
        Q_OBJECT
    public:
        explicit ToolBox(QWidget *parent = nullptr);
        ToolboxTree* toolboxTree();

    public slots:
        void showSettings();

    protected:
        virtual QSize sizeHint() const override;

    private slots:
        void filterList(const QString& filter);
        void handleMousePress(QTreeWidgetItem* item);

    private:
        QVBoxLayout _toolboxVLay;
        FilterLineEdit _searchEdit;
        ToolboxTree _toolboxTree;
        ToolboxSettings _settingsDialog;

};

#endif // TOOLBOX_H
