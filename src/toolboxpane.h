#ifndef TOOLBOXPANE_H
#define TOOLBOXPANE_H

#include <QWidget>

class QVBoxLayout;
class ToolboxTree;
class FilterLineEdit;
class QTreeWidgetItem;
class MainWindow;

class ToolboxPane : public QWidget
{
        Q_OBJECT

    public:
        explicit ToolboxPane(MainWindow* parent);
        ToolboxTree* toolboxTree();

    protected:
        QSize sizeHint() const override;

    private slots:
        void filterList(const QString& filter);
        void handleMousePress(QTreeWidgetItem* item);

    private:
        QVBoxLayout* _layout;
        ToolboxTree* _toolboxTree;
        FilterLineEdit* _searchEdit;
};

#endif // TOOLBOXPANE_H
