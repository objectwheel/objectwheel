#include <controlsscrollpanel.h>
#include <controlscene.h>
#include <fit.h>

#include <QGraphicsGridLayout>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneWheelEvent>

//TODO: Horizontal mode

using namespace Fit;

#define SCROLLBAR_LENGTH (fit(6))
#define ID_LABEL_LENGTH (fit(12))
#define FONT_PIXELSIZE (fit(11))
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
                            qreal spacing,
                            QGraphicsWidget* viewport);
        QList<qreal> findYs(const QList<Control*>& controls,
                            Qt::Orientation orientation,
                            const QMarginsF& margins,
                            qreal spacing,
                            bool showIds, QGraphicsWidget* viewport);
    public slots:
        void updateViewport();
        void fixCoords();
        void fixSelection();

    public:
        ControlsScrollPanel* parent;
        QList<QGraphicsSimpleTextItem*> idTexts;
};


ControlsScrollPanelPrivate::ControlsScrollPanelPrivate(ControlsScrollPanel* parent)
    : QObject(parent)
    , parent(parent)
{
}

QList<qreal> ControlsScrollPanelPrivate::findXs(const QList<Control*>& controls,
                                                Qt::Orientation orientation,
                                                const QMarginsF& margins,
                                                qreal spacing,
                                                QGraphicsWidget* viewport)
{
    QList<qreal> xS;
    for (int i = 0; i < controls.size(); i++) {
        switch (orientation) {
            case Qt::Vertical: {
                xS << viewport->mapToItem(controls[i]->parentControl(), margins.left(), 0).x();
                break;
            }
            case Qt::Horizontal: {
                qreal prevLengths = 0;
                for (int j = 0; j < i; j++)
                    prevLengths += controls[j]->size().width();
                xS <<   viewport->mapToItem(controls[i]->parentControl(),
                                            margins.left() + prevLengths + i * spacing, 0).x();
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
                                                bool showIds,
                                                QGraphicsWidget* viewport)
{
    QList<qreal> yS;
    for (int i = 0; i < controls.size(); i++) {
        switch (orientation) {
            case Qt::Vertical: {
                qreal prevLengths = 0;
                for (int j = 0; j < i; j++)
                    prevLengths += controls[j]->size().height();
                yS << viewport->mapToItem(controls[i]->parentControl(), 0,
                                          (margins.top() + prevLengths + (showIds ? i * ID_LABEL_LENGTH : 0) + i * spacing)).y();
                break;
            }
            case Qt::Horizontal: {
                yS << viewport->mapToItem(controls[i]->parentControl(), 0, margins.top()).y();
                break;
            }
        }
    }
    return yS;
}

void ControlsScrollPanelPrivate::updateViewport()
{
    if (parent->_controls.size() < 1) {
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
        totalHeight += (parent->_controls.size() - 1) * parent->_spacing;
        totalHeight += parent->_showIds ? parent->_controls.size() * ID_LABEL_LENGTH : 0;
    } else {
        totalWidth += (parent->_controls.size() - 1) * parent->_spacing;
        totalWidth += parent->_showIds ? parent->_controls.size() * ID_LABEL_LENGTH : 0;
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
                                    parent->_spacing,
                                    &parent->_viewport);
    const QList<qreal>& yS = findYs(parent->_controls,
                                    parent->_orientation,
                                    parent->_margins,
                                    parent->_spacing,
                                    parent->_showIds,
                                    &parent->_viewport);

    QFont font;
    font.setPixelSize(FONT_PIXELSIZE);
    for (int i = 0; i < parent->_controls.size(); i++) {
        auto control = parent->_controls[i];
        control->setPos(xS[i], yS[i]);
        if (parent->_showIds) {
            idTexts[i]->setFont(font);
            idTexts[i]->setText(control->id());
            idTexts[i]->setToolTip(control->id());
            auto vrect = parent->viewport()->mapFromItem(control, control->rect()).boundingRect();
            auto diff = parent->size().width() - SCROLLBAR_LENGTH - idTexts[i]->boundingRect().width();
            auto x = diff > 0 ? diff/2.0 : fit(1);
            idTexts[i]->setPos(x, vrect.bottom() + (ID_LABEL_LENGTH - FONT_PIXELSIZE));
        }
    }

    fixSelection();

    parent->update();
}

void ControlsScrollPanelPrivate::fixCoords()
{
    const QList<qreal>& yS = findYs(parent->_controls,
                                    parent->_orientation,
                                    parent->_margins,
                                    parent->_spacing,
                                    parent->_showIds,
                                    &parent->_viewport);

    for (int i = 0; i < parent->_controls.size(); i++) {
        auto control = parent->_controls[i];
        control->setPos(control->x(), yS[i]);

        auto vrect = parent->viewport()->mapFromItem(control, control->rect()).boundingRect();
        auto diff = parent->size().width() - SCROLLBAR_LENGTH - idTexts[i]->boundingRect().width();
        auto x = diff > 0 ? diff/2.0 : fit(1);
        idTexts[i]->setPos(x, vrect.bottom() + (ID_LABEL_LENGTH - FONT_PIXELSIZE));

        fixSelection();
        control->update();
    }
}

void ControlsScrollPanelPrivate::fixSelection()
{
    for (auto control : parent->_controls) {
        auto clipRect = control->rect().intersected(parent->
                         mapToItem(control, parent->rect().
                         adjusted(1, 1, -1, -1)).boundingRect());
        if (clipRect.isValid())
            control->show();
        else
            control->hide();

        if (!parent->rect().contains(parent->
            mapFromItem(control, control->rect()).boundingRect()))
            control->hideSelection();
        else
            control->showSelection();
    }
}

//!
//! ********************* [ControlsScrollPanel] *********************
//!

ControlsScrollPanel::ControlsScrollPanel(QGraphicsScene* scene, QGraphicsWidget *parent)
    : QGraphicsWidget(parent)
    , _d(new ControlsScrollPanelPrivate(this))
    , _viewport(this)
    , _orientation(Qt::Vertical)
    , _margins(fit(5), fit(5), fit(5), fit(5))
    , _spacing(fit(5))
    , _horizontalScrollBarPolicy(Qt::ScrollBarAsNeeded)
    , _verticalScrollBarPolicy(Qt::ScrollBarAsNeeded)
    , _showIds(false)
{
    scene->addItem(this);
    setFlag(ItemClipsToShape);
    setFlag(ItemClipsChildrenToShape);
    connect(&_viewport, SIGNAL(geometryChanged()), _d, SLOT(fixCoords()));
    connect(scene, SIGNAL(selectionChanged()), _d, SLOT(fixSelection()));
    connect(scene, SIGNAL(controlRemoved(Control*)), SLOT(removeControl(Control*)));

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
    connect(control, SIGNAL(initialized()), _d, SLOT(updateViewport()));
    if (_showIds)
        _d->idTexts << new QGraphicsSimpleTextItem(&_viewport);
    _d->updateViewport();
}

void ControlsScrollPanel::removeControl(Control* control)
{
    //FIXME: For form removal, child controls?
    int index = _controls.indexOf(control);
    if (index < 0)
        return;
    if (_showIds) {
        scene()->removeItem(_d->idTexts.at(index));
        delete _d->idTexts.at(index);
        _d->idTexts.removeAt(index);
    }
    _controls.removeOne(control);
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
    if (_showIds) {
        for (int i = _controls.size(); i--;)
            _d->idTexts << new QGraphicsSimpleTextItem(this);
    } else {
        for (auto textItem : _d->idTexts) {
            scene()->removeItem(textItem);
            delete textItem;
        }
        _d->idTexts.clear();
    }
    _d->updateViewport();
}

void ControlsScrollPanel::clear()
{
    for (auto control : _controls)
        removeControl(control);
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

QGraphicsWidget* ControlsScrollPanel::viewport()
{
    return &_viewport;
}

#include "controlsscrollpanel.moc"
