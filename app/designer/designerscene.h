#ifndef FORMSCENE_H
#define FORMSCENE_H

#include <form.h>
#include <QPointer>
#include <QGraphicsScene>

class GadgetLayer;
class AnchorLayer;
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

    void addForm(Form* form);
    void removeForm(Form* form);
    void removeControl(Control* control);
    void setCurrentForm(Form* currentForm);

    void shrinkSceneRect();
    void unsetCursor() const;
    void setCursor(Qt::CursorShape cursor) const;
    void prepareDragLayer(DesignerItem* item);
    bool isLayerItem(DesignerItem* item) const;

    Form* currentForm() const;
    DesignerItem* dragLayer() const;
    GadgetLayer* gadgetLayer() const;
    AnchorLayer* anchorLayer() const;
    PaintLayer* paintLayer() const;
    DesignerItem* parentBeforeDrag() const;

    QList<Form*> forms() const;
    QList<Control*> selectedControls() const;
    QList<DesignerItem*> selectedItems() const;
    QList<DesignerItem*> draggedResizedSelectedItems() const;

    Control* topLevelControl(const QPointF& pos) const;
    Control* highlightControl(const QPointF& pos) const;

    qreal devicePixelRatio() const;
    QPointF cursorPos() const;
    QRectF visibleItemsBoundingRect() const;
    QVector<QLineF> guidelines() const;

    template <typename T = DesignerItem, typename... Args>
    QList<T*> items(Args&&... args) const;

public:
    static bool showGridViewDots();
    static bool showClippedControls();
    static bool showMouseoverOutline();

    static int gridSize();
    static int startDragDistance();

    static qreal zoomLevel();
    static OutlineMode outlineMode();
    static QColor outlineColor();
    static QBrush backgroundTexture();
    static QBrush blankControlDecorationBrush(const QColor& color);
    static QPen pen(const QColor& color = outlineColor(), qreal width = 1, bool cosmetic = true);

    static QPointF snapPosition(qreal x, qreal y);
    static QPointF snapPosition(const QPointF& pos);

    static QSizeF snapSize(qreal x, qreal y, qreal w, qreal h);
    static QSizeF snapSize(const QPointF& pos,const QSizeF& size);

    static QRectF outerRect(const QRectF& rect);
    static QRectF itemsBoundingRect(const QList<DesignerItem*>& items);

    static qreal lowerZ(DesignerItem* parentItem);
    static qreal higherZ(DesignerItem* parentItem);

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
    AnchorLayer* m_anchorLayer;
    PaintLayer* m_paintLayer;
    QSet<Form*> m_forms;
    DesignerItem* m_parentBeforeDrag;
    QPointer<Form> m_currentForm;
    QPointer<DesignerItem> m_recentHighlightedItem;
};

template <typename T, typename... Args>
inline QList<T*> DesignerScene::items(Args&&... args) const
{
    QList<T*> items;
    const QList<QGraphicsItem*>& allItems = QGraphicsScene::items(std::forward<Args>(args)...);
    for (QGraphicsItem* item : allItems) { // detaches allItems if it is not const
        if (item->type() >= T::Type)
            items.append(static_cast<T*>(item));
    }
    return items;
}

#endif // FORMSCENE_H
