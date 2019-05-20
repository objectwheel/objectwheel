#include <modeselectorpane.h>
#include <utilityfunctions.h>
#include <paintutils.h>

#include <QPainter>
#include <QVBoxLayout>

#define TOOLTIP(x) QStringLiteral(QT_TR_NOOP(R"(<span style="font-size:12px">Open <b>%2</b></span>)")).arg(tr((x)))
#include <QTimer>
ModeSelectorPane::ModeSelectorPane(QWidget* parent) : QToolBar(parent)
  , m_designerAction(new QAction(this))
  , m_editorAction(new QAction(this))
  , m_splitAction(new QAction(this))
  , m_optionsAction(new QAction(this))
  , m_buildsAction(new QAction(this))
  , m_documentsAction(new QAction(this))
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    setIconSize({16, 16});

    // Workaround for QToolBarLayout's obsolote serMargin function usage
    QMetaObject::invokeMethod(this, [=] {
        setContentsMargins(0, 0, 0, 0);
        layout()->setContentsMargins(0, 0, 0, 0); // They must be all same
        layout()->setSpacing(7);
    }, Qt::QueuedConnection);

    m_designerAction->setCheckable(true);
    m_designerAction->setText(tr("Designer"));
    m_designerAction->setToolTip(TOOLTIP("Designer"));

    m_editorAction->setCheckable(true);
    m_editorAction->setText(tr("Code Editor"));
    m_editorAction->setToolTip(TOOLTIP("Code Editor"));

    m_splitAction->setCheckable(true);
    m_splitAction->setText(tr("Split View"));
    m_splitAction->setToolTip(TOOLTIP("Splitted View"));

    m_optionsAction->setCheckable(true);
    m_optionsAction->setText(tr("Project Options"));
    m_optionsAction->setToolTip(TOOLTIP("Project Options"));

    m_buildsAction->setCheckable(true);
    m_buildsAction->setText(tr("Cloud Builds"));
    m_buildsAction->setToolTip(TOOLTIP("Cloud Builds"));

    m_documentsAction->setCheckable(true);
    m_documentsAction->setText(tr("Documents"));
    m_documentsAction->setToolTip(TOOLTIP("Documents"));

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
    //    connect(GeneralSettings::instance(), &GeneralSettings::interfaceSettingsChanged,
    //            this, &ModeSelectorPane::updateColors);

    setStyleSheet(
                  "QToolButton { /* all types of tool button */"
                  "    border: none;"
                  "    background: none;"
                  "    font-weight: 400;"
                  "    font-size: 14;"
                  "}"

                  "QToolButton::checked { /* all types of tool button */"
                  "    font-weight: 480;"
                  "}"

//                  "QToolButton::hover { /* all types of tool button */"
//                  "    border: 5px solid #8f8f91;"
//                  "    border-radius: 6px;"
//                  "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
//                  "                                      stop: 0 #f6f7fa, stop: 1 #dadbde);"
//                  "}"

//                  "QToolButton[popupMode=\"1\"] { /* only for MenuButtonPopup */"
//                  "    padding-right: 20px; /* make way for the popup button */"
//                  "}"

//                  "QToolButton:pressed {"
//                  "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
//                  "                                      stop: 0 #dadbde, stop: 1 #f6f7fa);"
//                  "}"

//                  "/* the subcontrols below are used only in the MenuButtonPopup mode */"
//                  "QToolButton::menu-button {"
//                  "    border: 2px solid gray;"
//                  "    border-top-right-radius: 6px;"
//                  "    border-bottom-right-radius: 6px;"
//                  "    /* 16px width + 4px for border = 20px allocated above */"
//                  "    width: 16px;"
//                  "}"

//                  "QToolButton::menu-arrow {"
//                  "    image: url(downarrow.png);"
//                  "}"

//                  "QToolButton::menu-arrow:open {"
//                      "top: 1px; left: 1px; /* shift it a bit */"
//                  "}"
                );
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
    return {100, 26};
}

QSize ModeSelectorPane::minimumSizeHint() const
{
    return {0, 26};
}

void ModeSelectorPane::updateIcons()
{
    using namespace PaintUtils;
    m_designerAction->setIcon(renderModeButtonIcon(":/images/modes/designer.svg", this));
    m_editorAction->setIcon(renderModeButtonIcon(":/images/modes/editor.svg", this));
    m_splitAction->setIcon(renderModeButtonIcon(":/images/modes/split.svg", this));
    m_optionsAction->setIcon(renderModeButtonIcon(":/images/modes/options.svg", this));
    m_buildsAction->setIcon(renderModeButtonIcon(":/images/modes/builds.svg", this));
    m_documentsAction->setIcon(renderModeButtonIcon(":/images/modes/documents.svg", this));
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