#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QtWidgets>

#include <flatbutton.h>
#include <toolboxtree.h>
#include <fit.h>

class ToolBox : public QWidget
{
        Q_OBJECT
    public:
        explicit ToolBox(QWidget *parent = nullptr);
        ToolboxTree* toolboxTree();

    protected:
        virtual QSize sizeHint() const override;

    private slots:
        void refreshList();
        void showAdderArea();
        void hideAdderArea();
        void handleSelectionChange();
        void handleMousePress(QTreeWidgetItem* item);
        void handleToolboxUrlboxChanges(const QString& text);
        void handleToolboxNameboxChanges(QString name);
        void toolboxEditButtonToggled(bool);
        void toolboxRemoveButtonClicked();
        void toolboxAddButtonClicked();
        void toolboxImportButtonClicked();
        void toolboxExportButtonClicked();
        void handleImports(const QStringList& fileNames);

    private:
        QVBoxLayout _toolboxVLay;
        QLineEdit _searchEdit;
        ToolboxTree _toolboxTree;
        QWidget _toolboxAdderAreaWidget;
        QVBoxLayout _toolboxAdderAreaVLay;
        QHBoxLayout _toolboxAdderAreaButtonSideHLay;
        FlatButton _toolboxAddButton;
        FlatButton _toolboxEditButton;
        FlatButton _toolboxRemoveButton;
        FlatButton _toolboxExportButton;
        FlatButton _toolboxImportButton;
        QVBoxLayout _toolboxAdderAreaEditingLayout;
        QLineEdit _toolboxUrlBox;
        QLineEdit _toolBoxNameBox;
};

#endif // TOOLBOX_H
