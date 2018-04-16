#ifndef INSPECTORPANE_H
#define INSPECTORPANE_H

#include <QWidget>

class Control;
class QVBoxLayout;
class QTreeWidget;
class QTreeWidgetItem;
class DesignerScene;

class InspectorPane : public QWidget
{
    Q_OBJECT

public:
    explicit InspectorPane(DesignerScene* designerScene, QWidget* parent = nullptr);

public slots:
    void reset();
    void refresh();

protected:
    QSize sizeHint() const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void clear();
    void handleClick(QTreeWidgetItem* item, int column);
    void handleDoubleClick(QTreeWidgetItem* item, int column);

signals:
    void controlClicked(Control* control);
    void controlDoubleClicked(Control* control);

private:
    bool _blockRefresh;
    DesignerScene* m_designerScene;
    QVBoxLayout* _layout;
    QTreeWidget* _treeWidget;
};

#endif // INSPECTORPANE_H
