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
    void discharge();
    void onControlParentChange(Control* control);
    void onControlIndexChange(Control* control);
    void onControlIdChange(Control* control, const QString& previousId);
    void onControlRenderInfoChange(Control* control, bool codeChanged);

private slots:
    void onProjectStart();
    void onSceneSelectionChange();
    void onControlCreation(Control* control);
    void onControlRemove(Control* control);
    void onFormRemove(Control* control);
    void onCurrentFormChange(Form* currentForm);
    void onItemSelectionChange();

private:
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    void paintEvent(QPaintEvent* e) override;
    QSize sizeHint() const override;

signals:
    void controlSelectionChanged(const QList<Control*>& selectedControls);

private:
    DesignerScene* m_designerScene;
    QHash<Form*, FormState> m_formStates;
    QPointer<Form> m_currentForm;
};

#endif // INSPECTORPANE_H