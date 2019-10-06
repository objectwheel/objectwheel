#ifndef PROPERTIESDELEGATE_H
#define PROPERTIESDELEGATE_H

#include <QStyledItemDelegate>

class PropertiesTree;
class QTreeWidgetItem;
class PropertiesDelegateCache;

class PropertiesDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertiesDelegate)

public:
    enum { ROW_HEIGHT = 21 };

    enum Type {
        Invalid,
        String,
        Enum,
        Bool,
        Color,
        Int,
        Real,
        FontSize,
        FontFamily,
        FontWeight,
        FontCapitalization
    };
    Q_ENUM(Type)

    enum Roles {
        TypeRole = Qt::UserRole + 1,
        CallbackRole,
        ValuesRole,
        InitialValueRole,
        ModificationRole
    };

public:
    explicit PropertiesDelegate(PropertiesTree* propertiesTree);
    ~PropertiesDelegate() override;

    void reserve(int size);
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
    QTreeWidgetItem* createItem() const;
    void destroyItem(QTreeWidgetItem* item) const;

private:
    QWidget* createWidget(int type) const;
    void addConnection(QWidget* widget, int type, const QString& propertyName) const;
    void clearConnection(QWidget* widget) const;

signals:
    void propertyEdited(int type, const QString& propertyName, const QVariant& value) const;

private:
    PropertiesTree* m_propertiesTree;
    PropertiesDelegateCache* m_cache;
};

#endif // PROPERTIESDELEGATE_H
