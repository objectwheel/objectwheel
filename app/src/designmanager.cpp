#include <designmanager.h>
#include <QWidget>
#include <QList>

QList<Control*> Control::findChildren(const QString& id, Qt::FindChildOptions option) const
{
    QList<Control*> foundChilds;
    if (option == Qt::FindChildrenRecursively) {
        foundChilds = findChildrenRecursively(id, children);
    } else {
        for (auto child : children) {
            if (child->id == id)
                foundChilds << child;
        }
    }
    return foundChilds;
}

QList<Control*> Control::findChildrenRecursively(const QString& id, QList<Control*> parent) const
{
    QList<Control*> foundChilds;
    for (auto child : parent) {
        if (child->id == id) {
            foundChilds << child;
        }
        foundChilds << findChildrenRecursively(id, child->children);
    }
    return foundChilds;
}

class DesignManagerPrivate {

    public: // variables
        DesignManager* parent;
        QWidget* settleWidget = nullptr;
        QWidget widget;
        QList<Page*> pages;
        QList<Item*> items;

    public: // functions
        DesignManagerPrivate(DesignManager* parent);
        ~DesignManagerPrivate();
};

DesignManagerPrivate::DesignManagerPrivate(DesignManager* parent) :
    parent(parent),
    widget(settleWidget)
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
    _d->widget.setParent(_d->settleWidget);
}

void DesignManager::showWidget()
{
    _d->widget.show();
}

void DesignManager::hideWidget()
{
    _d->widget.hide();
}
