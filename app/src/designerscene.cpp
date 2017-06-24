#include <designerscene.h>
#include <fit.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#define NO_SKIN_SIZE (QSize(400, 400))
#define PHONE_PORTRAIT_SIZE (QSize(256, 455))
#define PHONE_LANDSCAPE_SIZE (QSize(455, 256))
#define DESKTOP_SIZE (QSize(500, 350))

#define PHONE_PORTRAIT_FRAME_RECT (QRectF(-((293 - PHONE_PORTRAIT_SIZE.width())/2.0), -((600 - PHONE_PORTRAIT_SIZE.height())/2.0), 294, 600))
#define PHONE_LANDSCAPE_FRAME_RECT (QRectF(-((293 - PHONE_PORTRAIT_SIZE.width())/2.0), -((600 - PHONE_PORTRAIT_SIZE.height())/2.0), 294, 600))
#define DESKTOP_FRAME_RECT (QRectF(-((293 - PHONE_PORTRAIT_SIZE.width())/2.0), -((600 - PHONE_PORTRAIT_SIZE.height())/2.0), 294, 600))

#define PHONE_PORTRAIT_SKIN_PATH (":/resources/images/phone.png")
#define PHONE_LANDSCAPE_SKIN_PATH (":/resources/images/phone.png")
#define DESKTOP_SKIN_PATH (":/resources/images/phone.png")

using namespace Fit;

class DesignerScenePrivate : public QObject
{
        Q_OBJECT
    public:
        explicit DesignerScenePrivate(DesignerScene* parent);
        inline QSize skinSize(const DesignerScene::Skin& skin) const;
        inline Page::SkinSetting* skinSetting(const DesignerScene::Skin& skin) const;
        inline bool guideLineBlocks(QGraphicsSceneMouseEvent* event) const;

    public:
        DesignerScene* parent;
        bool moving;
};

DesignerScenePrivate::DesignerScenePrivate(DesignerScene* parent)
    : QObject(parent)
    , parent(parent)
    , moving(false)
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

bool DesignerScenePrivate::guideLineBlocks(QGraphicsSceneMouseEvent* event) const
{
    QPointF blockPos;
    static bool blocked = false;
    bool vertical = false, horizontal = false;
    auto guideLines = parent->_currentPage->guideLines();
    auto selectedControl = parent->selectedControls()[0];

    for (auto guideLine : guideLines) {
        if (guideLine.dx() > 0)
            vertical = true;
        if (guideLine.dy() > 0)
            horizontal = true;
        if (horizontal && vertical)
            break;
    }

    if (vertical && horizontal) {
        if (!blocked)
            blockPos = event->scenePos();
        blocked = true;

        if ((event->scenePos() - blockPos).manhattanLength() < 10)
            selectedControl->setPos(selectedControl->x(), selectedControl->y());
        else {
            blocked = false;
            return false;
        }
    } else if (vertical) {
        if (!blocked)
            blockPos = event->scenePos();
        blocked = true;

        if (qAbs(event->scenePos().y() - blockPos.y()) < 10)
            selectedControl->setPos(selectedControl->parentItem()->mapFromScene(event->scenePos()).x() -
                                    selectedControl->mapFromScene(event->lastScenePos()).x(),
                                    selectedControl->y());
        else {
            blocked = false;
            return false;
        }
    } else if (horizontal) {
        if (!blocked)
            blockPos = event->scenePos();
        blocked = true;

        if (qAbs(event->scenePos().x() - blockPos.x()) < 10)
            selectedControl->setPos(selectedControl->x(),
                                    selectedControl->parentItem()->mapFromScene(event->scenePos()).y() -
                                    selectedControl->mapFromScene(event->lastScenePos()).y());
        else {
            blocked = false;
            return false;
        }
    }

    if (vertical || horizontal)
        return true;

    blocked = false;
    return false;
}

DesignerScene::DesignerScene(qreal x, qreal y, qreal width, qreal height, QObject *parent)
    : QGraphicsScene(x, y, width, height, parent)
    , _d(new DesignerScenePrivate(this))
    , _currentPage(nullptr)
{
    setSkin(NoSkin);
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
        if (dynamic_cast<Control*>(item)) {
            controls << static_cast<Control*>(item);
        }
    }
    return controls;
}

QList<Control*> DesignerScene::selectedControls() const
{
    QList<Control*> selectedControls;
    for (auto item : selectedItems()) {
        if (dynamic_cast<Control*>(item)) {
            selectedControls << static_cast<Control*>(item);
        }
    }
    return selectedControls;
}

void DesignerScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    const auto&  sControls = selectedControls();
    if (sControls.size() > 0 && _currentPage &&
        !sControls.contains(_currentPage)) {
        for (auto control : _currentPage->childControls()) {
            if (sControls.contains(control))
                control->setZValue(1); //BUG
            else
                control->setZValue(0);
        }
    }

    _d->moving = false;
    update();
}
#include <QCursor>
void DesignerScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{

    if (_currentPage &&
        selectedControls().size() > 0 &&
        !_pages.contains((Page*)selectedControls()[0]) ) {
        _d->moving = true;

        if (_d->guideLineBlocks(event)) {
            event->accept();
            return;
        }
    }

    QGraphicsScene::mouseMoveEvent(event);
    update();
}

void DesignerScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseReleaseEvent(event);
    _d->moving = false;
    update();
}

void DesignerScene::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsScene::drawForeground(painter, rect);

    if (_d->moving) {
        painter->setPen(Qt::blue);
        painter->drawLines(_currentPage->guideLines());
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

