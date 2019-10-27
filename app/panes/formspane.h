#ifndef FORMSPANE_H
#define FORMSPANE_H

#include <QTreeWidget>

class QPushButton;
class DesignerScene;

class FormsPane : public QTreeWidget
{
    Q_OBJECT

public:
    explicit FormsPane(DesignerScene* designerScene, QWidget* parent = nullptr);

public slots:
    void discharge();
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
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
};

#endif // FORMSPANE_H
