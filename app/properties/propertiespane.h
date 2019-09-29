#ifndef PROPERTIESPANE_H
#define PROPERTIESPANE_H

#include <QTreeWidget>

class LineEdit;
class DesignerScene;
class Control;
class QSpinBox;

class PropertiesPane final : public QTreeWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertiesPane)

public:
    explicit PropertiesPane(DesignerScene* designerScene, QWidget* parent = nullptr);

public slots:
    void discharge();

private:
    void filterList(const QString& filter);
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    void paintEvent(QPaintEvent* e) override;
    void updateGeometries() override;
    QSize sizeHint() const override;

public:
    DesignerScene* m_designerScene;
    LineEdit* m_searchEdit;
    QTreeWidgetItem* m_typeItem;
    QTreeWidgetItem* m_uidItem;
    QTreeWidgetItem* m_idItem;
    QTreeWidgetItem* m_indexItem;
    QLineEdit* m_idEdit;
    QSpinBox* m_indexEdit;
};

#endif // PROPERTIESPANE_H
