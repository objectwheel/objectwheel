#ifndef PAINTLAYER_H
#define PAINTLAYER_H

#include <designeritem.h>
#include <anchorline.h>

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
    };

public:
    explicit PaintLayer(DesignerItem* parent = nullptr);

public slots:
    void updateGeometry();

private:
    void paintAnchor(QPainter* painter, const AnchorData& data);
    void paintAnchors(QPainter* painter);
    void paintAnchorConnector(QPainter* painter);
    void paintHoverOutline(QPainter* painter);
    void paintGuidelines(QPainter* painter);
    void paintSelectionOutlines(QPainter* painter);
    void paintMovingSelectionOutline(QPainter* painter);
    void paintLabelOverLine(QPainter* painter, const QString& label, const QLineF& line);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    bool m_geometryUpdateScheduled;
};

#endif // PAINTLAYER_H
