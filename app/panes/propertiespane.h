#ifndef PROPERTIESPANE_H
#define PROPERTIESPANE_H

#include <QTreeWidget>

class LineEdit;
class DesignerScene;
class Control;

class PropertiesPane : public QTreeWidget
{
    Q_OBJECT

    friend class PropertiesListDelegate; // For itemFromIndex()

public:
    explicit PropertiesPane(DesignerScene* designerScene, QWidget* parent = nullptr);

public slots:
    void discharge();

private slots:
    void onSelectionChange();
    void onZChange(Control*);
    void onPreviewChange(Control*, bool codeChanged);
    void onGeometryChange(const Control*);
    void onIdChange(Control*, const QString& previousId);
    void onPropertyChange();

private:
    void filterList(const QString& filter);
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    void paintEvent(QPaintEvent* e) override;
    void updateGeometries() override;
    QSize sizeHint() const override;

public:
    DesignerScene* m_designerScene;
    LineEdit* m_searchEdit;
};

#endif // PROPERTIESPANE_H
