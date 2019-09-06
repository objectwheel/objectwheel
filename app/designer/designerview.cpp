#include <designerview.h>
#include <designerscene.h>
#include <utilsicons.h>
#include <controlcreationmanager.h>
#include <controlpropertymanager.h>
#include <controlremovingmanager.h>
#include <saveutils.h>
#include <transparentstyle.h>
#include <signalchooserdialog.h>
#include <utilityfunctions.h>
#include <qmlcodeeditorwidget.h>
#include <parserutils.h>
#include <projectmanager.h>
#include <qmlcodeeditor.h>
#include <qmlcodedocument.h>
#include <designersettings.h>
#include <scenesettings.h>
#include <anchoreditor.h>
#include <form.h>

#include <QScrollBar>
#include <QMenu>
#include <QDir>
#include <QToolBar>
#include <QToolButton>
#include <QPainter>
#include <QMessageBox>
#include <QComboBox>
#include <QInputDialog>

// 1. Control name; 2. Method name
#define METHOD_DECL "%1_%2"
#define FORM_DECL "%1_onCompleted"
#define METHOD_BODY \
    "\n\n"\
    "function %1() {\n"\
    "    // Do something...\n"\
    "}"

namespace {

// TODO: Improve copy-paste positioning. "Pasting into a sub control and" "positioning wherever you
// right click".. implement those 2 features in future

struct CopyPaste final
{
    enum ActionType { Invalid, Copy, Cut };

    CopyPaste() = delete;
    CopyPaste(const CopyPaste&) = delete;
    CopyPaste &operator=(const CopyPaste&) = delete;

    static inline ActionType actionType()
    { return s_actionType; }
    static inline QList<QPointer<Control>> controls()
    { return s_controls; }
    static inline bool isValid()
    { return s_actionType != Invalid && !s_controls.isEmpty(); }
    static inline void invalidate()
    { s_actionType = Invalid; s_controls.clear(); }
    static inline void setControls(const QList<QPointer<Control>>& value, ActionType actionType)
    { s_controls = value; s_actionType = actionType; }

private:
    static ActionType s_actionType;
    static QList<QPointer<Control>> s_controls;
};
QList<QPointer<Control>> CopyPaste::s_controls;
CopyPaste::ActionType CopyPaste::s_actionType = CopyPaste::Invalid;

QString methodName(const QString& signal)
{
    QString method(signal);
    method.replace(0, 1, method[0].toUpper());
    return method.prepend("on");
}

bool warnIfFileDoesNotExist(const QString& filePath)
{
    if (!QFileInfo::exists(filePath)) {
        return UtilityFunctions::showMessage(
                    nullptr, QObject::tr("Oops"),
                    QObject::tr("File %1 does not exist.").arg(QFileInfo(filePath).fileName()));
    }
    return false;
}


QString fixedTargetId(const Control* sourceControl, const Control* targetControl)
{
    if (targetControl->window() || targetControl->popup())
        return QStringLiteral("parent");
    if (sourceControl->parentControl() == targetControl)
        return QStringLiteral("parent");
    return targetControl->id();
}

QString anchorLineText(AnchorLine::Type type)
{
    switch (type) {
    case AnchorLine::Left:
        return QStringLiteral("left");
    case AnchorLine::Right:
        return QStringLiteral("right");
    case AnchorLine::Top:
        return QStringLiteral("top");
    case AnchorLine::Bottom:
        return QStringLiteral("bottom");
    case AnchorLine::Baseline:
        return QStringLiteral("baseline");
    case AnchorLine::HorizontalCenter:
        return QStringLiteral("horizontalCenter");
    case AnchorLine::VerticalCenter:
        return QStringLiteral("verticalCenter");
    case AnchorLine::Fill:
        return QStringLiteral("fill");
    case AnchorLine::Center:
        return QStringLiteral("centerIn");
    default:
        return QString();
    }
}

QString marginOffsetText(AnchorLine::Type type)
{
    switch (type) {
    case AnchorLine::Left:
        return QStringLiteral("leftMargin");
    case AnchorLine::Right:
        return QStringLiteral("rightMargin");
    case AnchorLine::Top:
        return QStringLiteral("topMargin");
    case AnchorLine::Bottom:
        return QStringLiteral("bottomMargin");
    case AnchorLine::Baseline:
        return QStringLiteral("baselineOffset");
    case AnchorLine::HorizontalCenter:
        return QStringLiteral("horizontalCenterOffset");
    case AnchorLine::VerticalCenter:
        return QStringLiteral("verticalCenterOffset");
    default:
        return QString();
    }
}
}

DesignerView::DesignerView(QWidget* parent) : QGraphicsView(new DesignerScene(parent), parent)
  , m_anchorEditor(new AnchorEditor(scene(), this))
  , m_panningState(Panning::NotStarted)
{
    setAlignment(Qt::AlignCenter);
    setResizeAnchor(AnchorViewCenter);
    setRubberBandSelectionMode(Qt::IntersectsItemBoundingRect);
    setDragMode(RubberBandDrag);
    setTransformationAnchor(AnchorUnderMouse);
    setFrameShape(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Window);
    setViewportUpdateMode(FullViewportUpdate);

    // as mousetracking only works for mouse key it is better to handle it in the
    // eventFilter method so it works also for the space scrolling case as expected
    QCoreApplication::instance()->installEventFilter(this);

    connect(ControlPropertyManager::instance(), &ControlPropertyManager::doubleClicked,
            this, [=] (Control* i, Qt::MouseButtons buttons) {
        if (QGraphicsItem* mouseGrabber = scene()->mouseGrabberItem())
            mouseGrabber->ungrabMouse();
        onControlDoubleClick(i, buttons);
    }, Qt::QueuedConnection);

    connect(m_anchorEditor, &AnchorEditor::anchored, this, [=] (AnchorLine::Type sourceLineType, const AnchorLine& targetLine) {
        if (targetLine.isValid()) {
            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                               "anchors." + anchorLineText(sourceLineType),
                                               fixedTargetId(m_anchorEditor->sourceControl(), targetLine.control())
                                               + "." + anchorLineText(targetLine.type()),
                                               ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
        } else {
            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                               "anchors." + anchorLineText(sourceLineType),
                                               QString(), ControlPropertyManager::SaveChanges);
            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                               "anchors." + anchorLineText(sourceLineType),
                                               "undefined", ControlPropertyManager::UpdateRenderer);
        }
    });
    connect(m_anchorEditor, &AnchorEditor::filled, this, [=] (Control* control) {
        if (control) {
            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                               "anchors.fill",
                                               fixedTargetId(m_anchorEditor->sourceControl(), control),
                                               ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
        } else {
            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                               "anchors.fill", QString(), ControlPropertyManager::SaveChanges);
            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                               "anchors.fill", "undefined", ControlPropertyManager::UpdateRenderer);
        }
    });
    connect(m_anchorEditor, &AnchorEditor::centered, this, [=] (Control* control, bool overlay) {
        if (control) {
            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                               "anchors.centerIn",
                                               fixedTargetId(m_anchorEditor->sourceControl(), control),
                                               ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
        } else {
            if (m_anchorEditor->sourceControl()->popup() && overlay) {
                ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                                   "anchors.centerIn", "Overlay.overlay",
                                                   ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
            } else {
                ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                                   "anchors.centerIn", QString(), ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                                   "anchors.centerIn", "undefined", ControlPropertyManager::UpdateRenderer);
            }
        }
    });
    connect(m_anchorEditor, &AnchorEditor::cleared, this, [=] {
        for (const QString& name : UtilityFunctions::anchorLineNames()) {
            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                               name, QString(),
                                               ControlPropertyManager::SaveChanges);
            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                               name, "undefined",
                                               ControlPropertyManager::UpdateRenderer);
        }
        for (const QString& name : UtilityFunctions::anchorPropertyNames()) {
            if (name == "anchors.leftMargin"
                    || name == "anchors.rightMargin"
                    || name == "anchors.topMargin"
                    || name == "anchors.bottomMargin") {
                ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                                   name, QString(), ControlPropertyManager::SaveChanges);
                ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                                   name, "undefined", ControlPropertyManager::UpdateRenderer);
            } else {
                ControlPropertyManager::setProperty(m_anchorEditor->sourceControl(),
                                                    name, QString(),
                                                    name.contains("alignWhenCentered") ? 1 : 0,
                                                    ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
            }
        }
    });
    connect(m_anchorEditor, &AnchorEditor::marginOffsetEdited, this, [=] (AnchorLine::Type sourceLineType, qreal marginOffset) {
        if (marginOffset == 0 && (sourceLineType == AnchorLine::Left
                                  || sourceLineType == AnchorLine::Right
                                  || sourceLineType == AnchorLine::Top
                                  || sourceLineType == AnchorLine::Bottom)) {
            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                               "anchors." + marginOffsetText(sourceLineType),
                                               QString(), ControlPropertyManager::SaveChanges);
            ControlPropertyManager::setBinding(m_anchorEditor->sourceControl(),
                                               "anchors." + marginOffsetText(sourceLineType),
                                               "undefined", ControlPropertyManager::UpdateRenderer);
        } else {
            ControlPropertyManager::setProperty(m_anchorEditor->sourceControl(),
                                                "anchors." + marginOffsetText(sourceLineType),
                                                marginOffset == 0 ? QString() : QString::number(marginOffset), marginOffset,
                                                ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
        }
    });
    connect(m_anchorEditor, &AnchorEditor::marginsEdited, this, [=] (qreal margins) {
        ControlPropertyManager::setProperty(m_anchorEditor->sourceControl(),
                                            "anchors.margins",
                                            margins == 0 ? QString() : QString::number(margins), margins,
                                            ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
    });
    connect(m_anchorEditor, &AnchorEditor::alignmentActivated, this, [=] (bool align) {
        ControlPropertyManager::setProperty(m_anchorEditor->sourceControl(),
                                            "anchors.alignWhenCentered",
                                            align ? QString() : "false", align,
                                            ControlPropertyManager::SaveChanges | ControlPropertyManager::UpdateRenderer);
    });
    connect(m_anchorEditor, &AnchorEditor::sourceControlActivated, this, [=] {
        scene()->clearSelection();
        m_anchorEditor->sourceControl()->setSelected(true);
    });
    connect(scene(), &DesignerScene::anchorEditorActivated, this, [=] (Control* sourceControl, Control* targetControl) {
        const QList<Control*>& selection = scene()->selectedControls();
        scene()->clearSelection();
        sourceControl->setSelected(true);
        m_anchorEditor->setSourceControl(sourceControl);
        m_anchorEditor->setPrimaryTargetControl(targetControl);
        m_anchorEditor->refresh();
        m_anchorEditor->exec();
        scene()->clearSelection();
        for (Control* control : selection)
            control->setSelected(true);
    });
}

void DesignerView::setZoomLevel(qreal zoomLevel)
{
    SceneSettings* settings = DesignerSettings::sceneSettings();
    qreal recentZoomLevel = matrix().m11();
    if (recentZoomLevel != zoomLevel) {
        resetTransform();
        scale((1.0 / recentZoomLevel) * zoomLevel, (1.0 / recentZoomLevel) * zoomLevel);
        settings->sceneZoomLevel = zoomLevel;
        settings->write();
    }
}

void DesignerView::discharge()
{
    scene()->discharge();
    update();
}

QSize DesignerView::sizeHint() const
{
    return QSize(680, 680);
}

void DesignerView::onControlDoubleClick(Control* control, Qt::MouseButtons buttons)
{
    if (buttons & Qt::LeftButton)
        m_qmlCodeEditorWidget->openDesigns(control, SaveUtils::controlMainQmlFileName());
    if (buttons & Qt::RightButton) {
        Control* sourceControl = control;
        Control* targetControl = control->parentControl();
        if (DesignerScene::isAnchorViable(sourceControl, targetControl)) {
            const QList<Control*>& selection = scene()->selectedControls();
            scene()->clearSelection();
            sourceControl->setSelected(true);
            m_anchorEditor->setSourceControl(sourceControl);
            m_anchorEditor->setPrimaryTargetControl(targetControl);
            m_anchorEditor->refresh();
            m_anchorEditor->exec();
            scene()->clearSelection();
            for (Control* control : selection)
                control->setSelected(true);
        }
    }
}

DesignerScene* DesignerView::scene() const
{
    Q_ASSERT(QGraphicsView::scene());
    return static_cast<DesignerScene*>(QGraphicsView::scene());
}

bool DesignerView::eventFilter(QObject *watched, QEvent *event)
{
    if (m_panningState != Panning::NotStarted) {
        if (event->type() == QEvent::Leave && m_panningState == Panning::SpaceKeyStarted) {
            // there is no way to keep the cursor so we stop panning here
            stopPanning(event);
        }
        if (event->type() == QEvent::MouseMove) {
            auto mouseEvent = static_cast<QMouseEvent*>(event);
            if (!m_panningStartPosition.isNull()) {
                horizontalScrollBar()->setValue(horizontalScrollBar()->value() -
                                                (mouseEvent->x() - m_panningStartPosition.x()));
                verticalScrollBar()->setValue(verticalScrollBar()->value() -
                                              (mouseEvent->y() - m_panningStartPosition.y()));
            }
            m_panningStartPosition = mouseEvent->pos();
            event->accept();
            return true;
        }
    }
    return QGraphicsView::eventFilter(watched, event);
}

void DesignerView::wheelEvent(QWheelEvent *event)
{
    // TODO: Implement a wheel zoom in/out for scene
    if (event->modifiers().testFlag(Qt::ControlModifier))
        event->ignore();
    else
        QGraphicsView::wheelEvent(event);
}

void DesignerView::mousePressEvent(QMouseEvent *event)
{
    if (m_panningState == Panning::NotStarted) {
        if (event->buttons().testFlag(Qt::MiddleButton))
            startPanning(event);
        else
            QGraphicsView::mousePressEvent(event);
    }
}

void DesignerView::mouseReleaseEvent(QMouseEvent *event)
{
    // not sure why buttons() are empty here, but we have that information from the enum
    if (m_panningState == Panning::MouseWheelStarted)
        stopPanning(event);
    else
        QGraphicsView::mouseReleaseEvent(event);
}

void DesignerView::keyPressEvent(QKeyEvent *event)
{
    // check for autorepeat to avoid a stoped space panning by leave event to be restarted
    if (!event->isAutoRepeat() && m_panningState == Panning::NotStarted && event->key() == Qt::Key_Space) {
        startPanning(event);
        return;
    }
    QGraphicsView::keyPressEvent(event);
}

void DesignerView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat() && m_panningState == Panning::SpaceKeyStarted)
        stopPanning(event);

    QGraphicsView::keyReleaseEvent(event);
}

void DesignerView::startPanning(QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
        m_panningState = Panning::SpaceKeyStarted;
    else
        m_panningState = Panning::MouseWheelStarted;
    viewport()->setCursor(Qt::ClosedHandCursor);
    event->accept();
}

void DesignerView::stopPanning(QEvent *event)
{
    m_panningState = Panning::NotStarted;
    m_panningStartPosition = QPoint();
    viewport()->unsetCursor();
    event->accept();
}
