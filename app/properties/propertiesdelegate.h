#ifndef PROPERTIESDELEGATE_H
#define PROPERTIESDELEGATE_H

#include <QStyledItemDelegate>

class PropertiesTree;
class QTreeWidgetItem;

class PropertiesDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertiesDelegate)

public:
    enum { ROW_HEIGHT = 21 };

public:
    explicit PropertiesDelegate(PropertiesTree* propertiesTree);

    int calculateVisibleRow(const QTreeWidgetItem* item) const;
    void paintBackground(QPainter* painter, const QStyleOptionViewItem& option, int rowNumber,
                         bool isClassRow, bool hasVerticalLine) const;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void destroyEditor(QWidget* editor, const QModelIndex& index) const override;

private:
    PropertiesTree* m_propertiesTree;
};

#endif // PROPERTIESDELEGATE_H
