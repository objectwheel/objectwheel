#ifndef PROPERTIESDELEGATE_H
#define PROPERTIESDELEGATE_H

#include <QStyledItemDelegate>

class QTreeWidgetItem;
class PropertiesTree;
class PropertiesDelegateCache;

class PropertiesDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertiesDelegate)

    friend class PropertiesTree; // For paintBackground
    friend class PropertiesController; // For lots of things

public:
    enum Type {
        Invalid,
        Url,
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

    struct Callback {
        std::function<void(const QVariant&)> call;
        QVariant toVariant() const {
            return QVariant::fromValue<Callback>(*this);
        }
    };

public:
    explicit PropertiesDelegate(PropertiesTree* propertiesTree);
    ~PropertiesDelegate() override;

    void reserveSmart();

private:
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void destroyItem(QTreeWidgetItem* item) const;
    void destroyEditor(QWidget* editor, const QModelIndex& index) const override;
    QTreeWidgetItem* createItem() const;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    int calculateVisibleRow(const QTreeWidgetItem* item) const;
    void paintBackground(QPainter* painter, const QStyleOptionViewItem& option, int rowNumber,
                         bool isClassRow, bool hasVerticalLine) const;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    template <typename... Args>
    static Callback makeCallback(Args&&... args)
    {
        Callback callback;
        callback.call = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
        return callback;
    }

private:
    void setValues(QWidget* widget, PropertiesDelegate::Type type, const QVariant& value) const;
    void setInitialValue(QWidget* widget, PropertiesDelegate::Type type, const QVariant& value) const;
    void setConnection(QWidget* widget, PropertiesDelegate::Type type, PropertiesDelegate::Callback callback) const;
    void clearWidget(QWidget* widget, PropertiesDelegate::Type type) const;
    int smartSize(PropertiesDelegate::Type type) const;
    QWidget* createWidget(PropertiesDelegate::Type type) const;

private:
    PropertiesTree* m_propertiesTree;
    PropertiesDelegateCache* m_cache;
};

Q_DECLARE_METATYPE(PropertiesDelegate::Callback)

#endif // PROPERTIESDELEGATE_H
