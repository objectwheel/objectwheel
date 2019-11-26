#ifndef FORMSCONTROLLER_H
#define FORMSCONTROLLER_H

#include <QStringListModel>

class DesignerScene;
class FormsPane;
class Control;
class QTreeWidgetItem;

class FormsController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FormsController)

public:
    explicit FormsController(FormsPane* formsPane, DesignerScene* designerScene,
                             QObject* parent = nullptr);

    Control* controlFromItem(const QTreeWidgetItem* item) const;
    QTreeWidgetItem* itemFromControl(const Control* control) const;

public slots:
    void discharge();
    void clear();

private slots:
    void onProjectStart();
    void onAddButtonClick();
    void onRemoveButtonClick();
    void onSearchEditReturnPress();
    void onControlCreation(Control* control);
    void onControlRemove(Control* control);
    void onControlIdChange(Control* control, const QString& previousId);
    void onControlIndexChange(Control* control) const;
    void onCurrentFormChange();
    void onItemSelectionChange();

private:
    void addCompleterEntry(const QString& entry);
    void removeCompleterEntry(const QString& entry);

private:
    FormsPane* m_formsPane;
    DesignerScene* m_designerScene;
    QStringListModel m_searchCompleterModel;
    bool m_isProjectStarted;
    bool m_isSelectionHandlingBlocked;
};

#endif // FORMSCONTROLLER_H