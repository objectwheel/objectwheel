#include <designerscene.h>
#include <fit.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

using namespace Fit;

class DesignerScenePrivate : public QObject
{
        Q_OBJECT
        enum LockWay { NoLock, Vertical, Horizontal, Both };

    public:
        explicit DesignerScenePrivate(DesignerScene* parent);

    public:
        DesignerScene* parent;
        bool itemPressed;
        bool itemMoving;
};

DesignerScenePrivate::DesignerScenePrivate(DesignerScene* parent)
    : QObject(parent)
    , parent(parent)
    , itemPressed(false)
    , itemMoving(false)
{
}

DesignerScenePrivate* DesignerScene::_d = nullptr;
QList<Page*> DesignerScene::_pages;
Page* DesignerScene::_currentPage = nullptr;
bool DesignerScene::_snapping = true;
QPointF DesignerScene::_lastMousePos;

DesignerScene::DesignerScene(QObject *parent)
    : QGraphicsScene(parent)
{
    if (_d)
        return;
    _d = new DesignerScenePrivate(this);
}

const QList<Page*>& DesignerScene::pages()
{
    return _pages;
}

void DesignerScene::addPage(Page* page)
{
    if (_pages.contains(page))
        return;

    _d->parent->addItem(page);
    page->setVisible(false);
    page->setZValue(-1);

    _pages.append(page);

    if (!_currentPage)
        setCurrentPage(page);
}

void DesignerScene::removePage(Page* page)
{
    if (!_pages.contains(page))
        return;

    _d->parent->removeItem(page);
    _pages.removeOne(page);

    if (_currentPage == page) {
        if (_pages.size() > 0)
            setCurrentPage(_pages[0]);
        else
            _currentPage = nullptr;
    }
}

Page* DesignerScene::currentPage()
{
    return _currentPage;
}

void DesignerScene::setCurrentPage(Page* currentPage)
{
    if (!_pages.contains(currentPage) || _currentPage == currentPage)
        return;

    if (_currentPage)
        _currentPage->setVisible(false);

    _currentPage = currentPage;
    _currentPage->setVisible(true);
}

QList<Control*> DesignerScene::controls(Qt::SortOrder order)
{
    QList<Control*> controls;
    for (auto item : _d->parent->items(order)) {
        if (dynamic_cast<Control*>(item) && !dynamic_cast<Page*>(item)) {
            controls << static_cast<Control*>(item);
        }
    }
    return controls;
}

QList<Control*> DesignerScene::selectedControls()
{
    QList<Control*> selectedControls;
    for (auto item : _d->parent->selectedItems()) {
        if (dynamic_cast<Control*>(item) && !dynamic_cast<Page*>(item)) {
            selectedControls << static_cast<Control*>(item);
        }
    }
    return selectedControls;
}

void DesignerScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    const auto&  selectedControls = this->selectedControls();
    if (selectedControls.size() > 0 && _currentPage) {
        for (auto control : _currentPage->childControls()) {
            if (selectedControls.contains(control))
                control->setZValue(1); //BUG
            else
                control->setZValue(0);
        }
    }

    auto itemUnderMouse = itemAt(event->scenePos(), QTransform());
    if (this->selectedControls().contains((Control*)itemUnderMouse))
        _d->itemPressed = true;

    _d->itemMoving = false;

    for (auto control : currentPage()->childControls())
        control->setDragging(false);

    update();
}

void DesignerScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseMoveEvent(event);

    if (_currentPage && selectedControls().size() > 0 &&
        _d->itemPressed && !Resizer::resizing()) {
        _d->itemMoving = true;
        if (_snapping)
            _currentPage->stickSelectedControlToGuideLines();
    }

    if (_d->itemMoving) {
        for (auto selectedControl : selectedControls()) {
            selectedControl->setDragging(true);
        }
    }

    _lastMousePos = event->scenePos();

    update();
}

void DesignerScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseReleaseEvent(event);
    _d->itemPressed = false;
    _d->itemMoving = false;

    for (auto control : currentPage()->childControls())
        control->setDragging(false);

    update();
}

void DesignerScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsScene::drawForeground(painter, rect);

    if ((_d->itemMoving || Resizer::resizing()) && _snapping) {
        auto guideLines = _currentPage->guideLines();
        QPen pen("#DB4C41");
        pen.setWidthF(fit(1.0));
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);
        painter->setBrush(pen.color());
        painter->drawLines(guideLines);

        for (QLineF line : guideLines) {
            pen.setStyle(Qt::SolidLine);
            painter->setPen(pen);
            painter->drawRoundedRect(QRectF(line.p1() - QPointF(fit(2.0), fit(2.0)), QSizeF(fit(4.0), fit(4.0))), fit(2.0), fit(2.0));
            painter->drawRoundedRect(QRectF(line.p2() - QPointF(fit(2.0), fit(2.0)), QSizeF(fit(4.0), fit(4.0))), fit(2.0), fit(2.0));
        }
    }
}

QPointF DesignerScene::lastMousePos()
{
    return _lastMousePos;
}

bool DesignerScene::snapping()
{
    return _snapping;
}

void DesignerScene::setSnapping(bool snapping)
{
    _snapping = snapping;
}

bool DesignerScene::showOutlines()
{
    return Control::showOutline();;
}

void DesignerScene::setShowOutlines(bool value)
{
    Control::setShowOutline(value);
    for (auto control : controls()) {
        control->update();
    }
}

#include "designerscene.moc"

