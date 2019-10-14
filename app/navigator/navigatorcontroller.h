#ifndef INSPECTORPANE_H
#define INSPECTORPANE_H

#include <QTreeWidget>
#include <QHash>
#include <QPointer>
#include <QStringListModel>

class Form;
class Control;
class DesignerScene;
class NavigatorPane;

class NavigatorController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(NavigatorController)

    struct FormState {
        QList<Control*> collapsedControls;
        int verticalScrollBarPosition = 0;
        int horizontalScrollBarPosition = 0;
    };

public:
    explicit NavigatorController(NavigatorPane* navigatorPane, DesignerScene* designerScene,
                                 QObject* parent = nullptr);

    Control* controlFromItem(const QTreeWidgetItem* item) const;
    QTreeWidgetItem* itemFromControl(const Control* control) const;

public slots:
    void discharge();
    void clear();

private slots:
    void onProjectStart();
    void onSearchEditReturnPress();
    void onSceneSelectionChange();
    void onItemSelectionChange();
    void onCurrentFormChange(Form* currentForm);
    void onControlCreation(Control* control);
    void onControlRemove(Control* control);
    void onFormRemove(Control* control);
    void onControlParentChange(Control* control);
    void onControlIndexChange(Control* control);
    void onControlIdChange(Control* control, const QString& previousId);

private:
    void addControls(QTreeWidgetItem* parentItem, const QList<Control*>& controls);

signals:
    void controlSelectionChanged(const QList<Control*>& selectedControls);

private:
    NavigatorPane* m_navigatorPane;
    DesignerScene* m_designerScene;
    QHash<Form*, FormState> m_formStates;
    QPointer<Form> m_currentForm;
    QStringListModel m_searchCompleterModel;
    bool m_isSelectionHandlingBlocked;
    bool m_isProjectStarted;
};

#endif // INSPECTORPANE_H
