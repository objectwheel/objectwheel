#include <bottombar.h>
#include <pushbutton.h>
#include <paintutils.h>

#include <QHBoxLayout>
#include <QStylePainter>
#include <QStyleOption>

using namespace PaintUtils;

BottomBar::BottomBar(QWidget* parent) : QWidget(parent)
  , m_layout(new QHBoxLayout(this))
  , m_consoleButton(new PushButton(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_layout->setContentsMargins(1, 1, 1, 1);
    m_layout->setSpacing(5);
    m_layout->addWidget(m_consoleButton, 0, Qt::AlignVCenter);
    m_layout->addStretch();

    m_consoleButton->setMaximumHeight(22);
    m_consoleButton->setCursor(Qt::PointingHandCursor);
    m_consoleButton->setCheckable(true);
    m_consoleButton->setText(tr("Console"));
    m_consoleButton->setToolTip(tr("Activate console output"));
    m_consoleButton->setIcon(renderColorizedIcon(":/images/console.png", palette().buttonText().color(), this));
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
