#include <designmanager.h>
#include <designerscene.h>
#include <designerview.h>
#include <control.h>
#include <qmlpreviewer.h>
#include <fit.h>

#include <QWidget>
#include <QList>
#include <QVBoxLayout>
#include <QTimer>

using namespace Fit;

class DesignManagerPrivate
{
    public:
        DesignManagerPrivate(DesignManager* parent);
        ~DesignManagerPrivate();

    public:
        DesignManager* parent;
        QWidget* settleWidget = nullptr;
        QVBoxLayout layout;
        DesignerScene designerScene;
        DesignerView designerView;
        QmlPreviewer qmlPreviewer;
};

DesignManagerPrivate::DesignManagerPrivate(DesignManager* parent)
    : parent(parent)
    , designerView(&designerScene)
{
    layout.setContentsMargins(0, 0, 0, 0);
    layout.setSpacing(0);
    layout.addWidget(&designerView);

    designerView.setRenderHint(QPainter::Antialiasing);
    designerView.setRubberBandSelectionMode(Qt::IntersectsItemShape);
    designerView.setDragMode(QGraphicsView::RubberBandDrag);
    designerView.setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    designerView.setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    designerView.setBackgroundBrush(QColor("#e0e4e7"));
    designerView.setFrameShape(QFrame::NoFrame);

    designerScene.setSceneRect(designerView.rect().adjusted(- designerView.width() / 2.0, - designerView.height() / 2.0, 0, 0));

    QTimer::singleShot(3000, [this] {
        auto page = new Page;
        page->setId("applicationWindow");
        page->setUrl(QUrl("qrc:/resources/qmls/mainPage.qml"));
        designerScene.addPage(page);
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
    if (_d->settleWidget)
        _d->settleWidget->setLayout(&_d->layout);
}

void DesignManager::showWidget()
{
    _d->designerView.show();
}

void DesignManager::hideWidget()
{
    _d->designerView.hide();
}
