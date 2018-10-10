#include <bottombar.h>
#include <pushbutton.h>
#include <paintutils.h>
#include <utilsicons.h>

#include <QHBoxLayout>
#include <QStylePainter>
#include <QStyleOption>
#include <QButtonGroup>
#include <QTimer>
#include <QApplication>

using namespace PaintUtils;
using namespace Utils;
using namespace Utils::Icons;

namespace {
const char* g_tooltip = "<span style=\"font-size: 12px !important;\">%1</span>";

/*!
    QPalette::Light     :  Button's frame rect color (not used)
    QPalette::Midlight  :  Button's glowing color (when pressed)
    QPalette::Button    :  Button's normal body color
    QPalette::Mid       :  Button's pressed body color
    QPalette::Dark      :  Button's base and surrounding border's color
    QPalette::Shadow    :  Button's drop shadow color
*/
void setPanelButtonPaletteDarkerShadows(QWidget* widget)
{
    QPalette palette(widget->palette());

    QLinearGradient shadowGrad({0.0, 0.5}, {1.0, 0.5});
    shadowGrad.setCoordinateMode(QGradient::ObjectMode);
    shadowGrad.setColorAt(0, "#52202020");
    shadowGrad.setColorAt(0.05, "#50202020");
    shadowGrad.setColorAt(0.5, "#50202020");
    shadowGrad.setColorAt(0.95, "#50202020");
    shadowGrad.setColorAt(1, "#52202020");
    palette.setBrush(QPalette::Shadow, shadowGrad);

    QLinearGradient darkGrad({0.0, 0.0}, {0.0, 1.0});
    darkGrad.setCoordinateMode(QGradient::ObjectMode);
    darkGrad.setColorAt(0.85, "#60303030");
    darkGrad.setColorAt(1, "#7d000000");
    palette.setBrush(QPalette::Dark, darkGrad);

    QLinearGradient midGrad({0.0, 0.0}, {0.0, 1.0});
    midGrad.setCoordinateMode(QGradient::ObjectMode);
    midGrad.setColorAt(0, "#e7e7e7");
    midGrad.setColorAt(1, "#e1e1e1");
    palette.setBrush(QPalette::Mid, midGrad);

    QLinearGradient buttonGrad({0.0, 0.0}, {0.0, 1.0});
    buttonGrad.setCoordinateMode(QGradient::ObjectMode);
    buttonGrad.setColorAt(0, "#fefefe");
    buttonGrad.setColorAt(1, "#f7f7f7");
    palette.setBrush(QPalette::Button, buttonGrad);

    QLinearGradient midlightGrad({0.5, 0.0}, {0.5, 1.0});
    midlightGrad.setCoordinateMode(QGradient::ObjectMode);
    midlightGrad.setColorAt(0, "#f4f4f4");
    midlightGrad.setColorAt(0.1, "#ededed");
    palette.setBrush(QPalette::Midlight, midlightGrad);
    palette.setBrush(QPalette::ButtonText, qApp->palette().buttonText());
    widget->setPalette(palette);
}

void setPanelButtonPaletteRed(QWidget* widget)
{
    QPalette palette(widget->palette());
    QLinearGradient shadowGrad({0.0, 0.5}, {1.0, 0.5});
    shadowGrad.setCoordinateMode(QGradient::ObjectMode);
    shadowGrad.setColorAt(0, "#22202020");
    shadowGrad.setColorAt(0.05, "#20202020");
    shadowGrad.setColorAt(0.5, "#20202020");
    shadowGrad.setColorAt(0.95, "#20202020");
    shadowGrad.setColorAt(1, "#22202020");
    palette.setBrush(QPalette::Shadow, shadowGrad);

    QLinearGradient darkGrad({0.0, 0.0}, {0.0, 1.0});
    darkGrad.setCoordinateMode(QGradient::ObjectMode);
    darkGrad.setColorAt(0.85, "#60303030");
    darkGrad.setColorAt(1, "#7d000000");
    palette.setBrush(QPalette::Dark, darkGrad);

    QLinearGradient midGrad({0.0, 0.0}, {0.0, 1.0});
    midGrad.setCoordinateMode(QGradient::ObjectMode);
    midGrad.setColorAt(0, "#b34b46");
    midGrad.setColorAt(1, "#a2403b");
    palette.setBrush(QPalette::Mid, midGrad);

    QLinearGradient buttonGrad({0.0, 0.0}, {0.0, 1.0});
    buttonGrad.setCoordinateMode(QGradient::ObjectMode);
    buttonGrad.setColorAt(0, "#c2504b");
    buttonGrad.setColorAt(1, "#b34b46");
    palette.setBrush(QPalette::Button, buttonGrad);

    QLinearGradient midlightGrad({0.5, 0.0}, {0.5, 1.0});
    midlightGrad.setCoordinateMode(QGradient::ObjectMode);
    midlightGrad.setColorAt(0, "#932b26");
    midlightGrad.setColorAt(0.1, "#802020");
    palette.setBrush(QPalette::Midlight, midlightGrad);
    palette.setBrush(QPalette::ButtonText, Qt::white);
    widget->setPalette(palette);
}
}

BottomBar::BottomBar(QWidget* parent) : QWidget(parent)
  , m_layout(new QHBoxLayout(this))
  , m_consoleButton(new PushButton(this))
  , m_issuesButton(new PushButton(this))
  , m_showHideLeftPanesButton(new PushButton(this))
  , m_showHideRightPanesButton(new PushButton(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_layout->setSpacing(2);
    m_layout->setContentsMargins(2, 2, 2, 2);
    m_layout->addWidget(m_showHideLeftPanesButton, 0, Qt::AlignVCenter);
    m_layout->addWidget(m_issuesButton, 0, Qt::AlignVCenter);
    m_layout->addWidget(m_consoleButton, 0, Qt::AlignVCenter);
    m_layout->addStretch();
    m_layout->addWidget(m_showHideRightPanesButton, 0, Qt::AlignVCenter);

    auto buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(m_consoleButton);
    buttonGroup->addButton(m_issuesButton);
    buttonGroup->setExclusive(false);
    connect(buttonGroup, qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked),
            this, [=] (QAbstractButton* button) {
        emit buttonActivated(button, button->isChecked());
        if (!button->isChecked())
            return;
        for (QAbstractButton* b : buttonGroup->buttons()) {
            if (b != button) {
                b->setChecked(false);
                emit buttonActivated(b, false);
            }
        }
    });

    setPanelButtonPaletteDarkerShadows(m_consoleButton);
    setPanelButtonPaletteDarkerShadows(m_issuesButton);
    setPanelButtonPaletteDarkerShadows(m_showHideLeftPanesButton);
    setPanelButtonPaletteDarkerShadows(m_showHideRightPanesButton);

    const QColor& iconColor = m_consoleButton->palette().buttonText().color();

    m_consoleButton->setMaximumHeight(20);
    m_consoleButton->setCursor(Qt::PointingHandCursor);
    m_consoleButton->setCheckable(true);
    m_consoleButton->setText(tr("Console Output"));
    m_consoleButton->setToolTip(tr("Activate console output"));
    m_consoleButton->setIconSize({16, 16});
    m_consoleButton->setIcon(renderColorizedIcon(":/images/console.png", iconColor, this));

    m_issuesButton->setMaximumHeight(20);
    m_issuesButton->setCursor(Qt::PointingHandCursor);
    m_issuesButton->setCheckable(true);
    m_issuesButton->setText(tr("Issues"));
    m_issuesButton->setToolTip(tr("Activate issues list"));
    m_issuesButton->setIconSize({16, 16});
    m_issuesButton->setIcon(renderColorizedIcon(":/images/issues.png", iconColor, this));

    m_showHideLeftPanesButton->setMaximumHeight(20);
    m_showHideLeftPanesButton->setCursor(Qt::PointingHandCursor);
    m_showHideLeftPanesButton->setCheckable(true);
    m_showHideLeftPanesButton->setIconSize({16, 16});
    m_showHideLeftPanesButton->setIcon(TOGGLE_LEFT_SIDEBAR_TOOLBAR.icon());

    m_showHideRightPanesButton->setMaximumHeight(20);
    m_showHideRightPanesButton->setCursor(Qt::PointingHandCursor);
    m_showHideRightPanesButton->setCheckable(true);
    m_showHideRightPanesButton->setIconSize({16, 16});
    m_showHideRightPanesButton->setIcon(TOGGLE_RIGHT_SIDEBAR_TOOLBAR.icon());

    connect(m_showHideLeftPanesButton, &PushButton::clicked,
            this, &BottomBar::showHideLeftPanesButtonActivated);
    connect(m_showHideRightPanesButton, &PushButton::clicked,
            this, &BottomBar::showHideRightPanesButtonActivated);
    connect(m_showHideLeftPanesButton, &PushButton::clicked,
            this, &BottomBar::setLeftShowHideButtonToolTip);
    connect(m_showHideRightPanesButton, &PushButton::clicked,
            this, &BottomBar::setRightShowHideButtonToolTip);
}

void BottomBar::flash(QAbstractButton* button)
{
    setPanelButtonPaletteRed(button);
    button->setIcon(renderColorizedIcon(button->icon(), button->iconSize(),
                                        button->palette().buttonText().color(), button));
    QTimer::singleShot(400, this,
                       [=] {
        setPanelButtonPaletteDarkerShadows(button);
        button->setIcon(renderColorizedIcon(button->icon(), button->iconSize(),
                                            button->palette().buttonText().color(), button));
    });
    QTimer::singleShot(800, this,
                       [=] {
        setPanelButtonPaletteRed(button);
        button->setIcon(renderColorizedIcon(button->icon(), button->iconSize(),
                                            button->palette().buttonText().color(), button));
    });
    QTimer::singleShot(1200, this,
                       [=] {
        setPanelButtonPaletteDarkerShadows(button);
        button->setIcon(renderColorizedIcon(button->icon(), button->iconSize(),
                                            button->palette().buttonText().color(), button));
    });
    QTimer::singleShot(1600, this,
                       [=] {
        setPanelButtonPaletteRed(button);
        button->setIcon(renderColorizedIcon(button->icon(), button->iconSize(),
                                            button->palette().buttonText().color(), button));
    });
    QTimer::singleShot(2000, this,
                       [=] {
        setPanelButtonPaletteDarkerShadows(button);
        button->setIcon(renderColorizedIcon(button->icon(), button->iconSize(),
                                            button->palette().buttonText().color(), button));
    });
}

void BottomBar::setLeftShowHideButtonToolTip(bool checked)
{
    if (checked)
        m_showHideLeftPanesButton->setToolTip(QString::fromUtf8(g_tooltip).arg(tr("Hide Left Panes")));
    else
        m_showHideLeftPanesButton->setToolTip(QString::fromUtf8(g_tooltip).arg(tr("Show Left Panes")));
}

void BottomBar::setRightShowHideButtonToolTip(bool checked)
{
    if (checked)
        m_showHideRightPanesButton->setToolTip(QString::fromUtf8(g_tooltip).arg(tr("Hide Right Panes")));
    else
        m_showHideRightPanesButton->setToolTip(QString::fromUtf8(g_tooltip).arg(tr("Show Right Panes")));
}

void BottomBar::sweep()
{
    m_consoleButton->setChecked(false);
    m_issuesButton->setChecked(false);
    m_showHideLeftPanesButton->setChecked(true);
    m_showHideRightPanesButton->setChecked(true);
    setLeftShowHideButtonToolTip(true);
    setRightShowHideButtonToolTip(true);
}

QAbstractButton* BottomBar::activeButton() const
{
    if (m_consoleButton->isChecked())
        return m_consoleButton;
    if (m_issuesButton->isChecked())
        return m_issuesButton;
    return nullptr;
}

QAbstractButton* BottomBar::consoleButton() const
{
    return m_consoleButton;
}

QAbstractButton* BottomBar::issuesButton() const
{
    return m_issuesButton;
}

void BottomBar::paintEvent(QPaintEvent*)
{
    QStylePainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen("#c4c4c4");
    p.drawLine(rect().topLeft() + QPointF(0.5, 0.5),
               rect().topRight() + QPointF(0.5, 0.5));
    p.drawLine(rect().topLeft() + QPointF(0.5, 0.5),
               rect().bottomLeft() + QPointF(0.5, 0.5));
    p.drawLine(rect().topRight() + QPointF(0.5, 0.5),
               rect().bottomRight() + QPointF(0.5, 0.5));
}

QSize BottomBar::minimumSizeHint() const
{
    return QSize(0, 24);
}

QSize BottomBar::sizeHint() const
{
    return QSize(100, 24);
}
