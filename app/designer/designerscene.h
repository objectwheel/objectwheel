#ifndef FORMSCENE_H
#define FORMSCENE_H

#include <form.h>
#include <QPointer>
#include <QGraphicsScene>

class GadgetLayer;
class PaintLayer;
class QMimeData;

class DesignerScene final : public QGraphicsScene
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerScene)

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

    void addForm(Form* form);
    void removeForm(Form* form);
    void removeControl(Control* control);

    void shrinkSceneRect();
    void unsetCursor();
    void setCursor(Qt::CursorShape cursor);
    void prepareDragLayer(DesignerItem* item);
    bool isLayerItem(DesignerItem* item) const;

    DesignerItem* dragLayer() const;
    GadgetLayer* gadgetLayer() const;
    PaintLayer* paintLayer() const;

    DesignerItem* dropItem(const QPointF& pos) const;
    DesignerItem* highlightItem(const QPointF& pos) const;

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
    static bool showClippedControls();
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
    static QBrush blankControlDecorationBrush(const QColor& color);
    static OutlineMode outlineMode();
    static void drawDashLine(QPainter* painter, const QLineF& line);
    static void drawDashRect(QPainter* painter, const QRectF& rect);

public slots:
    void discharge();

private slots:
    void onChange();
    void onHeadlineDoubleClick(bool isFormHeadline);

private:
    void handleToolDrop(QGraphicsSceneDragDropEvent* event);
    void reparentControl(Control* control, Control* parentControl) const;

private:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
    void dropEvent(QGraphicsSceneDragDropEvent* event) override;

signals:
    void currentFormChanged(Form* currentForm);

private:
    DesignerItem* m_dragLayer;
    GadgetLayer* m_gadgetLayer;
    PaintLayer* m_paintLayer;
    QSet<Form*> m_forms;
    QList<DesignerItem*> m_siblingsBeforeDrag;
    QPointer<Form> m_currentForm;
    QPointer<DesignerItem> m_recentHighlightedItem;
};

#endif // FORMSCENE_H
