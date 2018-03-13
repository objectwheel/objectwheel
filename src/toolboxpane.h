#ifndef TOOLBOXPANE_H
#define TOOLBOXPANE_H

#include <QWidget>

class QVBoxLayout;
class ToolboxTree;
class FocuslessLineEdit;
class QTreeWidgetItem;
class MainWindow;

class ToolboxPane : public QWidget
{
        Q_OBJECT

    public:
        explicit ToolboxPane(QWidget* parent = nullptr);
        ToolboxTree* toolboxTree();

    public slots:
        void reset();
        void clear();

    protected:
        QSize sizeHint() const override;

    private slots:
        void filterList(const QString& filter);
        void handleMousePress(QTreeWidgetItem* item);

    private:
        QVBoxLayout* _layout;
        ToolboxTree* _toolboxTree;
        FocuslessLineEdit* _searchEdit;
};

#endif // TOOLBOXPANE_H
