#include <controlsscrollpanel.h>
#include <fit.h>

#include <QGraphicsGridLayout>
#include <QPainter>

using namespace Fit;

#define SCROLLBAR_LENGTH (fit(8))
#define ID_LABEL_LENGTH (fit(8))

//!
//! ********************** [ControlsScrollBar] **********************
//!

ControlsScrollBar::ControlsScrollBar(QGraphicsWidget* parent)
    : QGraphicsWidget(parent)
{

}

Qt::Orientation ControlsScrollBar::orientation() const
{
    return _orientation;
}

void ControlsScrollBar::setOrientation(const Qt::Orientation& orientation)
{
    _orientation = orientation;
}

void ControlsScrollBar::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->fillRect(rect(), Qt::blue);

}

//!
//! ***************** [ControlsScrollPanelPrivate] ******************
//!

class ControlsScrollPanelPrivate : public QObject
{
        Q_OBJECT

    public:
        ControlsScrollPanelPrivate(ControlsScrollPanel* parent);
        void updateViewport();
        void setupLayout();

    public:
        ControlsScrollPanel* parent;
};


ControlsScrollPanelPrivate::ControlsScrollPanelPrivate(ControlsScrollPanel* parent)
    : QObject(parent)
    , parent(parent)
{

}

void ControlsScrollPanelPrivate::updateViewport()
{
    qreal totalHeight = parent->_margins.top() + parent->_margins.bottom();
    qreal totalWidth = parent->_margins.left() + parent->_margins.right();

    for (auto control : parent->_controls) {
        totalHeight += control->size().height();
        totalWidth += control->size().width();
    }

    if (parent->_spacing > 0 && parent->_controls.size() > 0) {
        if (parent->_orientation == Qt::Vertical)
            totalHeight += (parent->_spacing * (parent->_controls.size() - 1));
        else
            totalWidth += (parent->_spacing * (parent->_controls.size() - 1));
    }

    if (parent->_showIds && parent->_controls.size() > 0) {
        if (parent->_orientation == Qt::Vertical)
            totalHeight += (ID_LABEL_LENGTH * parent->_controls.size());
        else
            totalWidth += (ID_LABEL_LENGTH * parent->_controls.size());
    }

    if (parent->_verticalScrollBarPolicy == Qt::ScrollBarAlwaysOn) {
        totalWidth = totalWidth > (parent->size().width() - SCROLLBAR_LENGTH)
                     ? totalWidth : (parent->size().width() - SCROLLBAR_LENGTH);
        parent->_verticalScrollBar.show();
    } else if (parent->_verticalScrollBarPolicy == Qt::ScrollBarAsNeeded) {
        if (totalWidth > parent->size().width())
            parent->_verticalScrollBar.show();
        else
            parent->_verticalScrollBar.hide();

        totalWidth = totalWidth > parent->size().width()
                     ? totalWidth : parent->size().width();
    } else {
        parent->_verticalScrollBar.hide();
        totalWidth = totalWidth > parent->size().width()
                     ? totalWidth : parent->size().width();
    }

    parent->_viewport.setGeometry(0, 0, totalWidth, totalHeight);
    parent->_verticalScrollBar.setGeometry(parent->size().width() - SCROLLBAR_LENGTH, 0,
                                           SCROLLBAR_LENGTH, parent->size().height() - SCROLLBAR_LENGTH);
    parent->_horizontalScrollBar.setGeometry(0, parent->size().height() - SCROLLBAR_LENGTH,
                                           parent->size().width() - SCROLLBAR_LENGTH, SCROLLBAR_LENGTH);
}

//!
//! ********************* [ControlsScrollPanel] *********************
//!

ControlsScrollPanel::ControlsScrollPanel(QGraphicsWidget *parent)
    : QGraphicsWidget(parent)
    , _d(new ControlsScrollPanelPrivate(this))
    , _viewport(this)
    , _orientation(Qt::Vertical)
    , _alignment(Qt::AlignLeft)
    , _spacing(fit(5))
    , _horizontalScrollBarPolicy(Qt::ScrollBarAsNeeded)
    , _verticalScrollBarPolicy(Qt::ScrollBarAsNeeded)
    , _showIds(false)
    , _horizontalScrollBar(this)
    , _verticalScrollBar(this)

{
    _horizontalScrollBar.setOrientation(Qt::Horizontal);
    _verticalScrollBar.setOrientation(Qt::Vertical);
    _d->updateViewport();
}

Qt::Orientation ControlsScrollPanel::orientation() const
{
    return _orientation;
}

QList<Control*> ControlsScrollPanel::controls() const
{
    return _controls;
}

void ControlsScrollPanel::setOrientation(const Qt::Orientation& orientation)
{
    _orientation = orientation;
    _d->updateViewport();
}

void ControlsScrollPanel::addControl(Control* control)
{
    _controls << control;
    control->setParentItem(&_viewport);
    _d->updateViewport();
}

void ControlsScrollPanel::removeControl(Control* control)
{
    _controls.removeOne(control);
    control->setParentItem(0); //FIXME
    _d->updateViewport();
}

QMarginsF ControlsScrollPanel::margins() const
{
    return _margins;
}

void ControlsScrollPanel::setMargins(const QMarginsF& margins)
{
    _margins = margins;
    _d->updateViewport();
}

int ControlsScrollPanel::spacing() const
{
    return _spacing;
}

void ControlsScrollPanel::setSpacing(int spacing)
{
    _spacing = spacing;
    _d->updateViewport();
}

Qt::ScrollBarPolicy ControlsScrollPanel::horizontalScrollBarPolicy() const
{
    return _horizontalScrollBarPolicy;
}

void ControlsScrollPanel::setHorizontalScrollBarPolicy(const Qt::ScrollBarPolicy& horizontalScrollBarPolicy)
{
    _horizontalScrollBarPolicy = horizontalScrollBarPolicy;
    _d->updateViewport();
}

Qt::ScrollBarPolicy ControlsScrollPanel::verticalScrollBarPolicy() const
{
    return _verticalScrollBarPolicy;
}

void ControlsScrollPanel::setVerticalScrollBarPolicy(const Qt::ScrollBarPolicy& verticalScrollBarPolicy)
{
    _verticalScrollBarPolicy = verticalScrollBarPolicy;
    _d->updateViewport();
}

bool ControlsScrollPanel::showIds() const
{
    return _showIds;
}

void ControlsScrollPanel::setShowIds(bool showIds)
{
    _showIds = showIds;
    _d->updateViewport();
}

void ControlsScrollPanel::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    _d->updateViewport();
    QGraphicsWidget::resizeEvent(event);
}

void ControlsScrollPanel::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->fillRect(rect(), Qt::red);
}

#include "controlsscrollpanel.moc"
