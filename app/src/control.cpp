#include <control.h>
#include <delayer.h>
#include <fit.h>

#include <QDebug>
#include <QTimer>
#include <QtMath>
#include <QPen>
#include <QPainter>
#include <QBitmap>
#include <QMimeData>
#include <QGraphicsScene>
#include <QApplication>
#include <QQuickWidget>
#include <QQmlComponent>
#include <QQmlIncubator>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QSharedPointer>
#include <QPointer>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>

#define TOOLBOX_ITEM_KEY "QURBUEFaQVJMSVlJWiBIQUZJWg"
#define RESIZER_SIZE (fit(6.0))

using namespace Fit;

//!
//! ********************** [Resizer] **********************
//!

class Resizer : public QGraphicsItem
{
    public:
        enum Placement {
            Top,
            Right,
            Bottom,
            Left,
            TopLeft,
            TopRight,
            BottomRight,
            BottomLeft
        };

        explicit Resizer(Control *parent = Q_NULLPTR);
        virtual ~Resizer() {}

        Placement placement() const;
        void setPlacement(const Placement& placement);

    protected:
        virtual QRectF boundingRect() const override;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    private:
        Placement _placement;
};

Resizer::Resizer(Control *parent)
    : QGraphicsItem(parent)
    , _placement(Top)
{
    setVisible(false);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
}

QRectF Resizer::boundingRect() const
{
    return QRectF(0, 0, RESIZER_SIZE, RESIZER_SIZE);
}

void Resizer::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QColor("#252525"));
    painter->setBrush(Qt::white);
    painter->drawRoundedRect(boundingRect().adjusted(0.5, 0.5, -0.5, -0.5), RESIZER_SIZE / 4.0, RESIZER_SIZE / 4.0);
}

void Resizer::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mousePressEvent(event);
    event->accept();
}

void Resizer::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);

    qreal diff_x, diff_y;
    auto* parent = static_cast<Control*>(parentItem());
    auto startSize = parent->size();
    switch (_placement) {
        case Top:
            diff_y = event->lastPos().y() - event->pos().y();
            parent->setGeometry(parent->geometry().adjusted(0, -diff_y, 0, 0));
            break;

        case Right:
            diff_x = event->pos().x() - event->lastPos().x();
            parent->setGeometry(parent->geometry().adjusted(0, 0, diff_x, 0));
            break;

        case Bottom:
            diff_y = event->pos().y() - event->lastPos().y();
            parent->setGeometry(parent->geometry().adjusted(0, 0, 0, diff_y));
            break;

        case Left:
            diff_x = event->lastPos().x() - event->pos().x();
            parent->setGeometry(parent->geometry().adjusted(-diff_x, 0, 0, 0));
            break;

        case TopLeft:
            diff_x = event->lastPos().x() - event->pos().x();
            diff_y = event->lastPos().y() - event->pos().y();
            parent->setGeometry(parent->geometry().adjusted(-diff_x, -diff_y, 0, 0));
            break;

        case TopRight:
            diff_x = event->pos().x() - event->lastPos().x();
            diff_y = event->lastPos().y() - event->pos().y();
            parent->setGeometry(parent->geometry().adjusted(0, -diff_y, diff_x, 0));
            break;

        case BottomRight:
            diff_x = event->pos().x() - event->lastPos().x();
            diff_y = event->pos().y() - event->lastPos().y();
            parent->setGeometry(parent->geometry().adjusted(0, 0, diff_x, diff_y));
            break;

        case BottomLeft:
            diff_x = event->lastPos().x() - event->pos().x();
            diff_y = event->pos().y() - event->lastPos().y();
            parent->setGeometry(parent->geometry().adjusted(-diff_x, 0, 0, diff_y));
            break;
    }

    if (parent->size().width() < RESIZER_SIZE ||
        parent->size().height() < RESIZER_SIZE) {
        parent->resize(startSize);
    }
}

void Resizer::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseReleaseEvent(event);
}

Resizer::Placement Resizer::placement() const
{
    return _placement;
}

void Resizer::setPlacement(const Resizer::Placement& placement)
{
    _placement = placement;

    switch (_placement) {
        case Resizer::Top:
            setCursor(Qt::SizeVerCursor);
            break;

        case Resizer::Right:
            setCursor(Qt::SizeHorCursor);
            break;

        case Resizer::Bottom:
            setCursor(Qt::SizeVerCursor);
            break;

        case Resizer::Left:
            setCursor(Qt::SizeHorCursor);
            break;

        case Resizer::TopLeft:
            setCursor(Qt::SizeFDiagCursor);
            break;

        case Resizer::TopRight:
            setCursor(Qt::SizeBDiagCursor);
            break;

        case Resizer::BottomRight:
            setCursor(Qt::SizeFDiagCursor);
            break;

        case Resizer::BottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            break;
    }
}

//!
//! ****************** [Control Private] ******************
//!

class ControlPrivate
{
    public:
        ControlPrivate(Control* parent);
        void scratchPixmapIfEmpty(QPixmap& pixmap);
        QPixmap highlightPixmap(const QPixmap& pixmap);
        void fixResizerCoordinates();
        void hideResizers();
        void showResizers();

    public:
        Control* parent;
        QPixmap itemPixmap;
        QRectF itemRect;
        Resizer resizers[8];
        bool dragIn = false;
};


ControlPrivate::ControlPrivate(Control* parent)
    : parent(parent)
{
    int i = 0;
    for (auto& resizer : resizers) {
        resizer.setParentItem(parent);
        resizer.setPlacement(Resizer::Placement(i++));
    }
}

void ControlPrivate::scratchPixmapIfEmpty(QPixmap& pixmap)
{
    // Check 10 pixels atleast that has alpha > 250
    int totalAlpha = 0;
    QImage img = pixmap.toImage();
    for (int i = 0; i < img.width(); i++) {
        for (int j = 0; j < img.height(); j++) {
            totalAlpha += qAlpha(img.pixel(i, j));
            if (totalAlpha > (250 * 10)) {
                return;
            }
        }
    }

    QBrush brush;
    QPainter painter(&pixmap);
    brush.setColor(Qt::lightGray);
    brush.setStyle(Qt::DiagCrossPattern);
    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);
    painter.drawRect(pixmap.rect());
}

QPixmap ControlPrivate::highlightPixmap(const QPixmap& pixmap)
{
    auto buff = QPixmap(pixmap.size()).toImage();
    auto mask = pixmap.mask().toImage();
    for (int i = 0; i < mask.width(); i++) {
        for (int j = 0; j < mask.height(); j++) {
            buff.setPixel(i, j, !mask.pixelColor(i, j).red() ?
                              QColor("#70ffffff").rgba() : QColor("#00000000").rgba());
        }
    }
    return QPixmap::fromImage(buff);
}

void ControlPrivate::fixResizerCoordinates()
{
    for (auto& resizer : resizers) {
        switch (resizer.placement()) {
            case Resizer::Top:
                resizer.setPos(parent->size().width() / 2.0 - RESIZER_SIZE / 2.0, 0);
                break;

            case Resizer::Right:
                resizer.setPos(parent->size().width() - RESIZER_SIZE,
                               parent->size().height() / 2.0 - RESIZER_SIZE / 2.0);
                break;

            case Resizer::Bottom:
                resizer.setPos(parent->size().width() / 2.0 - RESIZER_SIZE / 2.0,
                               parent->size().height() - RESIZER_SIZE);
                break;

            case Resizer::Left:
                resizer.setPos(0, parent->size().height() / 2.0 - RESIZER_SIZE / 2.0);
                break;

            case Resizer::TopLeft:
                resizer.setPos(0, 0);
                break;

            case Resizer::TopRight:
                resizer.setPos(parent->size().width() - RESIZER_SIZE, 0);
                break;

            case Resizer::BottomRight:
                resizer.setPos(parent->size().width() - RESIZER_SIZE,
                               parent->size().height() - RESIZER_SIZE);
                break;

            case Resizer::BottomLeft:
                resizer.setPos(0, parent->size().height() - RESIZER_SIZE);
                break;
        }
    }
}

void ControlPrivate::hideResizers()
{
    for (auto& resizer : resizers) {
        resizer.hide();
    }
}

void ControlPrivate::showResizers()
{
    for (auto& resizer : resizers) {
        resizer.show();
    }
}

//!
//! ********************** [Control] **********************
//!

QPointer<QWidget> Control::_puppetWidget = nullptr;

Control::Control(Control* parent)
    : QGraphicsItem(parent)
    , _d(new ControlPrivate(this))
    , _showOutline(true)
{
    setFlag(ItemIsFocusable);
    setFlag(ItemIsSelectable);
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setFlag(ItemClipsToShape);
    setFlag(ItemClipsChildrenToShape);
    setFlag(ItemSendsGeometryChanges);
    setAcceptDrops(true);
    setAcceptHoverEvents(true);
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

            if (!child)
                continue;
            else if (id.isEmpty())
                foundChilds << child;
            else if (child->id() == id)
                foundChilds << child;
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

QRectF Control::geometry() const
{
    return QRectF(pos(), _size);
}

void Control::setGeometry(const QRectF& geometry)
{
    setPos(geometry.topLeft());
    resize(geometry.size());
}

QSizeF Control::size() const
{
    return _size;
}

void Control::resize(const QSizeF& size)
{
    prepareGeometryChange();
    _size = size;
    _d->fixResizerCoordinates();
}

QRectF Control::boundingRect() const
{
    return QRectF(QPointF(0, 0), _size);
}

bool Control::showOutline() const
{
    return _showOutline;
}

void Control::setShowOutline(const bool value)
{
    _showOutline = value;
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
    QTimer::singleShot(1000, [this] { refresh(); }); //TODO: Improve this
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

    QSizeF grabSize;
    QQuickItem* item;
    QSharedPointer<QQuickItemGrabResult> grabResult;
    QQuickWidget* quickWidget(new QQuickWidget(_puppetWidget));

    quickWidget->setSource(_url);
    quickWidget->show();
    quickWidget->lower();

    if (!quickWidget->errors().isEmpty()) {
        quickWidget->deleteLater();
        return;
    }

    item = quickWidget->rootObject();
    _d->itemRect = QRectF(RESIZER_SIZE, RESIZER_SIZE, item->width(), item->height());
    grabSize = _d->itemRect.size() * qApp->devicePixelRatio();
    grabSize = QSize(qCeil(grabSize.width()), qCeil(grabSize.height()));
    grabResult = item->grabToImage(grabSize.toSize());

    item->setVisible(false);
    resize(_d->itemRect.adjusted(-RESIZER_SIZE, -RESIZER_SIZE, RESIZER_SIZE, RESIZER_SIZE).size());

    QObject::connect(grabResult.data(), &QQuickItemGrabResult::ready, [=] {
        QPixmap pixmap = QPixmap::fromImage(grabResult->image());
        pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
        _d->itemPixmap = pixmap;
        _d->scratchPixmapIfEmpty(_d->itemPixmap);
        quickWidget->deleteLater();
        update();
    });
}

void Control::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    auto mimeData = event->mimeData();
    if (mimeData->hasUrls() && mimeData->hasText() &&
        mimeData->text() == TOOLBOX_ITEM_KEY) {
        _d->dragIn = true;
        event->accept();
    } else {
        event->ignore();
    }
    update();
}

void Control::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    _d->dragIn = false;
    event->accept();
    update();
}

void Control::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    event->accept();
}

void Control::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    auto item = new Item;
    item->setId("eben");
    item->setUrl(event->mimeData()->urls().at(0));
    item->setParentItem(this);
    item->setPos(event->pos());
    scene()->addItem(item);
    event->accept();
}

void Control::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mousePressEvent(event);
    event->accept();
}

void Control::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
    event->accept();
    setCursor(Qt::SizeAllCursor);
}

void Control::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    event->accept();
    setCursor(Qt::ArrowCursor);
}

QVariant Control::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    switch (change) {
        case ItemSelectedHasChanged:
            if (value.toBool()) {
                _d->showResizers();
            } else {
                _d->hideResizers();
            }
            break;
        default:
            break;
    }
    return QGraphicsItem::itemChange(change, value);
}

void Control::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (_d->itemPixmap.isNull()) return;
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPixmap(_d->itemRect, _d->itemPixmap, _d->itemPixmap.rect());
    if (_d->dragIn) {
        painter->fillRect(boundingRect().adjusted(RESIZER_SIZE / 2.0 + 0.5, RESIZER_SIZE / 2.0 + 0.5,
                                                  -RESIZER_SIZE / 2.0 - 0.5, -RESIZER_SIZE / 2.0 - 0.5),
                          QColor("#100088ff"));
    }

    { // Draw selection and outline
        QPen pen;
        QRectF selectionRect;
        pen.setStyle(Qt::DotLine);
        pen.setJoinStyle(Qt::MiterJoin);
        painter->setBrush(Qt::transparent);

        if (isSelected()) {
            pen.setColor("#444444");
        } else if (_showOutline) {
            pen.setColor(Qt::darkGray);
        }

        selectionRect = QRectF(RESIZER_SIZE / 2.0, RESIZER_SIZE / 2.0,
                               _size.width() - RESIZER_SIZE,
                               _size.height() - RESIZER_SIZE);
        painter->setPen(pen);
        painter->drawRect(selectionRect);
    }
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

