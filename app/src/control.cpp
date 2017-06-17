#include <control.h>
#include <delayer.h>

#include <QDebug>
#include <QTimer>
#include <QPainter>
#include <QApplication>
#include <QQuickWidget>
#include <QQmlComponent>
#include <QQmlIncubator>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QSharedPointer>
#include <QPointer>

//! ********************** [Control] **********************

class ControlPrivate
{
    public:
        ControlPrivate(Control* parent);

    public:
        Control* parent;
        QPixmap itemPixmap;
        QRectF itemRect;
};


ControlPrivate::ControlPrivate(Control* parent)
    : parent(parent)
{
}

QPointer<QWidget> Control::_puppetWidget = nullptr;

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

        if (!child)
            continue;
        else if (id.isEmpty())
            foundChilds << child;
        else if (child->id() == id)
            foundChilds << child;

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
    QTimer::singleShot(500, [this] { refresh(); }); //TODO: Improve this
}

QWidget* Control::puppetWidget()
{
    return _puppetWidget;
}

void Control::setPuppetWidget(QWidget* puppetWidget)
{
    _puppetWidget = puppetWidget;
}

void Control::refresh()
{
    if (!_url.isValid() || !_puppetWidget) return;

    QSize grabSize;
    QQuickItem* item;
    QSharedPointer<QQuickItemGrabResult> grabResult;
    QQuickWidget* quickWidget(new QQuickWidget(_puppetWidget));

    quickWidget->setSource(_url);
    quickWidget->show();
    quickWidget->lower();

    item = quickWidget->rootObject();
    _d->itemRect = QRectF(item->x(), item->y(), item->width(), item->height());
    grabSize = (QSizeF(item->width(), item->height()) * qApp->devicePixelRatio()).toSize();
    grabResult = item->grabToImage(grabSize);

    item->setVisible(false);

    QObject::connect(grabResult.data(), &QQuickItemGrabResult::ready, [=] {
        QPixmap pixmap = QPixmap::fromImage(grabResult->image());
        pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
        _d->itemPixmap = pixmap;
        quickWidget->deleteLater();
        update();
    });
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
