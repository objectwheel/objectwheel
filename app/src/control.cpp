#include <control.h>
#include <delayer.h>
#include <fit.h>
#include <qmlpreviewer.h>

#include <QDebug>
#include <QTimer>
#include <QtMath>
#include <QPen>
#include <QQuickWindow>
#include <QWidget>
#include <QPainter>
#include <QBitmap>
#include <QVector>
#include <QMimeData>
#include <QGraphicsScene>
#include <QApplication>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QQmlComponent>
#include <QQmlIncubator>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QSharedPointer>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>

#define TOOLBOX_ITEM_KEY "QURBUEFaQVJMSVlJWiBIQUZJWg"
#define RESIZER_SIZE (fit(6.0))
#define HIGHLIGHT_COLOR ("#20404447")
#define SELECTION_COLOR ("#444444")
#define OUTLINE_COLOR (Qt::gray)
#define RESIZER_COLOR (Qt::white)
#define RESIZER_OUTLINE_COLOR ("#252525")
#define PREVIEW_REFRESH_INTERVAL 100

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
    painter->setPen(RESIZER_OUTLINE_COLOR);
    painter->setBrush(RESIZER_COLOR);
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

class ControlPrivate : public QObject
{
        Q_OBJECT

    public:
        ControlPrivate(Control* parent);
        void fixResizerCoordinates();
        void hideResizers();
        void showResizers();

    public slots:
        void refreshPreview();
        void updatePreview(const QPixmap& preview);

    public:
        Control* parent;
        QPixmap itemPixmap;
        Resizer resizers[8];
        QTimer refreshTimer;
        QmlPreviewer qmlPreviewer;
        bool dragIn = false;
};


ControlPrivate::ControlPrivate(Control* parent)
    : QObject(parent)
    , parent(parent)
{
    int i = 0;
    for (auto& resizer : resizers) {
        resizer.setParentItem(parent);
        resizer.setPlacement(Resizer::Placement(i++));
    }

    refreshTimer.setInterval(PREVIEW_REFRESH_INTERVAL);
    QObject::connect(&refreshTimer, SIGNAL(timeout()), SLOT(refreshPreview()));
    QObject::connect(&qmlPreviewer, SIGNAL(previewReady(QPixmap)), SLOT(updatePreview(QPixmap)));
}

void ControlPrivate::fixResizerCoordinates()
{
    for (auto& resizer : resizers) {
        switch (resizer.placement()) {
            case Resizer::Top:
                resizer.setPos(parent->size().width() / 2.0 - RESIZER_SIZE / 2.0,
                               - RESIZER_SIZE / 2.0 + 0.5);
                break;

            case Resizer::Right:
                resizer.setPos(parent->size().width() - RESIZER_SIZE / 2.0 - 0.5,
                               parent->size().height() / 2.0 - RESIZER_SIZE / 2.0);
                break;

            case Resizer::Bottom:
                resizer.setPos(parent->size().width() / 2.0 - RESIZER_SIZE / 2.0,
                               parent->size().height() - RESIZER_SIZE / 2.0 - 0.5);
                break;

            case Resizer::Left:
                resizer.setPos(- RESIZER_SIZE / 2.0 + 0.5,
                               parent->size().height() / 2.0 - RESIZER_SIZE / 2.0);
                break;

            case Resizer::TopLeft:
                resizer.setPos(- RESIZER_SIZE / 2.0 + 0.5, - RESIZER_SIZE / 2.0 + 0.5);
                break;

            case Resizer::TopRight:
                resizer.setPos(parent->size().width() - RESIZER_SIZE / 2.0 - 0.5,
                               - RESIZER_SIZE / 2.0 + 0.5);
                break;

            case Resizer::BottomRight:
                resizer.setPos(parent->size().width() - RESIZER_SIZE / 2.0 - 0.5,
                               parent->size().height() - RESIZER_SIZE / 2.0 - 0.5);
                break;

            case Resizer::BottomLeft:
                resizer.setPos( - RESIZER_SIZE / 2.0 + 0.5,
                                parent->size().height() - RESIZER_SIZE / 2.0 - 0.5);
                break;
        }
        resizer.setZValue(1);
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

void ControlPrivate::refreshPreview()
{
    refreshTimer.stop();
    if (!parent->url().isValid()) return;

    QSizeF size;
    if (!parent->size().isNull())
        size = parent->size();

    qmlPreviewer.requestReview(parent->url(), size);
}

void ControlPrivate::updatePreview(const QPixmap& preview)
{
    itemPixmap = preview;
    parent->resize(preview.size() / qApp->devicePixelRatio());
    parent->update();
}

//!
//! ********************** [Control] **********************
//!

Control::Control(Control* parent)
    : QGraphicsWidget(parent)
    , _d(new ControlPrivate(this))
    , _showOutline(true)
{
    setFlag(ItemIsFocusable);
    setFlag(ItemIsSelectable);
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    setGeometry(0, 0, 0, 0);
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
}

void Control::refresh()
{
    _d->refreshTimer.start();
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
    _d->dragIn = false;
    event->accept();
    update();
}

void Control::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mousePressEvent(event);
    event->accept();
}

void Control::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mouseMoveEvent(event);
    event->accept();
    setCursor(Qt::SizeAllCursor);
}

void Control::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mouseReleaseEvent(event);
    event->accept();
    setCursor(Qt::ArrowCursor);
}

void Control::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);
    _d->fixResizerCoordinates();
    refresh();
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
    return QGraphicsWidget::itemChange(change, value);
}

void Control::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (_d->itemPixmap.isNull()) return;
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPixmap(rect().adjusted(0.5, 0.5, -0.5, -0.5), _d->itemPixmap, _d->itemPixmap.rect());

    if (_d->dragIn) {
        if (_showOutline) {
            painter->fillRect(rect().adjusted(0.5, 0.5, -0.5, -0.5), HIGHLIGHT_COLOR);
        } else {
            QPixmap highlight(_d->itemPixmap);
            QPainter p(&highlight);
            p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            p.fillRect(_d->itemPixmap.rect(), HIGHLIGHT_COLOR);
            p.end();
            painter->drawPixmap(rect().adjusted(0.5, 0.5, -0.5, -0.5), highlight, _d->itemPixmap.rect());
        }
    }

    if (isSelected() || _showOutline) {
        QPen pen;
        pen.setStyle(Qt::DotLine);
        pen.setJoinStyle(Qt::MiterJoin);
        painter->setBrush(Qt::transparent);

        if (isSelected()) {
            pen.setColor(SELECTION_COLOR);
        } else if (_showOutline) {
            pen.setColor(OUTLINE_COLOR);
        }

        painter->setPen(pen);
        painter->drawRect(rect().adjusted(0.5, 0.5, -0.5, -0.5));
    }
}

//! ********************** [Page] **********************


class PagePrivate : public QObject
{
        Q_OBJECT
    public:
        explicit PagePrivate(Page* parent);
        QVector<QLineF> centerGuidelines(const QPointF& center) const;

    public:
        Page* parent;

};

PagePrivate::PagePrivate(Page* parent)
    : QObject(parent)
    , parent(parent)
{
}

QVector<QLineF> PagePrivate::centerGuidelines(const QPointF& center) const
{
//    parent->items();

}

bool Page::mainPage() const
{
    return _mainPage;
}

void Page::setMainPage(bool mainPage)
{
    _mainPage = mainPage;
}

void Page::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    //    if (selectedItems().size() > 0) {
    //        painter->setRenderHint(QPainter::Antialiasing);
    //        painter->drawLines(_d->centerGuidelines(selectedItems().at(0)->pos()
    //                          + selectedItems().at(0)->boundingRect().center()));
    //    } else {
    //        DesignerScene::drawForeground(painter, rect);
    //    }
}
#include "control.moc"
