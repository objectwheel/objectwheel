#ifndef INSPECTORPANE_H
#define INSPECTORPANE_H

#include <QTreeWidget>
#include <QHash>
#include <QPointer>
#include <navigatorpane.h>

class Form;
class Control;
class DesignerScene;

class NavigatorController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(NavigatorController)

    struct FormState {
        QList<QString> selectedIds;
        QList<QString> collapsedIds;
        int verticalScrollBarPosition = 0;
        int horizontalScrollBarPosition = 0;
    };

public:
    explicit NavigatorController(NavigatorPane* navigatorPane, DesignerScene* designerScene,
                                 QObject* parent = nullptr);
public slots:
    void discharge();
    void clear();

private slots:
    void onControlParentChange(Control* control);
    void onControlIndexChange(Control* control);
    void onControlIdChange(Control* control, const QString& previousId);
    void onControlRenderInfoChange(Control* control, bool codeChanged);

    void onProjectStart();
    void onSceneSelectionChange();
    void onControlCreation(Control* control);
    void onControlRemove(Control* control);
    void onFormRemove(Control* control);
    void onCurrentFormChange(Form* currentForm);
    void onItemSelectionChange();

signals:
    void controlSelectionChanged(const QList<Control*>& selectedControls);

private:
    NavigatorPane* m_navigatorPane;
    DesignerScene* m_designerScene;
    QHash<Form*, FormState> m_formStates;
    QPointer<Form> m_currentForm;
    bool m_isSelectionHandlingBlocked;
    bool m_isProjectStarted;
};

#endif // INSPECTORPANE_H
