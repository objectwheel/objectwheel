#ifndef INSPECTORPANE_H
#define INSPECTORPANE_H

#include <QTreeWidget>
#include <QHash>
#include <QPointer>

class Form;
class Control;
class DesignerScene;

class InspectorPane : public QTreeWidget
{
    Q_OBJECT

    struct FormState {
        QList<QString> selectedIds;
        QList<QString> collapsedIds;

        int verticalScrollBarPosition = 0;
        int horizontalScrollBarPosition = 0;
    };

public:
    explicit InspectorPane(DesignerScene* designerScene, QWidget* parent = nullptr);

public slots:
    void sweep();

private slots:
    void onSelectionChange();
    void onControlAdd(Control* control);
    void onControlRemove(Control* control);
    void onFormRemove(Form* form);
    void onCurrentFormChange(Form* currentForm);
    void onItemSelectionChange();
    void onItemDoubleClick(QTreeWidgetItem* item, int column);

private:
    void paintEvent(QPaintEvent* e) override;
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    QSize sizeHint() const override;

signals:
    void controlDoubleClicked(Control* control);
    void controlSelectionChanged(const QList<Control*>& selectedControls);

private:
    DesignerScene* m_designerScene;
    QPointer<Form> m_currentForm;
    QHash<Form*, FormState> m_formStates;
};

#endif // INSPECTORPANE_H
