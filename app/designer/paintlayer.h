#ifndef PAINTLAYER_H
#define PAINTLAYER_H

#include <designeritem.h>

class PaintLayer final : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(PaintLayer)

public:
    explicit PaintLayer(DesignerItem* parent = nullptr);

public slots:
    void updateGeometry();

private:
    void paintGuidelines(QPainter* painter);
    void paintSelectionOutlines(QPainter* painter);
    void paintMovingSelectionOutline(QPainter* painter);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    bool m_geometryUpdateScheduled;
};

#endif // PAINTLAYER_H