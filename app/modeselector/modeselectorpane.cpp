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
    setFocusPolicy(Qt::NoFocus);
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
    m_designerAction->setToolTip(toToolTip(tr("Switch to <b>Designer</b>")));

    m_editorAction->setCheckable(true);
    m_editorAction->setText(tr("Code Editor"));
    m_editorAction->setToolTip(toToolTip(tr("Switch to <b>Code Editor</b>")));

    m_splitAction->setCheckable(true);
    m_splitAction->setText(tr("Split View"));
    m_splitAction->setToolTip(toToolTip(tr("Switch to <b>Split View</b>")));

    m_optionsAction->setCheckable(true);
    m_optionsAction->setText(tr("Project Options"));
    m_optionsAction->setToolTip(toToolTip(tr("Switch to <b>Project Options</b>")));

    m_buildsAction->setCheckable(true);
    m_buildsAction->setText(tr("Cloud Builds"));
    m_buildsAction->setToolTip(toToolTip(tr("Switch to <b>Cloud Builds</b>")));

    m_documentsAction->setCheckable(true);
    m_documentsAction->setText(tr("Documents"));
    m_documentsAction->setToolTip(toToolTip(tr("Switch to <b>Documents</b>")));

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

QAction* ModeSelectorPane::documentsAction() const
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
    m_designerAction->setIcon(QIcon(":/images/modes/designer.svg"));
    m_editorAction->setIcon(QIcon(":/images/modes/editor.svg"));
    m_splitAction->setIcon(QIcon(":/images/modes/split.svg"));
    m_optionsAction->setIcon(QIcon(":/images/modes/options.svg"));
    m_buildsAction->setIcon(QIcon(":/images/modes/builds.svg"));
    m_documentsAction->setIcon(QIcon(":/images/modes/documents.svg"));
}

void ModeSelectorPane::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QLinearGradient gradient(0, 0, 0, 1);
    gradient.setCoordinateMode(QGradient::ObjectMode);
    gradient.setColorAt(0, "#d6d6d6");
    gradient.setColorAt(1, "#cdcdcd");
    painter.fillRect(rect(), gradient);
    painter.setPen("#bdbdbd");
    painter.drawLine(QRectF(rect()).bottomLeft() - QPointF(0, 0.5),
                     QRectF(rect()).bottomRight() - QPointF(0, 0.5));
    painter.setPen("#a7a7a7");
    painter.drawLine(QRectF(rect()).bottomLeft(), QRectF(rect()).bottomRight());
    // FIXME: hardcoded, what happens if it is vertical toolbar?
}