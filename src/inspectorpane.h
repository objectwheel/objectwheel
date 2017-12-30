#ifndef INSPECTORPANE_H
#define INSPECTORPANE_H

#include <QWidget>

class Control;
class QVBoxLayout;
class QTreeWidget;
class QTreeWidgetItem;
class MainWindow;

class InspectorPane : public QWidget
{
        Q_OBJECT

    public:
        explicit InspectorPane(MainWindow* parent);

    public slots:
        void clear();
        void refresh();

    protected:
        QSize sizeHint() const override;
        bool eventFilter(QObject* watched, QEvent* event) override;

    private slots:
        void handleClick(QTreeWidgetItem* item, int column);
        void handleDoubleClick(QTreeWidgetItem* item, int column);

    signals:
        void controlClicked(Control* control);
        void controlDoubleClicked(Control* control);

    private:
        bool _blockRefresh;
        QVBoxLayout* _layout;
        QTreeWidget* _treeWidget;
};

#endif // INSPECTORPANE_H
