#ifndef INSPECTORPANE_H
#define INSPECTORPANE_H

#include <QTreeWidget>

class Control;
class DesignerScene;

class InspectorPane : public QTreeWidget
{
    Q_OBJECT

public:
    explicit InspectorPane(DesignerScene* designerScene, QWidget* parent = nullptr);

public slots:
    void reset() override;

protected:
    QSize sizeHint() const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onControlAdd();
    void onControlRemove();
    void onSelectionChange();
    void onCurrentFormChange();
    void onItemClick(QTreeWidgetItem* item, int column);
    void onItemDoubleClick(QTreeWidgetItem* item, int column);
signals:
    void controlClicked(Control* control);
    void controlDoubleClicked(Control* control);

private:
    DesignerScene* m_designerScene;
};

#endif // INSPECTORPANE_H
