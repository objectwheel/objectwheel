#ifndef FORMSCONTROLLER_H
#define FORMSCONTROLLER_H

#include <QStringListModel>

class DesignerScene;
class FormsPane;

class FormsController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FormsController)

public:
    explicit FormsController(FormsPane* formsPane, DesignerScene* designerScene,
                             QObject* parent = nullptr);

public slots:
    void discharge();
    void refresh();

private slots:
    void onAddButtonClick();
    void onRemoveButtonClick();
    void onCurrentItemChange();
    void onSearchEditReturnPress();

private:
    void addCompleterEntry(const QString& entry);
    void removeCompleterEntry(const QString& entry);

private:
    FormsPane* m_formsPane;
    DesignerScene* m_designerScene;
    QStringListModel m_searchCompleterModel;
    bool m_isProjectStarted;
};

#endif // FORMSCONTROLLER_H
