#include <designmanager.h>
#include <designerscene.h>
#include <designerview.h>
#include <control.h>
#include <QWidget>
#include <QList>
#include <QVBoxLayout>

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
        QList<Page*> pages;
};

DesignManagerPrivate::DesignManagerPrivate(DesignManager* parent)
    : parent(parent)
    , puppetWidget(settleWidget)
    , designerView(&designerScene, &puppetWidget)
{
    auto layout = new QVBoxLayout(&puppetWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(&designerView);

    // TODO: Find out a better solution to catch control previews
    Control::setPuppetWidget(&puppetWidget);
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
