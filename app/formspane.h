#ifndef FORMSPANE_H
#define FORMSPANE_H

#include <QTreeWidget>

class FlatButton;
class DesignerScene;

class FormsPane : public QTreeWidget
{
    Q_OBJECT

public:
    explicit FormsPane(DesignerScene* designerScene, QWidget* parent = nullptr);

public slots:
    void sweep();
    void refresh();

private slots:
    void onAddButtonClick();
    void onRemoveButtonClick();
    void onCurrentItemChange();

private:
    void paintEvent(QPaintEvent* e) override;
    void updateGeometries() override;
    QSize sizeHint() const override;

private:
    DesignerScene* m_designerScene;
    FlatButton* m_addButton;
    FlatButton* m_removeButton;
};

#endif // FORMSPANE_H
