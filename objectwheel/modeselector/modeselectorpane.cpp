#include <modeselectorpane.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QPainter>
#include <QLayout>
#include <QButtonGroup>
#include <QToolButton>
#include <QEvent>

static QIcon icon(const QString& fileName)
{
    QIcon icon;
    icon.addFile(":/images/modes/" + fileName + ".svg", QSize(), QIcon::Normal, QIcon::Off);
    icon.addFile(":/images/modes/" + fileName + "-on.svg", QSize(), QIcon::Normal, QIcon::On);
    return icon;
}

ModeSelectorPane::ModeSelectorPane(QWidget* parent) : QToolBar(parent)
  , m_designerButton(new QToolButton(this))
  , m_editorButton(new QToolButton(this))
  , m_splitButton(new QToolButton(this))
  , m_optionsButton(new QToolButton(this))
  , m_buildsButton(new QToolButton(this))
  , m_documentsButton(new QToolButton(this))
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    layout()->setContentsMargins(2, 2, 2, 2);
    layout()->setSpacing(7);

    m_designerButton->setCheckable(true);
    m_designerButton->setText(tr("Designer"));
    m_designerButton->setShortcut(Qt::CTRL + Qt::Key_1);
    m_designerButton->setToolTip(tr("Switch to <b>Designer</b>") + UtilityFunctions::shortcutSymbol(m_designerButton->shortcut()));

    m_editorButton->setCheckable(true);
    m_editorButton->setText(tr("Code Editor"));
    m_editorButton->setShortcut(Qt::CTRL + Qt::Key_2);
    m_editorButton->setToolTip(tr("Switch to <b>Code Editor</b>") + UtilityFunctions::shortcutSymbol(m_editorButton->shortcut()));

    m_splitButton->setCheckable(true);
    m_splitButton->setText(tr("Split View"));
    m_splitButton->setShortcut(Qt::CTRL + Qt::Key_3);
    m_splitButton->setToolTip(tr("Switch to <b>Split View</b>") + UtilityFunctions::shortcutSymbol(m_splitButton->shortcut()));

    m_optionsButton->setCheckable(true);
    m_optionsButton->setText(tr("Project Options"));
    m_optionsButton->setShortcut(Qt::CTRL + Qt::Key_4);
    m_optionsButton->setToolTip(tr("Switch to <b>Project Options</b>") + UtilityFunctions::shortcutSymbol(m_optionsButton->shortcut()));

    m_buildsButton->setCheckable(true);
    m_buildsButton->setText(tr("Cloud Builds"));
    m_buildsButton->setShortcut(Qt::CTRL + Qt::Key_5);
    m_buildsButton->setToolTip(tr("Switch to <b>Cloud Builds</b>") + UtilityFunctions::shortcutSymbol(m_buildsButton->shortcut()));

    m_documentsButton->setCheckable(true);
    m_documentsButton->setText(tr("Documents"));
    m_documentsButton->setShortcut(Qt::CTRL + Qt::Key_6);
    m_documentsButton->setToolTip(tr("Switch to <b>Documents</b>") + UtilityFunctions::shortcutSymbol(m_documentsButton->shortcut()));

    m_designerButton->setCursor(Qt::PointingHandCursor);
    m_editorButton->setCursor(Qt::PointingHandCursor);
    m_splitButton->setCursor(Qt::PointingHandCursor);
    m_optionsButton->setCursor(Qt::PointingHandCursor);
    m_buildsButton->setCursor(Qt::PointingHandCursor);
    m_documentsButton->setCursor(Qt::PointingHandCursor);

    m_designerButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_editorButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_splitButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_optionsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_buildsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_documentsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    m_designerButton->setFixedHeight(18);
    m_editorButton->setFixedHeight(18);
    m_splitButton->setFixedHeight(18);
    m_optionsButton->setFixedHeight(18);
    m_buildsButton->setFixedHeight(18);
    m_documentsButton->setFixedHeight(18);

    auto buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(m_designerButton);
    buttonGroup->addButton(m_editorButton);
    buttonGroup->addButton(m_splitButton);
    buttonGroup->addButton(m_optionsButton);
    buttonGroup->addButton(m_buildsButton);
    buttonGroup->addButton(m_documentsButton);

    addWidget(m_designerButton);
    addWidget(m_editorButton);
    addWidget(m_splitButton);
    addWidget(m_optionsButton);
    addWidget(m_buildsButton);
    addWidget(m_documentsButton);

    updateIcons();
}

QToolButton* ModeSelectorPane::designerButton() const
{
    return m_designerButton;
}

QToolButton* ModeSelectorPane::editorButton() const
{
    return m_editorButton;
}

QToolButton* ModeSelectorPane::splitButton() const
{
    return m_splitButton;
}

QToolButton* ModeSelectorPane::optionsButton() const
{
    return m_optionsButton;
}

QToolButton* ModeSelectorPane::buildsButton() const
{
    return m_buildsButton;
}

QToolButton* ModeSelectorPane::documentsButton() const
{
    return m_documentsButton;
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
    m_designerButton->setIcon(icon("designer"));
    m_editorButton->setIcon(icon("editor"));
    m_splitButton->setIcon(icon("split"));
    m_optionsButton->setIcon(icon("options"));
    m_buildsButton->setIcon(icon("builds"));
    m_documentsButton->setIcon(icon("documents"));
}

void ModeSelectorPane::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QLinearGradient gradient(0, 0, 0, 1);
    gradient.setCoordinateMode(QGradient::ObjectMode);
    gradient.setColorAt(0, "#d6d6d6");
    gradient.setColorAt(1, "#cdcdcd");
    painter.fillRect(rect(), gradient);
    painter.setPen("#a7a7a7");
    painter.drawLine(QRectF(rect()).bottomLeft(), QRectF(rect()).bottomRight());
}

void ModeSelectorPane::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::ApplicationFontChange
            || event->type() == QEvent::PaletteChange) {
        updateIcons();
    }
    QToolBar::changeEvent(event);
}