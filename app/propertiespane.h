#ifndef PROPERTIESPANE_H
#define PROPERTIESPANE_H

#include <QTreeWidget>

class QVBoxLayout;
class FocuslessLineEdit;
class DesignerScene;
class Control;

class PropertiesPane : public QTreeWidget
{
    Q_OBJECT

public:
    explicit PropertiesPane(DesignerScene* designerScene, QWidget* parent = nullptr);
    void saveChanges(const QString& property, const QString& parserValue, const QVariant& value);

public slots:
    void sweep();
    void refreshList();

private slots:
    void clearList();
    void handleSelectionChange();
    void filterList(const QString& filter);

private:
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    void paintEvent(QPaintEvent* e) override;
    void updateGeometries() override;
    QSize sizeHint() const override;

public:
    DesignerScene* m_designerScene;
    FocuslessLineEdit* m_searchEdit;
};

#endif // PROPERTIESPANE_H
