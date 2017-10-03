#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QtWidgets>

#include <flatbutton.h>
#include <listwidget.h>
#include <fit.h>
#include <lineedit.h>

class ToolBox : public QWidget
{
        Q_OBJECT
    public:
        explicit ToolBox(QWidget *parent = nullptr);
        ListWidget* toolboxList();

    protected:
        virtual void paintEvent(QPaintEvent *event) override;
        virtual QSize sizeHint() const override;

    private slots:
        void showAdderArea();
        void hideAdderArea();
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
        ListWidget _toolboxList;
        QWidget _toolboxAdderAreaWidget;
        QVBoxLayout _toolboxAdderAreaVLay;
        QHBoxLayout _toolboxAdderAreaButtonSideHLay;
        FlatButton _toolboxAddButton;
        FlatButton _toolboxEditButton;
        FlatButton _toolboxRemoveButton;
        FlatButton _toolboxExportButton;
        FlatButton _toolboxImportButton;
        QVBoxLayout _toolboxAdderAreaEditingLayout;
        LineEdit _toolboxUrlBox;
        LineEdit _toolBoxNameBox;
};

#endif // TOOLBOX_H
