#include <form.h>
#include <resizer.h>
#include <saveutils.h>
#include <suppressor.h>
#include <designerscene.h>
#include <controlpreviewingmanager.h>
#include <windowmanager.h>
#include <mainwindow.h>
#include <centralwidget.h>
#include <designerwidget.h>
#include <controlpropertymanager.h>
#include <paintutils.h>
#include <parserutils.h>
#include <utilityfunctions.h>
#include <toolutils.h>

#include <QtMath>
#include <QCursor>
#include <QPainter>
#include <QMimeData>
#include <QRegularExpression>
#include <QGraphicsSceneDragDropEvent>
#include <QApplication>

extern const char* TOOL_KEY;

namespace {

const QSize g_baseControlSize(40, 40);

QList<Resizer*> initializeResizers(Control* control)
{
    QList<Resizer*> resizers;
    for (int i = 0; i < 8; ++i)
        resizers.append(new Resizer(control, Resizer::Placement(i)));
    return resizers;
}
}

QList<Control*> Control::m_controls;
Control::Control(const QString& dir, Control* parent) : QGraphicsWidget(parent)
  , m_gui(false)
  , m_clip(false)
  , m_popup(false)
  , m_window(false)
  , m_dragIn(false)
  , m_hoverOn(false)
  , m_dragging(false)
  , m_resizing(false)
  , m_dir(dir)
  , m_uid(SaveUtils::controlUid(m_dir))
  , m_pixmap(QPixmap::fromImage(PaintUtils::renderInitialControlImage(g_baseControlSize)))
  , m_resizers(initializeResizers(this))
{
    m_controls << this;

    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    setFlag(ItemIsMovable);
    setFlag(ItemIsFocusable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);

    ControlPropertyManager::setId(this, ParserUtils::id(m_dir), ControlPropertyManager::NoOption);
    ControlPropertyManager::setIndex(this, SaveUtils::controlIndex(m_dir), ControlPropertyManager::NoOption);
    ControlPropertyManager::setSize(this, g_baseControlSize, ControlPropertyManager::NoOption);

    connect(ControlPreviewingManager::instance(), &ControlPreviewingManager::previewDone,
            this, &Control::updatePreview);

    connect(this, &Control::resizingChanged,
            this, &Control::applyCachedGeometry);
    connect(this, &Control::draggingChanged,
            this, &Control::applyCachedGeometry);
}

Control::~Control()
{
    // In order to prevent any kind of signals being emitted while deletion in progress
    blockSignals(true);
    m_controls.removeOne(this);
}

bool Control::gui() const
{
    return m_gui;
}

bool Control::form() const
{
    return type() == Form::Type;
}

bool Control::clip() const
{
    return m_clip;
}

bool Control::popup() const
{
    return m_popup;
}

bool Control::window() const
{
    return m_window;
}

bool Control::dragIn() const
{
    return m_dragIn;
}

bool Control::dragging() const
{
    return m_dragging;
}

bool Control::resizing() const
{
    return m_resizing;
}

bool Control::hasErrors() const
{
    return !m_errors.isEmpty();
}

quint32 Control::index() const
{
    return m_index;
}

int Control::type() const
{
    return Type;
}

int Control::higherZValue() const
{
    int z = 0;
    for (const auto& control : childControls())
        if (control->zValue() > z)
            z = control->zValue();
    return z;
}

int Control::lowerZValue() const
{
    int z = 0;
    for (const auto& control : childControls())
        if (control->zValue() < z)
            z = control->zValue();
    return z;
}

QString Control::id() const
{
    return m_id;
}

QString Control::uid() const
{
    return m_uid;
}

QString Control::dir() const
{
    return m_dir;
}

QMarginsF Control::margins() const
{
    return m_margins;
}

DesignerScene* Control::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsWidget::scene());
}

Control* Control::parentControl() const
{
    if (parentItem() && (parentItem()->type() == Form::Type || parentItem()->type() == Control::Type))
        return static_cast<Control*>(parentItem());
    return nullptr;
}

QList<QmlError> Control::errors() const
{
    return m_errors;
}

QList<QString> Control::events() const
{
    return m_events;
}

QList<PropertyNode> Control::properties() const
{
    return m_properties;
}

QList<Control*> Control::siblings() const
{
    QList<Control*> siblings;
    if (form()) {
        if (const DesignerScene* scene = this->scene()) {
            for (Form* form : scene->forms())
                siblings.append(form);
        }
    } else {
        if (const Control* parent = parentControl())
            siblings = parent->childControls(false);
    }
    Q_ASSERT(siblings.contains(const_cast<Control*>(this)));
    siblings.removeOne(const_cast<Control*>(this));
    std::sort(siblings.begin(), siblings.end(), [] (const Control* left, const Control* right) {
        return left->index() < right->index();
    });
    return siblings;
}

QList<Control*> Control::childControls(bool recursive) const
{
    QList<Control*> controls;

    for (QGraphicsItem* item : childItems()) {
        if (item->type() == Form::Type || item->type() == Control::Type)
            controls.append(static_cast<Control*>(item));
    }

    std::sort(controls.begin(), controls.end(), [] (const Control* left, const Control* right) {
        return left->index() < right->index();
    });

    if (recursive) {
        const int SIBLINGS_COUNT = controls.size();
        for (int i = 0; i < SIBLINGS_COUNT; ++i)
            controls.append(controls.at(i)->childControls(true));
    }

    return controls;
}

QList<Control*> Control::controls()
{
    return m_controls;
}

void Control::setClip(bool clip)
{
    m_clip = clip;
}

void Control::setId(const QString& id)
{
    m_id = id;
    setToolTip(id);
}

void Control::setDir(const QString& dir)
{
    m_dir = dir;
}

void Control::setDragIn(bool dragIn)
{
    m_dragIn = dragIn;
}

void Control::setDragging(bool dragging)
{
    m_dragging = dragging;

    if (dragging)
        setCursor(Qt::SizeAllCursor);
    else
        setCursor(Qt::ArrowCursor);

    // FIXME:
    Suppressor::suppress(150, "draggingChanged", std::bind(&Control::draggingChanged, this));
    //    emit draggingChanged();
}

void Control::setResizing(bool resizing)
{
    m_resizing = resizing;
    // FIXME:
    Suppressor::suppress(150, "resizingChanged", std::bind(&Control::resizingChanged, this));
    //    emit resizingChanged();
}

void Control::setIndex(quint32 index)
{
    m_index = index;
}

void Control::hideResizers()
{
    for (auto resizer : m_resizers)
        resizer->hide();
}

void Control::showResizers()
{
    for (auto resizer : m_resizers)
        resizer->show();
}

QVariant::Type Control::propertyType(const QString& propertyName) const
{
    for (const PropertyNode& propertyNode : m_properties) {
        const QMap<QString, QVariant>& propertyMap = propertyNode.properties;
        for (const QString& property : propertyMap.keys()) {
            const QVariant& propertyValue = propertyMap.value(property);
            if (property == propertyName)
                return propertyValue.type();
        }
    }
    return QVariant::Invalid;
}

void Control::dropControl(Control* control)
{
    Q_ASSERT(!control->form());

    // NOTE: Do not move this assignment below setParent, because parent change effects the newPos result
    const QPointF& newPos = mapFromItem(control->parentItem(), control->pos());
    ControlPropertyManager::setParent(control, this, ControlPropertyManager::SaveChanges
                                      | ControlPropertyManager::UpdatePreviewer);
    ControlPropertyManager::setPos(control, newPos, ControlPropertyManager::SaveChanges
                                   | ControlPropertyManager::UpdatePreviewer
                                   | ControlPropertyManager::CompressedCall);
    // NOTE: We compress setPos because there might be some other compressed setPos'es in the list
    // We want the setPos that happens after reparent operation to take place at the very last

    update();
}

void Control::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    // FIXME: Move this out of Control.cpp. Move everything unrelated with a Control
    // to DesignerView or DesignerScene or somewhere else that is related to.
    m_dragIn = false;
    event->accept();
    WindowManager::mainWindow()->centralWidget()->designerWidget()->onControlDrop(
                this,
                event->mimeData()->urls().first().toLocalFile(),
                event->pos());
    update();
}

void Control::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    event->accept();
}

void Control::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    auto mimeData = event->mimeData();
    if (mimeData->hasUrls() && mimeData->hasText() &&
            mimeData->text() == TOOL_KEY) {
        m_dragIn = true;
        event->accept();
    } else {
        event->ignore();
    }
    update();
}

void Control::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    m_dragIn = false;
    event->accept();
    update();
}

void Control::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mouseMoveEvent(event);

    Control* control = nullptr;
    const QList<Control*>& controlsUnderCursor = scene()->controlsAt(event->scenePos());

    if (controlsUnderCursor.size() > 1)
        control = controlsUnderCursor.at(1);

    if (control && control != this && !control->dragIn() && m_dragging) {
        control->setDragIn(true);

        for (Control* childControls : scene()->currentForm()->childControls())
            if (childControls != control)
                childControls->setDragIn(false);

        if (scene()->currentForm() != control)
            scene()->currentForm()->setDragIn(false);
    }

    if (event->button() == Qt::MidButton)
        event->ignore();
    else
        event->accept();

    if (!form()) {
        ControlPropertyManager::setPos(this, pos(), ControlPropertyManager::SaveChanges
                                       | ControlPropertyManager::UpdatePreviewer
                                       | ControlPropertyManager::CompressedCall);
    }
}

void Control::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mousePressEvent(event);

    if (event->button() == Qt::MidButton)
        event->ignore();
    else
        event->accept();
}

void Control::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsWidget::mouseReleaseEvent(event);

    auto selectedControls = scene()->selectedControls();
    selectedControls.removeOne(scene()->currentForm());

    for (auto control : scene()->currentForm()->childControls()) {
        if (control->dragIn() && dragging() && parentControl() != control) {
            for (auto sc : selectedControls) {
                if (sc->dragging())
                    control->dropControl(sc);
            }
            scene()->clearSelection();
            control->setSelected(true);
        }
        control->setDragIn(false);
    }

    if (scene()->currentForm()->dragIn() && dragging() &&
            parentControl() != scene()->currentForm()) {
        scene()->currentForm()->dropControl(this);
        scene()->clearSelection();
        scene()->currentForm()->setSelected(true);
    }
    scene()->currentForm()->setDragIn(false);

    event->accept();
}

void Control::mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)
{
    WindowManager::mainWindow()->centralWidget()->designerWidget()->onControlDoubleClick(this);
}

void Control::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsWidget::hoverEnterEvent(event);
    m_hoverOn = true;
    update();
}

void Control::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsWidget::hoverLeaveEvent(event);
    m_hoverOn = false;
    update();
}

QVariant Control::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    switch (change) {
    case ItemSelectedHasChanged:
        if (value.toBool())
            showResizers();
        else
            hideResizers();
        break;
    default:
        break;
    }
    return QGraphicsWidget::itemChange(change, value);
}

void Control::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);

    for (auto resizer : m_resizers)
        resizer->correct();
}

void Control::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (m_pixmap.isNull())
        return;

    if (parentControl() && parentControl()->clip() && !m_dragging) {
        painter->setClipRect(
                    rect().intersected(
                        parentControl()->mapToItem(
                            this,
                            parentControl()->rect().adjusted(1, 1, -1, -1)
                            ).boundingRect()
                        )
                    );
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPixmap(rect(), m_pixmap, QRectF(QPointF(0, 0), size() * qApp->devicePixelRatio()));

    QLinearGradient gradient(rect().center().x(), rect().y(), rect().center().x(), rect().bottom());
    gradient.setColorAt(0, QColor("#174C4C4C").lighter(110));
    gradient.setColorAt(1, QColor("#174C4C4C").darker(120));

    if (m_dragIn) {
        if (scene()->showOutlines()) {
            painter->fillRect(rect(), gradient);
        } else {
            QPixmap highlight(m_pixmap);
            QPainter p(&highlight);
            p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            p.fillRect(m_pixmap.rect(), gradient);
            p.end();
            painter->drawPixmap(rect(), highlight, QRectF(QPointF(0, 0), size() * qApp->devicePixelRatio()));
        }
    }

    if (!form() && (isSelected() || scene()->showOutlines())) {
        QPen pen;
        pen.setStyle(Qt::DotLine);
        painter->setBrush(Qt::transparent);
        painter->setClipping(false);

        if (isSelected()) {
            pen.setColor(Qt::black);
        } else if (scene()->showOutlines()) {
            if (m_hoverOn) {
                pen.setStyle(Qt::SolidLine);
                pen.setColor("#4BA2FF");
            } else {
                pen.setColor("#777777");
            }
        }

        painter->setPen(pen);
        painter->drawRect(rect().adjusted(0.5, 0.5, -0.5, -0.5));
    }
}

void Control::updatePreview(const PreviewResult& result)
{
    if (result.uid != uid())
        return;

    // FIXME
    // result.id ?? Preview Engine bize yeni id değeri göndermeli mi, zaten id değişikliğini
    // önce biz ona gönderiyoruz, kod değişikliği yada propertis pane'den değiştirerek??
    // zaten her halükarda id bu tarafta değiştirilip orada değişik kod gitmiyormu ve bu kod
    // daki id değişikliği daha preview engine gitmeden önce zaten ilgili control'e setId edilmiyor mu?
    ControlPropertyManager::setId(this, result.id, ControlPropertyManager::NoOption);

    m_errors = result.errors;
    m_gui = result.gui;
    m_popup = result.popup;
    m_window = result.window;
    m_events = result.events;
    m_properties = result.properties;

    if (result.codeChanged)
        m_margins = UtilityFunctions::getMarginsFromProperties(result.properties);
    m_cachedGeometry = UtilityFunctions::getGeometryFromProperties(result.properties);

    if (!dragging() && !resizing())
        applyCachedGeometry();

    m_pixmap = QPixmap::fromImage(hasErrors() ? PaintUtils::renderErrorControlImage(size())
                                              : result.image);

    if (m_pixmap.isNull()) {
        if (m_gui) {
            m_pixmap = QPixmap::fromImage(PaintUtils::renderInvisibleControlImage(size()));
        } else {
            m_pixmap = QPixmap::fromImage(
                        PaintUtils::renderNonGuiControlImage(
                            ToolUtils::toolIcon(m_dir, qApp->devicePixelRatio()), size()));
        }
    }

    for (auto resizer : m_resizers)
        resizer->setDisabled(!gui());

    update();

    ControlPropertyManager::instance()->previewChanged(this, result.codeChanged);
}

void Control::applyCachedGeometry()
{
    if (!dragging() && !resizing()) {
        if (form()) {
            ControlPropertyManager::setSize(this, m_cachedGeometry.size(),
                                            ControlPropertyManager::NoOption);
        } else {
            ControlPropertyManager::setGeometry(
                        this, ControlPropertyManager::geoWithMargin(this, m_cachedGeometry, true),
                        ControlPropertyManager::NoOption);
        }
    }
}
