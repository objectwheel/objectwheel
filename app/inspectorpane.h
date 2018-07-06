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

    friend class InspectorListDelegate; // For itemFromIndex()

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
    void handleControlParentChange(Control* control);
    void handleControlIdChange(Control* control, const QString& previousId);
    void handleControlPreviewChange(Control* control);

private slots:
    void onProjectStart();
    void onSelectionChange();
    void onControlCreation(Control* control);
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
    QHash<Form*, FormState> m_formStates;
    QPointer<Form> m_currentForm;
};

#endif // INSPECTORPANE_H
