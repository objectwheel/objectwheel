#include <control.h>

#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QQuickWidget>
#include <QQmlComponent>
#include <QQmlIncubator>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QSharedPointer>

//! ********************** [Control] **********************

class ControlPrivate
{
    public:
        ControlPrivate(Control* parent);

    public:
        Control* _parent;
        QPixmap itemPixmap;
        QRectF itemRect;
};

ControlPrivate::ControlPrivate(Control* parent)
    : _parent(parent)
{
}

Control::Control(Control* parent)
    : QGraphicsItem(parent)
    , _d(new ControlPrivate(this))
{
}

Control::~Control()
{
    delete _d;
}

QList<Control*> Control::findChildren(const QString& id, Qt::FindChildOptions option) const
{
    QList<Control*> foundChilds;
    if (option == Qt::FindChildrenRecursively) {
        foundChilds = findChildrenRecursively(id, childItems());
    } else {
        for (auto c : childItems()) {
            auto child = dynamic_cast<Control*>(c);
            if (!child) continue;
            if (id.isEmpty()) {
                foundChilds << child;
            } else if (child->id() == id) {
                foundChilds << child;
            }
        }
    }
    return foundChilds;
}

QList<Control*> Control::findChildrenRecursively(const QString& id, QList<QGraphicsItem*> parent) const
{
    QList<Control*> foundChilds;
    for (auto c : parent) {
        auto child = dynamic_cast<Control*>(c);
        if (!child) continue;
        if (id.isEmpty()) {
            foundChilds << child;
        } else if (child->id() == id) {
            foundChilds << child;
        }
        foundChilds << findChildrenRecursively(id, child->childItems());
    }
    return foundChilds;
}

QString Control::id() const
{
    return _id;
}

void Control::setId(const QString& id)
{
    _id = id;
}

QUrl Control::url() const
{
    return _url;
}

void Control::setUrl(const QUrl& url)
{
    _url = url;
    refresh();
}
#include <delayer.h>
void Control::refresh()
{
    if (!_url.isValid()) return;
    QSharedPointer<QQuickWidget> quickWidget(new QQuickWidget);
//    quickWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
    quickWidget->setAttribute(Qt::WA_TranslucentBackground);
    quickWidget->setSource(_url);
    auto item = quickWidget->rootObject();
    _d->itemRect = QRectF(item->x(), item->y(), item->width(), item->height());
    auto pixmap = QPixmap(quickWidget->grab());
    pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
    _d->itemPixmap = pixmap;
    pixmap.save("/users/omergoktas/desktop/anan.png");
    update();
}

QRectF Control::boundingRect() const
{
    return _d->itemRect.adjusted(-3, -3, 3, 3);
}

void Control::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    if (_d->itemPixmap.isNull()) return;
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPixmap(boundingRect().adjusted(3, 3, -3, -3), _d->itemPixmap, QRectF(_d->itemPixmap.rect()));
}

//! ********************** [Page] **********************

bool Page::isMain() const
{
    return _main;
}

void Page::setMain(const bool main)
{
    _main = main;
}
