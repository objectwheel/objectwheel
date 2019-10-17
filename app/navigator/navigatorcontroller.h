#ifndef NAVIGATORCONTROLLER_H
#define NAVIGATORCONTROLLER_H

#include <QHash>
#include <QPointer>
#include <QStringListModel>

class Form;
class Control;
class DesignerScene;
class NavigatorPane;
class QTreeWidgetItem;

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
    void onItemDoubleClick(QTreeWidgetItem* item, int column);
    void onItemSelectionChange();
    void onSceneSelectionChange();

    void onControlCreation(Control* control);
    void onControlRemove(Control* control);
    void onFormRemove(Control* control);
    void onCurrentFormChange(Form* currentForm);
    void onControlParentChange(Control* control);
    void onControlIndexChange(Control* control) const;
    void onControlIdChange(Control* control, const QString& previousId);

private:
    void addCompleterEntry(const QString& entry);
    void removeCompleterEntry(const QString& entry);
    void expandRecursive(const QTreeWidgetItem* parentItem);
    void addControls(QTreeWidgetItem* parentItem, const QList<Control*>& controls);

signals:
    void goToSlotActionTriggered();
    void editAnchorsActionTriggered();
    void viewSourceCodeActionTriggered();
    void controlSelectionChanged(const QList<Control*>& selectedControls);

private:
    NavigatorPane* m_navigatorPane;
    DesignerScene* m_designerScene;
    QPointer<Form> m_currentForm;
    QStringListModel m_searchCompleterModel;
    QHash<Form*, FormState> m_formStates;
    bool m_isSelectionHandlingBlocked;
    bool m_isProjectStarted;
};

#endif // NAVIGATORCONTROLLER_H
