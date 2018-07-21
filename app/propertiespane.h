#ifndef PROPERTIESPANE_H
#define PROPERTIESPANE_H

#include <QTreeWidget>

class FocuslessLineEdit;
class DesignerScene;

class PropertiesPane : public QTreeWidget
{
    Q_OBJECT

public:
    explicit PropertiesPane(DesignerScene* designerScene, QWidget* parent = nullptr);

public slots:
    void sweep();

private:
//    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
//    void paintEvent(QPaintEvent* e) override;
    void updateGeometries() override;
    QSize sizeHint() const override;

public:
    DesignerScene* m_designerScene;
    FocuslessLineEdit* m_searchEdit;
};

#endif // PROPERTIESPANE_H
