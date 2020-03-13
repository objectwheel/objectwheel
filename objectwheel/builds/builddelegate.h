#ifndef BUILDDELEGATE_H
#define BUILDDELEGATE_H

#include <styleditemdelegate.h>

class BuildDelegate final : public StyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildDelegate)

public:
    explicit BuildDelegate(QObject* parent = nullptr);

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* widget, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

signals:
    void infoButtonClicked(const QModelIndex& parent) const;
    void deleteButtonClicked(const QModelIndex& parent) const;
    void openFolderButtonClicked(const QModelIndex& parent) const;
};

#endif // BUILDDELEGATE_H
