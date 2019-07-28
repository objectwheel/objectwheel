#ifndef FORMSCENE_H
#define FORMSCENE_H

#include <form.h>
#include <gadgetlayer.h>

#include <QPointer>
#include <QGraphicsScene>

class DesignerScene final : public QGraphicsScene
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerScene)

    friend class ControlRemovingManager; // For removeControl(), removeForm()
    friend class ControlCreationManager; // For addForm(), addControl()
    friend class ProjectExposingManager; // For addForm(), addControl()

public:
    explicit DesignerScene(QObject* parent = nullptr);

    qreal zoomLevel() const;

    Form* currentForm() const;
    void setCurrentForm(Form* currentForm);

    void unsetCursor();
    void setCursor(Qt::CursorShape cursor);
    void prepareDragLayer(DesignerItem* item);
    void shrinkSceneRect();

    DesignerItem* dragLayer() const;
    GadgetLayer* gadgetLayer() const;

    QList<Form*> forms() const;
    QVector<QLineF> guidelines() const;
    QList<Control*> controlsAt(const QPointF& pos) const;
    QList<Control*> selectedControls() const;
    QList<DesignerItem*> selectedItems() const;
    QList<DesignerItem*> draggedResizedSelectedItems() const;
    QRectF visibleItemsBoundingRect() const;
    QRectF outerRect(const QRectF& rect);

    static int startDragDistance();
    static QColor outlineColor();
    static QPointF snapPosition(qreal x, qreal y);
    static QPointF snapPosition(const QPointF& pos);
    static QSizeF snapSize(qreal x, qreal y, qreal w, qreal h);
    static QSizeF snapSize(const QPointF& pos,const QSizeF& size);
    static QRectF boundingRect(const QList<DesignerItem*>& items);
    static QPen pen(const QColor& color = outlineColor(), qreal width = 1, bool cosmetic = true);

    void paintOutline(QPainter* painter, const QRectF& rect);
    void paintSelectionOutline(QPainter* painter, DesignerItem* selectedItem);

public slots:
    void discharge();

private:
    void addForm(Form* form);
    void removeForm(Form* form);
    void removeControl(Control* control);

private:
    void drawForeground(QPainter* painter, const QRectF& rect) override;

signals:
    void currentFormChanged(Form* currentForm);

private:
    QList<Form*> m_forms;
    QPointer<Form> m_currentForm;
    QList<DesignerItem*> m_siblingsBeforeDrag;
    DesignerItem* m_dragLayer;
    GadgetLayer* m_gadgetLayer;
};

#endif // FORMSCENE_H
