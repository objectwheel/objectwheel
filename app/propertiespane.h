#ifndef PROPERTIESPANE_H
#define PROPERTIESPANE_H

#include <QTreeWidget>

class FocuslessLineEdit;
class DesignerScene;
class Control;

class PropertiesPane : public QTreeWidget
{
    Q_OBJECT

    friend class PropertiesListDelegate; // For itemFromIndex()

public:
    explicit PropertiesPane(DesignerScene* designerScene, QWidget* parent = nullptr);

public slots:
    void sweep();

private slots:
    void onSelectionChange();
    void onZChange(Control*);
    void onPreviewChange(Control*);
    void onGeometryChange(Control*);
    void onPropertyChange(Control*, const QString& propertyName);
    void onIdChange(Control*, const QString& previousId);

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
