#include <designerscene.h>
#include <fit.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

#define NO_SKIN_SIZE (fit(QSize(400, 400)))
#define PHONE_PORTRAIT_SIZE (fit(QSize(213, 379)))
#define PHONE_LANDSCAPE_SIZE (fit(QSize(379, 213)))
#define DESKTOP_SIZE (fit(QSize(500, 350)))

#define PHONE_PORTRAIT_FRAME_RECT (QRectF(-((fit(244) - PHONE_PORTRAIT_SIZE.width())/2.0), -((fit(500) - PHONE_PORTRAIT_SIZE.height())/2.0), fit(245), fit(500)))
#define PHONE_LANDSCAPE_FRAME_RECT (QRectF(-((fit(244) - PHONE_PORTRAIT_SIZE.width())/2.0), -((fit(500) - PHONE_PORTRAIT_SIZE.height())/2.0), fit(500), fit(245)))
#define DESKTOP_FRAME_RECT (QRectF(-((fit(244) - PHONE_PORTRAIT_SIZE.width())/2.0), -((fit(500) - PHONE_PORTRAIT_SIZE.height())/2.0), fit(245), fit(500)))

#define PHONE_PORTRAIT_SKIN_PATH (":/resources/images/phone.png")
#define PHONE_LANDSCAPE_SKIN_PATH (":/resources/images/phonel.png")
#define DESKTOP_SKIN_PATH (":/resources/images/phone.png")

using namespace Fit;

class DesignerScenePrivate : public QObject
{
        Q_OBJECT
        enum LockWay { NoLock, Vertical, Horizontal, Both };

    public:
        explicit DesignerScenePrivate(DesignerScene* parent);
        inline QSize skinSize(const DesignerScene::Skin& skin) const;
        inline Page::SkinSetting* skinSetting(const DesignerScene::Skin& skin) const;

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

QSize DesignerScenePrivate::skinSize(const DesignerScene::Skin& skin) const
{
    switch (skin) {
        case DesignerScene::NoSkin:
            return NO_SKIN_SIZE;
            break;

        case DesignerScene::PhonePortrait:
            return PHONE_PORTRAIT_SIZE;
            break;

        case DesignerScene::PhoneLandscape:
            return PHONE_LANDSCAPE_SIZE;
            break;

        case DesignerScene::Desktop:
            return DESKTOP_SIZE;
            break;
    }
}

Page::SkinSetting* DesignerScenePrivate::skinSetting(const DesignerScene::Skin& skin) const
{
    switch (skin) {
        case DesignerScene::NoSkin:
            return new Page::SkinSetting();
            break;

        case DesignerScene::PhonePortrait:
            return new Page::SkinSetting(QPixmap(PHONE_PORTRAIT_SKIN_PATH), PHONE_PORTRAIT_FRAME_RECT, false);
            break;

        case DesignerScene::PhoneLandscape:
            return new Page::SkinSetting(QPixmap(PHONE_LANDSCAPE_SKIN_PATH), PHONE_LANDSCAPE_FRAME_RECT, false);
            break;

        case DesignerScene::Desktop:
            return new Page::SkinSetting(QPixmap(DESKTOP_SKIN_PATH), DESKTOP_FRAME_RECT, true);
            break;
    }
}

DesignerScene::DesignerScene(QObject *parent)
    : QGraphicsScene(parent)
    , _d(new DesignerScenePrivate(this))
    , _currentPage(nullptr)
{
    setSkin(PhonePortrait);
}

const QList<Page*>& DesignerScene::pages() const
{
    return _pages;
}

void DesignerScene::addPage(Page* page)
{
    if (_pages.contains(page))
        return;

    addItem(page);
    page->setVisible(false);
    page->setZValue(-1);
    page->resize(_d->skinSize(skin()));

    _pages.append(page);

    if (!_currentPage)
        setCurrentPage(page);
}

void DesignerScene::removePage(Page* page)
{
    if (!_pages.contains(page))
        return;

    removeItem(page);
    _pages.removeOne(page);

    if (_currentPage == page) {
        if (_pages.size() > 0)
            setCurrentPage(_pages[0]);
        else
            _currentPage = nullptr;
    }
}

Page* DesignerScene::currentPage() const
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

QList<Control*> DesignerScene::controls(Qt::SortOrder order) const
{
    QList<Control*> controls;
    for (auto item : items(order)) {
        if (dynamic_cast<Control*>(item) && !dynamic_cast<Page*>(item)) {
            controls << static_cast<Control*>(item);
        }
    }
    return controls;
}

QList<Control*> DesignerScene::selectedControls() const
{
    QList<Control*> selectedControls;
    for (auto item : selectedItems()) {
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
        _currentPage->stickSelectedControlToGuideLines();
    }

    if (_d->itemMoving) {
        for (auto selectedControl : selectedControls()) {
            selectedControl->setDragging(true);
        }
    }

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

    if (_d->itemMoving || Resizer::resizing()) {
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

DesignerScene::Skin DesignerScene::skin() const
{
    return _skin;
}

void DesignerScene::setSkin(const Skin& skin)
{
    _skin = skin;
    if (Page::skinSetting())
        delete Page::skinSetting();
    Page::setSkinSetting(_d->skinSetting(skin));
    for (auto page : pages()) {
        page->setResizable(Page::skinSetting()->resizable);
        page->resize(_d->skinSize(skin));
        page->update();
    }
}

bool DesignerScene::showOutlines() const
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

