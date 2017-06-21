#include <designerscene.h>

class DesignerScenePrivate : public QObject
{
        Q_OBJECT
    public:
        explicit DesignerScenePrivate(DesignerScene* parent);

    public:
        DesignerScene* parent;

};

DesignerScenePrivate::DesignerScenePrivate(DesignerScene* parent)
    : QObject(parent)
    , parent(parent)
{
}

DesignerScene::DesignerScene(qreal x, qreal y, qreal width, qreal height, QObject *parent)
    : QGraphicsScene(x, y, width, height, parent)
    , _d(new DesignerScenePrivate(this))
    , _currentPage(nullptr)
{
}

const QList<Page*>& DesignerScene::pages() const
{
    return _pages;
}

void DesignerScene::addPage(Page* page)
{
    if (_pages.contains(page))
        return;

    page->setVisible(false);
    addItem(page);

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

#include "designerscene.moc"

