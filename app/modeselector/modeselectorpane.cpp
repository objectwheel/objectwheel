#include <modeselectorpane.h>
#include <utilityfunctions.h>
#include <paintutils.h>

#include <QPainter>
#include <QVBoxLayout>

#define TOOLTIP(x) QStringLiteral(QT_TR_NOOP(R"(<span style="font-size:12px">Open <b>%2</b></span>)")).arg(x)

ModeSelectorPane::ModeSelectorPane(QWidget* parent) : QToolBar(parent)
  , m_designerAction(new QAction(this))
  , m_editorAction(new QAction(this))
  , m_splitAction(new QAction(this))
  , m_optionsAction(new QAction(this))
  , m_buildsAction(new QAction(this))
  , m_documentsAction(new QAction(this))
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setIconSize({16, 16});

    // Workaround for QToolBarLayout's obsolote serMargin function usage
    QMetaObject::invokeMethod(this, [=] {
        setContentsMargins(0, 0, 0, 0);
        layout()->setContentsMargins(2, 2, 2, 2); // They must be all same
        layout()->setSpacing(7);
    }, Qt::QueuedConnection);

    m_designerAction->setCheckable(true);
    m_designerAction->setText(tr("Designer"));
    m_designerAction->setToolTip(TOOLTIP(tr("Designer")));

    m_editorAction->setCheckable(true);
    m_editorAction->setText(tr("Code Editor"));
    m_editorAction->setToolTip(TOOLTIP(tr("Code Editor")));

    m_splitAction->setCheckable(true);
    m_splitAction->setText(tr("Split View"));
    m_splitAction->setToolTip(TOOLTIP(tr("Splitted View")));

    m_optionsAction->setCheckable(true);
    m_optionsAction->setText(tr("Project Options"));
    m_optionsAction->setToolTip(TOOLTIP(tr("Project Options")));

    m_buildsAction->setCheckable(true);
    m_buildsAction->setText(tr("Cloud Builds"));
    m_buildsAction->setToolTip(TOOLTIP(tr("Cloud Builds")));

    m_documentsAction->setCheckable(true);
    m_documentsAction->setText(tr("Documents"));
    m_documentsAction->setToolTip(TOOLTIP(tr("Documents")));

    auto actionGroup = new QActionGroup(this);
    actionGroup->addAction(m_designerAction);
    actionGroup->addAction(m_editorAction);
    actionGroup->addAction(m_splitAction);
    actionGroup->addAction(m_optionsAction);
    actionGroup->addAction(m_buildsAction);
    actionGroup->addAction(m_documentsAction);

    addAction(m_designerAction);
    addAction(m_editorAction);
    addAction(m_splitAction);
    addAction(m_optionsAction);
    addAction(m_buildsAction);
    addAction(m_documentsAction);
    updateIcons();

//    setStyleSheet(
//                "QToolButton { /* all types of tool button */"
//                "    border: none;"
//                "    margin: 0px;"
//                "    padding: 0px;"
//                "    padding-left: 5px;"
//                "    border-radius: 5px;"
//                "    color: #2f2f2f;"
//                "}"
//                "QToolButton::hover { /* all types of tool button */"
//                "    background: #bdbdbd;"
//                "}"
//                "QToolButton::pressed { /* all types of tool button */"
//                "    background: #909090;"
//                "    color: white;"
//                "}"
//                "QToolButton::checked { /* all types of tool button */"
//                "    background: #a0a0a0;"
//                "    color: white;"
//                "}"
//                );
}

QAction* ModeSelectorPane::designerAction() const
{
    return m_designerAction;
}

QAction* ModeSelectorPane::editorAction() const
{
    return m_editorAction;
}

QAction* ModeSelectorPane::splitAction() const
{
    return m_splitAction;
}

QAction* ModeSelectorPane::optionsAction() const
{
    return m_optionsAction;
}

QAction* ModeSelectorPane::buildsAction() const
{
    return m_buildsAction;
}

QAction* ModeSelectorPane::helpAction() const
{
    return m_documentsAction;
}

QSize ModeSelectorPane::sizeHint() const
{
    return {100, 24};
}

QSize ModeSelectorPane::minimumSizeHint() const
{
    return {0, 24};
}

void ModeSelectorPane::updateIcons()
{
    using namespace PaintUtils;
//    m_runDevicesButton->setIcon(renderButtonIcon(":/images/devices.png", m_runDevicesButton));
//    m_runButton->setIcon(renderMaskedButtonIcon(":/utils/images/run_small@2x.png", m_runButton));
//    m_stopButton->setIcon(renderMaskedButtonIcon(":/utils/images/stop_small@2x.png", m_stopButton));
//    m_preferencesButton->setIcon(renderOverlaidButtonIcon(":/images/settings.svg", m_preferencesButton));
//    m_projectsButton->setIcon(renderOverlaidButtonIcon(":/images/projects.svg", m_projectsButton));

    m_designerAction->setIcon(renderButtonIcon(":/images/modes/designer.svg", this));
    m_editorAction->setIcon(renderButtonIcon(":/images/modes/editor.svg", this));
    m_splitAction->setIcon(renderButtonIcon(":/images/modes/split.svg", this));
    m_optionsAction->setIcon(renderButtonIcon(":/images/modes/options.svg", this));
    m_buildsAction->setIcon(renderButtonIcon(":/images/modes/builds.svg", this));
    m_documentsAction->setIcon(renderButtonIcon(":/images/modes/documents.svg", this));
}

bool ModeSelectorPane::event(QEvent* event)
{
    return QToolBar::event(event);
}

void ModeSelectorPane::changeEvent(QEvent* event)
{
    QToolBar::changeEvent(event);
}

void ModeSelectorPane::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QLinearGradient gradient({0.0, 0.0}, {0.0, 1.0});
    gradient.setCoordinateMode(QGradient::ObjectMode);
    gradient.setColorAt(0, "#d6d6d6");
    gradient.setColorAt(1, "#cccccc");
    painter.fillRect(rect(), gradient);
    painter.setPen("#bebebe");
    painter.drawLine(QRectF(rect()).bottomLeft() - QPointF(0, 0.5),
                     QRectF(rect()).bottomRight() - QPointF(0, 0.5));
    painter.setPen("#a7a7a7");
    painter.drawLine(QRectF(rect()).bottomLeft(), QRectF(rect()).bottomRight());
}