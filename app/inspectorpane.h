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

protected:
    QSize sizeHint() const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onSelectionChange();
    void onControlAdd(Control* control);
    void onControlRemove(Control* control);
    void onFormRemove(Form* form);
    void onCurrentFormChange(Form* currentForm);
    void onItemClick(QTreeWidgetItem* item, int column);
    void onItemDoubleClick(QTreeWidgetItem* item, int column);

signals:
    void controlClicked(Control* control);
    void controlDoubleClicked(Control* control);

private:
    DesignerScene* m_designerScene;
    QPointer<Form> m_currentForm;
    QHash<Form*, FormState> m_formStates;
};

#endif // INSPECTORPANE_H
