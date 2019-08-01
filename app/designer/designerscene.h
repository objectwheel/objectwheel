#ifndef FORMSCENE_H
#define FORMSCENE_H

#include <form.h>
#include <QPointer>
#include <QGraphicsScene>

class GadgetLayer;
class PaintLayer;

class DesignerScene final : public QGraphicsScene
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerScene)

    friend class ControlRemovingManager; // For removeControl(), removeForm()
    friend class ControlCreationManager; // For addForm(), addControl()
    friend class ProjectExposingManager; // For addForm(), addControl()

public:
    enum OutlineMode {
        NoOutline,
        ClippingDashLine,
        BoundingDashLine,
        ClippingSolidLine,
        BoundingSolidLine
    };

public:
    explicit DesignerScene(QObject* parent = nullptr);

    Form* currentForm() const;
    void setCurrentForm(Form* currentForm);

    void unsetCursor();
    void setCursor(Qt::CursorShape cursor);
    void prepareDragLayer(DesignerItem* item);
    void shrinkSceneRect();
    bool isLayerItem(DesignerItem* item) const;

    DesignerItem* dragLayer() const;
    GadgetLayer* gadgetLayer() const;
    PaintLayer* paintLayer() const;

    QList<Form*> forms() const;
    QRectF visibleItemsBoundingRect() const;
    QVector<QLineF> guidelines() const;
    QList<Control*> selectedControls() const;
    QList<DesignerItem*> selectedItems() const;
    QList<DesignerItem*> draggedResizedSelectedItems() const;

    template <typename... Args>
    QList<DesignerItem*> items(Args&&... args) const
    {
        QList<DesignerItem*> items;
        for (QGraphicsItem* item : QGraphicsScene::items(std::forward<Args>(args)...)) {
            if (item->type() >= DesignerItem::Type)
                items.append(static_cast<DesignerItem*>(item));
        }
        return items;

    }

    static bool showMouseoverOutline();
    static int startDragDistance();
    static int gridSize();
    static qreal zoomLevel();
    static qreal lowerZ(DesignerItem* parentItem);
    static qreal higherZ(DesignerItem* parentItem);
    static QColor outlineColor();
    static QPointF snapPosition(qreal x, qreal y);
    static QPointF snapPosition(const QPointF& pos);
    static QSizeF snapSize(qreal x, qreal y, qreal w, qreal h);
    static QSizeF snapSize(const QPointF& pos,const QSizeF& size);
    static QRectF itemsBoundingRect(const QList<DesignerItem*>& items);
    static QRectF outerRect(const QRectF& rect);
    static QPen pen(const QColor& color = outlineColor(), qreal width = 1, bool cosmetic = true);
    static QBrush backgroundTexture();
    static OutlineMode outlineMode();
    static void drawDashLine(QPainter* painter, const QLineF& line);
    static void drawDashRect(QPainter* painter, const QRectF& rect);

public slots:
    void discharge();

private:
    void addForm(Form* form);
    void removeForm(Form* form);
    void removeControl(Control* control);

private:
    DesignerItem* highlightItem(const QPointF& pos) const;

private:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

signals:
    void currentFormChanged(Form* currentForm);

private:
    DesignerItem* m_dragLayer;
    GadgetLayer* m_gadgetLayer;
    PaintLayer* m_paintLayer;
    QPointer<Form> m_currentForm;
    QList<Form*> m_forms;
    QPointer<DesignerItem> m_recentHighlightedItem;
    QList<DesignerItem*> m_siblingsBeforeDrag;
};

#endif // FORMSCENE_H
