#include <designmanager.h>
#include <designerscene.h>
#include <designerview.h>
#include <control.h>
#include <QWidget>
#include <QList>

class DesignManagerPrivate
{
    public:
        DesignManagerPrivate(DesignManager* parent);
        ~DesignManagerPrivate();

    public:
        DesignManager* parent;
        QWidget* settleWidget = nullptr;
        DesignerScene designerScene;
        DesignerView designerView;
        QList<Page*> pages;
};

DesignManagerPrivate::DesignManagerPrivate(DesignManager* parent) :
    parent(parent),
    designerView(&designerScene, settleWidget)
{
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
    _d->designerView.setParent(_d->settleWidget);
}

void DesignManager::showWidget()
{
    _d->designerView.show();
}

void DesignManager::hideWidget()
{
    _d->designerView.hide();
}
