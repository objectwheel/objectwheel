#ifndef PROPERTIESPANE_H
#define PROPERTIESPANE_H

#include <QWidget>

class PropertiesTree;
class LineEdit;
class QLineEdit;
class QSpinBox;
class QTreeWidgetItem;

class PropertiesPane final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertiesPane)

public:
    explicit PropertiesPane(QWidget* parent = nullptr);

    PropertiesTree* propertiesTree() const;
    LineEdit* searchEdit() const;
    QTreeWidgetItem* typeItem() const;
    QTreeWidgetItem* uidItem() const;
    QTreeWidgetItem* idItem() const;
    QTreeWidgetItem* indexItem() const;
    QLineEdit* idEdit() const;
    QSpinBox* indexEdit() const;

    QSize sizeHint() const override;

public:
    PropertiesTree* m_propertiesTree;
    LineEdit* m_searchEdit;
    QTreeWidgetItem* m_typeItem;
    QTreeWidgetItem* m_uidItem;
    QTreeWidgetItem* m_idItem;
    QTreeWidgetItem* m_indexItem;
    QLineEdit* m_idEdit;
    QSpinBox* m_indexEdit;
};

#endif // PROPERTIESPANE_H
