#include <outputbar.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <generalsettings.h>
#include <interfacesettings.h>
#include <buttonflasher.h>

#include <QHBoxLayout>
#include <QStylePainter>
#include <QButtonGroup>
#include <QApplication>

using namespace PaintUtils;

OutputBar::OutputBar(QWidget* parent) : QWidget(parent)
  , m_layout(new QHBoxLayout(this))
  , m_buttonGroup(new QButtonGroup(this))
  , m_consoleButton(new PushButton(this))
  , m_issuesButton(new PushButton(this))
  , m_consoleFlasher(new ButtonFlasher(m_consoleButton))
  , m_issuesFlasher(new ButtonFlasher(m_issuesButton))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_layout->setSpacing(4);
    m_layout->setContentsMargins(4, 2, 4, 2);
    m_layout->addWidget(m_issuesButton, 0, Qt::AlignVCenter);
    m_layout->addWidget(m_consoleButton, 0, Qt::AlignVCenter);
    m_layout->addStretch();

    m_buttonGroup->addButton(m_consoleButton);
    m_buttonGroup->addButton(m_issuesButton);
    m_buttonGroup->setExclusive(false);
    connect(m_buttonGroup, qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked),
            this, &OutputBar::onButtonClick);

    m_consoleButton->setMaximumHeight(22);
    m_consoleButton->setCursor(Qt::PointingHandCursor);
    m_consoleButton->setCheckable(true);
    m_consoleButton->setText(tr("Console Output"));
    m_consoleButton->setToolTip(tr("Activate console output"));
    m_consoleButton->setIconSize({14, 14});
    m_consoleButton->setIcon(renderButtonIcon(":/images/console.svg", m_consoleButton->palette()));

    m_issuesButton->setMaximumHeight(22);
    m_issuesButton->setCursor(Qt::PointingHandCursor);
    m_issuesButton->setCheckable(true);
    m_issuesButton->setText(tr("Issues") + " [0]");
    m_issuesButton->setToolTip(tr("Activate issues list"));
    m_issuesButton->setIconSize({14, 14});
    m_issuesButton->setIcon(renderButtonIcon(":/images/issues.svg", m_issuesButton->palette()));
}

QAbstractButton* OutputBar::activeButton() const
{
    if (m_consoleButton->isChecked())
        return m_consoleButton;
    if (m_issuesButton->isChecked())
        return m_issuesButton;
    return nullptr;
}

QAbstractButton* OutputBar::consoleButton() const
{
    return m_consoleButton;
}

QAbstractButton* OutputBar::issuesButton() const
{
    return m_issuesButton;
}

QSize OutputBar::sizeHint() const
{
    return {100, 26};
}

QSize OutputBar::minimumSizeHint() const
{
    return {0, 26};
}

void OutputBar::discharge()
{
    m_issuesButton->setChecked(false);
    m_consoleButton->setChecked(false);

    const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    if (settings->visibleBottomPane != "None") {
        if (settings->visibleBottomPane == "Console Pane")
            m_consoleButton->click();
        else
            m_issuesButton->click();
    }
}

void OutputBar::flash(QAbstractButton* button)
{
    if (button == m_consoleButton)
        m_consoleFlasher->flash(400, 3);
    else if (button == m_issuesButton)
        m_issuesFlasher->flash(400, 3);

    const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    if (settings->bottomPanesPop && !button->isChecked())
        button->animateClick();
}

void OutputBar::onButtonClick(QAbstractButton* button)
{
    emit buttonActivated(button, button->isChecked());
    if (!button->isChecked())
        return;
    for (QAbstractButton* b : m_buttonGroup->buttons()) {
        if (b != button) {
            b->setChecked(false);
            emit buttonActivated(b, false);
        }
    }
}

void OutputBar::paintEvent(QPaintEvent*)
{
    QStylePainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen("#c4c4c4");
    p.setBrush(Qt::NoBrush);
    p.drawLine(rect().topLeft() + QPointF(0.5, 0.0),
               rect().topRight() + QPointF(0.5, 0.0));
    p.drawLine(rect().topLeft() + QPointF(0.0, 0.5),
               rect().bottomLeft() + QPointF(0.0, 0.5));
    p.drawLine(rect().topRight() + QPointF(1.0, 0.5),
               rect().bottomRight() + QPointF(1.0, 0.5));
}
