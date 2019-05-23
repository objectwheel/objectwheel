#include <modeselectorpane.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QPainter>
#include <QLayout>

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

    using namespace UtilityFunctions;
    m_designerAction->setCheckable(true);
    m_designerAction->setText(tr("Designer"));
    m_designerAction->setToolTip(toModeToolTip(tr("Designer")));

    m_editorAction->setCheckable(true);
    m_editorAction->setText(tr("Code Editor"));
    m_editorAction->setToolTip(toModeToolTip(tr("Code Editor")));

    m_splitAction->setCheckable(true);
    m_splitAction->setText(tr("Split View"));
    m_splitAction->setToolTip(toModeToolTip(tr("Splitted View")));

    m_optionsAction->setCheckable(true);
    m_optionsAction->setText(tr("Project Options"));
    m_optionsAction->setToolTip(toModeToolTip(tr("Project Options")));

    m_buildsAction->setCheckable(true);
    m_buildsAction->setText(tr("Cloud Builds"));
    m_buildsAction->setToolTip(toModeToolTip(tr("Cloud Builds")));

    m_documentsAction->setCheckable(true);
    m_documentsAction->setText(tr("Documents"));
    m_documentsAction->setToolTip(toModeToolTip(tr("Documents")));

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
    m_designerAction->setIcon(renderToolButtonIcon(":/images/modes/designer.svg", this));
    m_editorAction->setIcon(renderToolButtonIcon(":/images/modes/editor.svg", this));
    m_splitAction->setIcon(renderToolButtonIcon(":/images/modes/split.svg", this));
    m_optionsAction->setIcon(renderToolButtonIcon(":/images/modes/options.svg", this));
    m_buildsAction->setIcon(renderToolButtonIcon(":/images/modes/builds.svg", this));
    m_documentsAction->setIcon(renderToolButtonIcon(":/images/modes/documents.svg", this));
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
    // FIXME: hardcoded, what happens if it is vertical toolbar?
}