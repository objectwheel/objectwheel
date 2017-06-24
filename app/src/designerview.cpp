#include <designerview.h>
#include <designerscene.h>
#include <control.h>
#include <QTimer>

DesignerView::DesignerView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
{
    setMouseTracking(true);
}

void DesignerView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    auto _scene = static_cast<DesignerScene*>(scene());
    auto currentPage = _scene->currentPage();
    if (currentPage)
        currentPage->centralize();
}
