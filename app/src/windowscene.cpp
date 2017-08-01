#include <windowscene.h>
#include <fit.h>
#include <savemanager.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

#define GUIDELINE_COLOR ("#0D80E7")

using namespace Fit;

class WindowScenePrivate : public QObject
{
        Q_OBJECT
        enum LockWay { NoLock, Vertical, Horizontal, Both };

    public:
        explicit WindowScenePrivate(WindowScene* parent);

    public:
        WindowScene* parent;
        bool itemPressed;
        bool itemMoving;
};

WindowScenePrivate::WindowScenePrivate(WindowScene* parent)
    : QObject(parent)
    , parent(parent)
    , itemPressed(false)
    , itemMoving(false)
{
}

WindowScenePrivate* WindowScene::_d = nullptr;
QList<Page*> WindowScene::_pages;
Page* WindowScene::_currentPage = nullptr;
bool WindowScene::_snapping = true;
QPointF WindowScene::_lastMousePos;

WindowScene::WindowScene(QObject *parent)
    : QGraphicsScene(parent)
{
    if (_d)
        return;
    _d = new WindowScenePrivate(this);

    connect(this, &WindowScene::changed, [=] {
        setSceneRect(currentPage()->frameGeometry().adjusted(-fit(8), -fit(8), fit(8), fit(8)));
    });
}

WindowScene* WindowScene::instance()
{
    return _d->parent;
}

const QList<Page*>& WindowScene::pages()
{
    return _pages;
}

void WindowScene::addPage(Page* page)
{
    if (_pages.contains(page))
        return;

    _d->parent->addItem(page);
    page->setVisible(false);

    _pages.append(page);

    if (!_currentPage)
        setCurrentPage(page);
}

void WindowScene::removePage(Page* page)
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

Page* WindowScene::currentPage()
{
    return _currentPage;
}

void WindowScene::setCurrentPage(Page* currentPage)
{
    if (!_pages.contains(currentPage) || _currentPage == currentPage)
        return;

    if (_currentPage)
        _currentPage->setVisible(false);

    _currentPage = currentPage;
    _currentPage->setVisible(true);
}

void WindowScene::removeControl(Control* control)
{
    SaveManager::removeSave(control->id());
    WindowScene::instance()->removeItem(control);
}

void WindowScene::removeChildControlsOnly(Control* parent)
{
    SaveManager::removeChildSavesOnly(parent->id());

    for (auto control : parent->childControls())
        WindowScene::instance()->removeItem(control);
}

QList<Control*> WindowScene::controls(Qt::SortOrder order)
{
    QList<Control*> controls;
    for (auto item : _d->parent->items(order)) {
        if (dynamic_cast<Control*>(item) && !dynamic_cast<Page*>(item)) {
            controls << static_cast<Control*>(item);
        }
    }
    return controls;
}

QList<Control*> WindowScene::selectedControls()
{
    QList<Control*> selectedControls;
    for (auto item : _d->parent->selectedItems()) {
        if (dynamic_cast<Control*>(item) && !dynamic_cast<Page*>(item)) {
            selectedControls << static_cast<Control*>(item);
        }
    }
    return selectedControls;
}

void WindowScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    auto selectedControls = this->selectedControls();
    for (auto control : selectedControls)
        if (_currentPage->higherZValue() != control->zValue())
            control->setZValue(_currentPage->higherZValue() == -MAX_Z_VALUE
                               ? 0 : _currentPage->higherZValue() + 1);

    auto itemUnderMouse = itemAt(event->scenePos(), QTransform());
    if (this->selectedControls().contains((Control*)itemUnderMouse))
        _d->itemPressed = true;

    _d->itemMoving = false;

    for (auto control : currentPage()->childControls())
        control->setDragging(false);

    update();
}

void WindowScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseMoveEvent(event);

    if (_currentPage && selectedControls().size() > 0 &&
        _d->itemPressed && !Resizer::resizing()) {
        _d->itemMoving = true;
        if (_snapping && selectedControls().size() == 1)
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

void WindowScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseReleaseEvent(event);
    _d->itemPressed = false;
    _d->itemMoving = false;

    for (auto control : currentPage()->childControls())
        control->setDragging(false);

    update();
}

void WindowScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsScene::drawForeground(painter, rect);

    if ((_d->itemMoving || Resizer::resizing()) && _snapping) {
        auto guideLines = _currentPage->guideLines();
        QPen pen(GUIDELINE_COLOR);
        pen.setWidthF(1.0);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);
        painter->setBrush(pen.color());
        painter->drawLines(guideLines);

        for (QLineF line : guideLines) {
            pen.setStyle(Qt::SolidLine);
            painter->setPen(pen);
            painter->drawRoundedRect(QRectF(line.p1() - QPointF(1.0, 1.0), QSizeF(2.0, 2.0)), 1.0, 1.0);
            painter->drawRoundedRect(QRectF(line.p2() - QPointF(1.0, 1.0), QSizeF(2.0, 2.0)), 1.0, 1.0);
        }
    }
}

QPointF WindowScene::lastMousePos()
{
    return _lastMousePos;
}

bool WindowScene::snapping()
{
    return _snapping;
}

void WindowScene::setSnapping(bool snapping)
{
    _snapping = snapping;
}

bool WindowScene::showOutlines()
{
    return Control::showOutline();;
}

void WindowScene::setShowOutlines(bool value)
{
    Control::setShowOutline(value);
    for (auto control : controls()) {
        control->update();
    }
}

#include "windowscene.moc"

