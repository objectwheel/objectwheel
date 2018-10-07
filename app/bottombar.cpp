#include <bottombar.h>
#include <pushbutton.h>
#include <paintutils.h>

#include <QHBoxLayout>
#include <QStylePainter>
#include <QStyleOption>
#include <QButtonGroup>

using namespace PaintUtils;

BottomBar::BottomBar(QWidget* parent) : QWidget(parent)
  , m_layout(new QHBoxLayout(this))
  , m_consoleButton(new PushButton(this))
  , m_issuesButton(new PushButton(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_layout->setSpacing(1);
    m_layout->setContentsMargins(1, 1, 1, 1);
    m_layout->addWidget(m_issuesButton, 0, Qt::AlignVCenter);
    m_layout->addWidget(m_consoleButton, 0, Qt::AlignVCenter);
    m_layout->addStretch();

    auto buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(m_consoleButton);
    buttonGroup->addButton(m_issuesButton);
    buttonGroup->setExclusive(false);
    connect(buttonGroup, qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked),
            this, [=] (QAbstractButton* button) {
        if (!button->isChecked())
            return;
        for (QAbstractButton* b : buttonGroup->buttons()) {
            if (b != button)
                b->setChecked(false);
        }
    });

    m_consoleButton->setMaximumHeight(22);
    m_consoleButton->setCursor(Qt::PointingHandCursor);
    m_consoleButton->setCheckable(true);
    m_consoleButton->setText(tr("Console Output"));
    m_consoleButton->setToolTip(tr("Activate console output"));
    m_consoleButton->setIcon(renderColorizedIcon(":/images/console.png", palette().buttonText().color(), this));

    m_issuesButton->setMaximumHeight(22);
    m_issuesButton->setCursor(Qt::PointingHandCursor);
    m_issuesButton->setCheckable(true);
    m_issuesButton->setText(tr("Issues"));
    m_issuesButton->setToolTip(tr("Activate issues list"));
    m_issuesButton->setIcon(renderColorizedIcon(":/images/issues.png", palette().buttonText().color(), this));
}

void BottomBar::paintEvent(QPaintEvent*)
{
    QStylePainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QStyleOptionToolBar opt;
    opt.initFrom(this);
    opt.state |= QStyle::State_Horizontal;
    p.drawControl(QStyle::CE_ToolBar, opt);
}

QSize BottomBar::sizeHint() const
{
    return QSize(100, 24);
}

QSize BottomBar::minimumSizeHint() const
{
    return QSize(0, 24);
}
