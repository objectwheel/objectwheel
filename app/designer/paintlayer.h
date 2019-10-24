#ifndef PAINTLAYER_H
#define PAINTLAYER_H

#include <designeritem.h>
#include <anchors.h>

class PaintLayer final : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(PaintLayer)

public:
    struct AnchorData {
        QPointF startPoint;
        QPointF firstControlPoint;
        QPointF secondControlPoint;
        QPointF endPoint;
        QPointF sourceAnchorLineFirstPoint;
        QPointF sourceAnchorLineSecondPoint;
        QPointF targetAnchorLineFirstPoint;
        QPointF targetAnchorLineSecondPoint;
        AnchorLine::Type sourceAnchorLineType = AnchorLine::Invalid;
        AnchorLine::Type targetAnchorLineType = AnchorLine::Invalid;
        Anchors* anchors;
    };

public:
    explicit PaintLayer(DesignerItem* parent = nullptr);

public slots:
    void updateGeometry();

private:
    void paintMarginOffset(QPainter* painter, const AnchorData& data);
    void paintAnchor(QPainter* painter, const AnchorData& data);
    void paintFillAnchor(QPainter* painter, Control* control);
    void paintCenterAnchor(QPainter* painter, Control* control);
    void paintAnchors(QPainter* painter);
    void paintAnchorConnection(QPainter* painter);
    void paintHoverOutline(QPainter* painter);
    void paintGuidelines(QPainter* painter);
    void paintSelectionOutlines(QPainter* painter);
    void paintMultipleSelectionOutline(QPainter* painter);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    bool m_geometryUpdateScheduled;
    const QPixmap m_anchorPixmap;
};

#endif // PAINTLAYER_H
