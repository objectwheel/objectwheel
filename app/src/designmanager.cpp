#include <designmanager.h>
#include <designerscene.h>
#include <designerview.h>
#include <control.h>
#include <qmlpreviewer.h>

#include <QWidget>
#include <QList>
#include <QVBoxLayout>
#include <QTimer>

class DesignManagerPrivate
{
    public:
        DesignManagerPrivate(DesignManager* parent);
        ~DesignManagerPrivate();

    public:
        DesignManager* parent;
        QWidget* settleWidget = nullptr;
        QWidget puppetWidget;
        DesignerScene designerScene;
        DesignerView designerView;
        QmlPreviewer qmlPreviewer;
};

DesignManagerPrivate::DesignManagerPrivate(DesignManager* parent)
    : parent(parent)
    , puppetWidget(settleWidget)
    , designerScene(-300, -300, 600, 600)
    , designerView(&designerScene, &puppetWidget)
{
    auto layout = new QVBoxLayout(&puppetWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(&designerView);

    // TODO: Find out a better solution to catch control previews
    QmlPreviewer::setPuppetWidget(&puppetWidget);

    designerView.setRenderHint(QPainter::Antialiasing);
    designerView.setRubberBandSelectionMode(Qt::IntersectsItemShape);
    designerView.setDragMode(QGraphicsView::RubberBandDrag);
    designerView.setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    designerView.setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    designerView.setBackgroundBrush(QColor("#e0e4e7"));

    QTimer::singleShot(3000, [this] {
        auto item = new Item;
        item->setId("eben");
        item->setUrl(QUrl("file:///Users/omergoktas/Projeler/Git/objectwheel/build/macos-x86_64-debug/Objectwheel.app/Contents/MacOS/data/4168122e4024a427b612af60ad3620c1/4e65772050726f6a6563742d31/tools/DragonFire/main.qml"));
        item->setPos(0, 0);
        designerScene.addItem(item);
    });


}

DesignManagerPrivate::~DesignManagerPrivate()
{
    /* delete stuff */
}

DesignManagerPrivate* DesignManager::_d = nullptr;

DesignManager::DesignManager(QObject *parent)
    : QObject(parent)
{
    if (_d) return;
    _d = new DesignManagerPrivate(this);
}

DesignManager* DesignManager::instance()
{
    return _d->parent;
}

DesignManager::~DesignManager()
{
    delete _d;
}

void DesignManager::setSettleWidget(QWidget* widget)
{
    _d->settleWidget = widget;
    _d->puppetWidget.setParent(_d->settleWidget);
}

void DesignManager::showWidget()
{
    _d->puppetWidget.show();
}

void DesignManager::hideWidget()
{
    _d->puppetWidget.hide();
}
