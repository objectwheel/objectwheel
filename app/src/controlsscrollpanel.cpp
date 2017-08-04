#include <controlsscrollpanel.h>
#include <fit.h>

#include <QGraphicsGridLayout>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneWheelEvent>

using namespace Fit;

#define SCROLLBAR_LENGTH (fit(6))
#define ID_LABEL_LENGTH (fit(6))
#define STYLE_SHEET "\
QScrollBar { \
    background: transparent; \
    width: %2px; \
} QScrollBar::handle { \
    background: rgba(122,125,127); \
    min-height: %1px; \
    border-radius: %3px; \
} QScrollBar::add-line { \
    background: none; \
} QScrollBar::sub-line { \
    background: none; \
} QScrollBar::up-arrow, QScrollBar::down-arrow { \
    background: none; \
} QScrollBar::add-page, QScrollBar::sub-page { \
    background: none; \
}"

//!
//! ***************** [ControlsScrollPanelPrivate] ******************
//!

class ControlsScrollPanelPrivate : public QObject
{
        Q_OBJECT

    public:
        ControlsScrollPanelPrivate(ControlsScrollPanel* parent);
        QList<qreal> findXs(const QList<Control*>& controls,
                            Qt::Orientation orientation,
                            const QMarginsF& margins,
                            qreal spacing);
        QList<qreal> findYs(const QList<Control*>& controls,
                            Qt::Orientation orientation,
                            const QMarginsF& margins,
                            qreal spacing,
                            bool showIds);
    public slots:
        void updateViewport();

    public:
        ControlsScrollPanel* parent;
};


ControlsScrollPanelPrivate::ControlsScrollPanelPrivate(ControlsScrollPanel* parent)
    : QObject(parent)
    , parent(parent)
{

}

QList<qreal> ControlsScrollPanelPrivate::findXs(const QList<Control*>& controls,
                                                Qt::Orientation orientation,
                                                const QMarginsF& margins,
                                                qreal spacing)
{
    QList<qreal> xS;
    for (int i = 0; i < controls.size(); i++) {
        switch (orientation) {
            case Qt::Vertical: {
                xS << margins.left();
                break;
            }
            case Qt::Horizontal: {
                qreal prevLengths = 0;
                for (int j = 0; j < i; j++)
                    prevLengths += controls[j]->size().width();
                xS << (margins.left() + prevLengths + i * spacing);
                break;
            }
        }
    }
    return xS;
}

QList<qreal> ControlsScrollPanelPrivate::findYs(const QList<Control*>& controls,
                                                Qt::Orientation orientation,
                                                const QMarginsF& margins,
                                                qreal spacing,
                                                bool showIds)
{
    QList<qreal> yS;
    for (int i = 0; i < controls.size(); i++) {
        switch (orientation) {
            case Qt::Vertical: {
                qreal prevLengths = 0;
                for (int j = 0; j < i; j++)
                    prevLengths += controls[j]->size().height();
                yS << (margins.top() + prevLengths + (showIds ? i * ID_LABEL_LENGTH : 0) + i * spacing);
                break;
            }
            case Qt::Horizontal: {
                yS << margins.top();
                break;
            }
        }
    }
    return yS;
}

void ControlsScrollPanelPrivate::updateViewport()
{
    if (parent->controls().size() < 1) {
        parent->_horizontalScrollBar.hide();
        parent->_verticalScrollBar.hide();
        parent->hide();
        return;
    } else {
        parent->show();
    }

    qreal totalHeight = parent->_margins.top() + parent->_margins.bottom();
    qreal totalWidth = parent->_margins.left() + parent->_margins.right();

    for (auto control : parent->_controls) {
        if (parent->_orientation == Qt::Vertical)
            totalHeight += control->size().height();
        else
            totalWidth += control->size().width();
    }

    if (parent->_orientation == Qt::Vertical) {
        qreal maxWidth = 0;
        for (auto control : parent->_controls)
            if (control->size().width() > maxWidth)
                maxWidth = control->size().width();
        totalWidth += maxWidth;
    } else {
        qreal maxHeight = 0;
        for (auto control : parent->_controls)
            if (control->size().height() > maxHeight)
                maxHeight = control->size().height();
        totalHeight += maxHeight;
    }

    if (parent->_orientation == Qt::Vertical) {
        totalHeight += (parent->controls().size() - 1) * parent->_spacing;
        totalHeight += parent->_showIds ? parent->controls().size() * ID_LABEL_LENGTH : 0;
    } else {
        totalWidth += (parent->controls().size() - 1) * parent->_spacing;
        totalWidth += parent->_showIds ? parent->controls().size() * ID_LABEL_LENGTH : 0;
    }

    if (parent->_verticalScrollBarPolicy == Qt::ScrollBarAlwaysOn) {
        totalHeight = totalHeight > (parent->size().height() - SCROLLBAR_LENGTH)
                     ? totalHeight : (parent->size().height() - SCROLLBAR_LENGTH);
        parent->_verticalScrollBar.show();
    } else if (parent->_verticalScrollBarPolicy == Qt::ScrollBarAsNeeded) {
        if (totalHeight > parent->size().height())
            parent->_verticalScrollBar.show();
        else
            parent->_verticalScrollBar.hide();

        totalHeight = totalHeight > parent->size().height()
                     ? totalHeight : parent->size().height();
    } else {
        parent->_verticalScrollBar.hide();
        totalHeight = totalHeight > parent->size().height()
                     ? totalHeight : parent->size().height();
    }

    if (parent->_horizontalScrollBarPolicy == Qt::ScrollBarAlwaysOn) {
        totalWidth = totalWidth > (parent->size().width() - SCROLLBAR_LENGTH)
                     ? totalWidth : (parent->size().width() - SCROLLBAR_LENGTH);
        parent->_horizontalScrollBar.show();
    } else if (parent->_horizontalScrollBarPolicy == Qt::ScrollBarAsNeeded) {
        if (totalWidth > parent->size().width())
            parent->_horizontalScrollBar.show();
        else
            parent->_horizontalScrollBar.hide();

        totalWidth = totalWidth > parent->size().width()
                     ? totalWidth : parent->size().width();
    } else {
        parent->_horizontalScrollBar.hide();
        totalWidth = totalWidth > parent->size().width()
                     ? totalWidth : parent->size().width();
    }

    parent->_viewport.setGeometry(0, 0, totalWidth, totalHeight);
    parent->_horizontalScrollBar.setGeometry(0, parent->size().height() - SCROLLBAR_LENGTH,
                                           parent->size().width(), SCROLLBAR_LENGTH);
    parent->_verticalScrollBar.setGeometry(parent->size().width() - SCROLLBAR_LENGTH, 0,
                                           SCROLLBAR_LENGTH, parent->size().height());

    parent->_horizontalScrollBar.setPageStep(parent->size().width() - SCROLLBAR_LENGTH);
    parent->_verticalScrollBar.setPageStep(parent->size().height() - SCROLLBAR_LENGTH);
    parent->_horizontalScrollBar.setMaximum(totalWidth - parent->_horizontalScrollBar.pageStep());
    parent->_verticalScrollBar.setMaximum(totalHeight - parent->_verticalScrollBar.pageStep());

    const QList<qreal>& xS = findXs(parent->_controls,
                                    parent->_orientation,
                                    parent->_margins,
                                    parent->_spacing);
    const QList<qreal>& yS = findYs(parent->_controls,
                                    parent->_orientation,
                                    parent->_margins,
                                    parent->_spacing,
                                    parent->_showIds);

    for (int i = 0; i < parent->_controls.size(); i++)
        parent->_controls[i]->setPos(xS[i], yS[i]);

    parent->update();
}

//!
//! ********************* [ControlsScrollPanel] *********************
//!

ControlsScrollPanel::ControlsScrollPanel(QGraphicsScene* scene, QGraphicsWidget *parent)
    : QGraphicsWidget(parent)
    , _d(new ControlsScrollPanelPrivate(this))
    , _viewport(this)
    , _orientation(Qt::Vertical)
    , _margins(fit(3), fit(3), fit(3), fit(3))
    , _spacing(fit(6))
    , _horizontalScrollBarPolicy(Qt::ScrollBarAsNeeded)
    , _verticalScrollBarPolicy(Qt::ScrollBarAsNeeded)
    , _showIds(false)
{
    scene->addItem(this);
    setFlag(ItemClipsToShape);
    setFlag(ItemClipsChildrenToShape);
    _horizontalScrollBar.setOrientation(Qt::Horizontal);
    _verticalScrollBar.setOrientation(Qt::Vertical);

    QString styleSheet(STYLE_SHEET);
    styleSheet = styleSheet.arg(SCROLLBAR_LENGTH).arg(SCROLLBAR_LENGTH).arg(fit(3));
    _verticalScrollBar.setStyleSheet(styleSheet);
    _horizontalScrollBar.setStyleSheet(styleSheet);

    auto hBarProxy = scene->addWidget(&_horizontalScrollBar);
    auto vBarProxy = scene->addWidget(&_verticalScrollBar);
    hBarProxy->setParentItem(this);
    vBarProxy->setParentItem(this);
    connect(&_horizontalScrollBar, SIGNAL(valueChanged(int)), SLOT(handleHBarValueChange(int)));
    connect(&_verticalScrollBar, SIGNAL(valueChanged(int)), SLOT(handleVBarValueChange(int)));
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
    connect(control, SIGNAL(geometryChanged()), _d, SLOT(updateViewport()));
    _d->updateViewport();
}

void ControlsScrollPanel::removeControl(Control* control)
{
    _controls.removeOne(control);
    control->setParentItem(0); //FIXME
    control->disconnect(_d);
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

qreal ControlsScrollPanel::spacing() const
{
    return _spacing;
}

void ControlsScrollPanel::setSpacing(qreal spacing)
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

void ControlsScrollPanel::handleHBarValueChange(int value)
{
    _viewport.setX(-value);
}

void ControlsScrollPanel::handleVBarValueChange(int value)
{
    _viewport.setY(-value);
}

void ControlsScrollPanel::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    _d->updateViewport();
    QGraphicsWidget::resizeEvent(event);
}

void ControlsScrollPanel::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (_orientation == Qt::Vertical) {
        QLinearGradient gradient(0, rect().center().y(), size().width(), rect().center().y());
        gradient.setColorAt(0, "#DEE2E5");
        gradient.setColorAt(1, "#D7DADD");
        painter->setBrush(gradient);
        painter->setPen(QColor("#9FA2A5"));
        painter->drawRoundedRect(rect().adjusted(0.5, 0.5, -0.5, -0.5), fit(3), fit(3));

        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(size().width() - (SCROLLBAR_LENGTH) + 0.5, 1.5,
                     (SCROLLBAR_LENGTH) / 2.0 - 2.0, size().height() - 2.5);
        path.addRoundedRect(size().width() - (SCROLLBAR_LENGTH) / 2.0 + 0.5 - fit(3), 1.5,
                            (SCROLLBAR_LENGTH) / 2.0 - 2.0 + fit(3), size().height() - 2.5, fit(3), fit(3));
        painter->setPen(QColor("#BEC2C5"));
        painter->setBrush(QColor("#BEC2C5"));
        painter->drawPath(path.simplified());

        painter->setPen(QColor("#9FA2A5"));
        painter->drawLine(QPointF(size().width() - (SCROLLBAR_LENGTH) + 0.5, 0.5),
                          QPointF(size().width() - (SCROLLBAR_LENGTH) + 0.5, size().height() - 1.0));
    } else {
        QLinearGradient gradient(rect().center().x(), 0, rect().center().x(), size().height());
        gradient.setColorAt(0, "#DEE2E5");
        gradient.setColorAt(1, "#D7DADD");
        painter->setBrush(gradient);
        painter->setPen(QColor("#9FA2A5"));
        painter->drawRoundedRect(rect().adjusted(0.5, 0.5, -0.5, -0.5), fit(3), fit(3));

        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(1.5, size().height() - (SCROLLBAR_LENGTH) + 0.5,
                     size().width() - 2.5, (SCROLLBAR_LENGTH) / 2.0 - 2.0);
        path.addRoundedRect(1.5, size().height() - (SCROLLBAR_LENGTH) / 2.0 + 0.5 - fit(3),
                            size().width() - 2.5, (SCROLLBAR_LENGTH) / 2.0 - 2.0 + fit(3), fit(3), fit(3));
        painter->setPen(QColor("#BEC2C5"));
        painter->setBrush(QColor("#BEC2C5"));
        painter->drawPath(path.simplified());

        painter->setPen(QColor("#9FA2A5"));
        painter->drawLine(QPointF(0.5, size().height() - (SCROLLBAR_LENGTH) + 0.5),
                          QPointF(size().width() - 1.0, size().height() - (SCROLLBAR_LENGTH) + 0.5));
    }
}

void ControlsScrollPanel::wheelEvent(QGraphicsSceneWheelEvent* event)
{
    if (event->orientation() == Qt::Vertical && _verticalScrollBar.isVisible())
        _verticalScrollBar.setValue(_verticalScrollBar.value() - event->delta() / 15.0);
    if (event->orientation() == Qt::Horizontal && _horizontalScrollBar.isVisible())
        _horizontalScrollBar.setValue(_horizontalScrollBar.value() - event->delta() / 15.0);
    QGraphicsWidget::wheelEvent(event);
}

#include "controlsscrollpanel.moc"
