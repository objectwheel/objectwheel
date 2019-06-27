#include <outputbar.h>
#include <paintutils.h>
#include <buttonflasher.h>

#include <QBoxLayout>
#include <QPainter>

OutputBar::OutputBar(QWidget* parent) : QWidget(parent)
  , m_consoleButton(new PushButton(this))
  , m_issuesButton(new PushButton(this))
  , m_consoleFlasher(new ButtonFlasher(m_consoleButton))
  , m_issuesFlasher(new ButtonFlasher(m_issuesButton))
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    auto layout = new QHBoxLayout(this);
    layout->setSpacing(4);
    layout->setContentsMargins(3, 2, 3, 2);
    layout->addWidget(m_issuesButton, 0, Qt::AlignVCenter);
    layout->addWidget(m_consoleButton, 0, Qt::AlignVCenter);
    layout->addStretch();

    m_consoleButton->setCheckable(true);
    m_consoleButton->setAutoExclusive(true);
    m_consoleButton->setFixedHeight(22);
    m_consoleButton->setCursor(Qt::PointingHandCursor);
    m_consoleButton->setText(tr("Console Output"));
    m_consoleButton->setToolTip(tr("Activate console output"));
    m_consoleButton->setIconSize({14, 14});
    m_consoleButton->setIcon(PaintUtils::renderButtonIcon(":/images/console.svg",
                                                          m_consoleButton->palette()));

    m_issuesButton->setCheckable(true);
    m_issuesButton->setAutoExclusive(true);
    m_issuesButton->setFixedHeight(22);
    m_issuesButton->setCursor(Qt::PointingHandCursor);
    m_issuesButton->setText(tr("Issues") + " [0]");
    m_issuesButton->setToolTip(tr("Activate issues list"));
    m_issuesButton->setIconSize({14, 14});
    m_issuesButton->setIcon(PaintUtils::renderButtonIcon(":/images/issues.svg",
                                                         m_issuesButton->palette()));

    connect(m_issuesButton, &PushButton::clicked, this, &OutputBar::onButtonClick);
    connect(m_consoleButton, &PushButton::clicked, this, &OutputBar::onButtonClick);
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

void OutputBar::flash(QAbstractButton* button)
{
    if (button == m_consoleButton)
        m_consoleFlasher->flash();
    else if (button == m_issuesButton)
        m_issuesFlasher->flash();
}

void OutputBar::onButtonClick(bool checked)
{
    if (activeButton())
        emit buttonActivated(activeButton(), false);

    auto activatedButton = static_cast<PushButton*>(sender());
    if (activatedButton != activeButton())
        emit buttonActivated(activatedButton, checked);
}

void OutputBar::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), palette().window());
    painter.setPen("#c4c4c4");
    painter.drawLine(rect().topLeft() + QPointF(0.5, 0.0),
                     rect().topRight() + QPointF(0.5, 0.0));
    painter.drawLine(rect().topLeft() + QPointF(0.0, 0.5),
                     rect().bottomLeft() + QPointF(0.0, 0.5));
    painter.drawLine(rect().topRight() + QPointF(1.0, 0.5),
                     rect().bottomRight() + QPointF(1.0, 0.5));
}
