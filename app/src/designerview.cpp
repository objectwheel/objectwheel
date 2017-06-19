#include <designerview.h>
#include <control.h>
#include <QTimer>

DesignerView::DesignerView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
{
    QTimer::singleShot(2000, [this] {
    auto item = new Item;
    item->setId("eben");
    item->setUrl(QUrl("file:///Users/omergoktas/Projeler/Git/objectwheel/build/macos-x86_64-debug/Objectwheel.app/Contents/MacOS/data/4168122e4024a427b612af60ad3620c1/4e65772050726f6a6563742d31/tools/DragonFire/main.qml"));
    item->setPos(0, 0);
    this->scene()->addItem(item);
    });
}
