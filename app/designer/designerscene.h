#ifndef FORMSCENE_H
#define FORMSCENE_H

#include <QGraphicsScene>
#include <QPointer>
#include <QSet>
#include <QGraphicsItem>

class DesignerItem;
class Control;
class Form;
class GadgetLayer;
class MouseLayer;
class PaintLayer;
class DesignerView;

class DesignerScene final : public QGraphicsScene
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerScene)

public:
    enum AnchorVisibilityReasons {
        VisibleForSelectedControlsOnly = 0x00,
        VisibleForAllControlsDueToAnchorLayer = 0x02,
        VisibleForAllControlsDueToAnchorEditor = 0x04,
        VisibleForAllControlsDueToAnchorEditorConnection = 0x08
    };
    Q_DECLARE_FLAGS(AnchorVisibility, AnchorVisibilityReasons)

public:
    explicit DesignerScene(QObject* parent = nullptr);

    void clear();
    void addForm(Form* form);
    void removeForm(Form* form);
    void removeControl(Control* control);
    void setCurrentForm(Form* currentForm);

    void shrinkSceneRect();
    void unsetCursor();
    void setCursor(Qt::CursorShape cursorShape);
    void prepareDragLayer(const DesignerItem* item);
    bool isLayerItem(const DesignerItem* item) const;

    Form* currentForm() const;
    DesignerItem* dragLayer() const;
    GadgetLayer* gadgetLayer() const;
    MouseLayer* mouseLayer() const;
    DesignerItem* parentBeforeDrag() const;
    DesignerView* view() const;

    QList<Form*> forms() const;
    QList<Control*> selectedControls() const;
    QList<DesignerItem*> selectedItems() const;
    QList<DesignerItem*> draggedResizedSelectedItems() const;

    Control* topLevelControl(const QPointF& pos) const;
    Control* highlightControl(const QPointF& pos) const;

    QRectF outerRect(const QRectF& rect) const;
    QRectF visibleItemsBoundingRect() const;

    qreal zoomLevel() const;
    qreal devicePixelRatio() const;

    QPointF cursorPos() const;
    QVector<QLineF> guidelines() const;

    AnchorVisibility anchorVisibility() const;
    void setAnchorVisibility(AnchorVisibility anchorVisibility);

    void reparentControl(Control* control, Control* parentControl, const QPointF& pos = QPointF()) const;

    template <typename T = DesignerItem, typename... Args>
    QList<T*> items(Args&&... args) const;

public:
    static bool isAnchorViable(const Control* sourceControl, const Control* targetControl);
    static bool isInappropriateAnchorSource(const Control* control);
    static bool isInappropriateAnchorTarget(const Control* control);

    static QPointF snapPosition(qreal x, qreal y);
    static QPointF snapPosition(const QPointF& pos);

    static QSizeF snapSize(qreal x, qreal y, qreal w, qreal h);
    static QSizeF snapSize(const QPointF& pos,const QSizeF& size);

    static QRectF rect(const Control* control);
    static QRectF contentRect(const Control* control);
    static QRectF itemsBoundingRect(const QList<DesignerItem*>& items);

    static qreal lowerZ(const DesignerItem* parentItem);
    static qreal higherZ(const DesignerItem* parentItem);

    static QPen pen(const QColor& color = QColor(), qreal width = 1, bool cosmetic = true);

    static void drawDashLine(QPainter* painter, const QLineF& line);
    static void drawDashRect(QPainter* painter, const QRectF& rect);

private slots:
    void onChange();
    void onAnchorLayerActivation();
    void onSceneSettingsChange();

private:
    void handleToolDrop(QGraphicsSceneDragDropEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
    void dropEvent(QGraphicsSceneDragDropEvent* event) override;

signals:
    void currentFormChanged(Form* currentForm);
    void anchorEditorActivated(Control* sourceControl, Control* targetControl);

private:
    AnchorVisibility m_anchorVisibility;
    Qt::CursorShape m_cursorShape;
    DesignerItem* m_dragLayer;
    GadgetLayer* m_gadgetLayer;
    MouseLayer* m_mouseLayer;
    PaintLayer* m_paintLayer;
    QSet<Form*> m_forms;
    DesignerItem* m_parentBeforeDrag;
    QPointer<Form> m_currentForm;
    QPointer<DesignerItem> m_recentHighlightedItem;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DesignerScene::AnchorVisibility)

template <typename T, typename... Args>
inline QList<T*> DesignerScene::items(Args&&... args) const
{
    QList<T*> items;
    const QList<QGraphicsItem*>& allItems = QGraphicsScene::items(std::forward<Args>(args)...);
    for (QGraphicsItem* item : allItems) {
        if (item->type() >= T::Type)
            items.append(static_cast<T*>(item));
    }
    return items;
}

#endif // FORMSCENE_H
