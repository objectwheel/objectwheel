#include <bottombar.h>
#include <pushbutton.h>
#include <paintutils.h>
#include <utilsicons.h>
#include <utilityfunctions.h>
#include <generalsettings.h>
#include <interfacesettings.h>

#include <QHBoxLayout>
#include <QStylePainter>
#include <QStyleOption>
#include <QButtonGroup>
#include <QApplication>

class ButtonFlasher;

using namespace PaintUtils;
using namespace Utils;
using namespace Utils::Icons;

namespace {
const char* g_tooltip = "<span style=\"font-size: 12px !important;\">%1</span>";
ButtonFlasher* g_consoleFlasher;
ButtonFlasher* g_issuesFlasher;

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
    widget->setProperty("ow_flashed", false);
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
    midlightGrad.setColorAt(0.1, "#cc5650");
    midlightGrad.setColorAt(0, "#e5615a");
    palette.setBrush(QPalette::Midlight, midlightGrad);
    palette.setBrush(QPalette::ButtonText, Qt::white);
    widget->setProperty("ow_flashed", true);
    widget->setPalette(palette);
}
}

class ButtonFlasher : public QObject
{
    Q_OBJECT
public:
    explicit ButtonFlasher(QAbstractButton* button) : QObject(button)
      , m_id(0)
    {
    }

    void flash(int timeout, int repeat)
    {
        if (m_id > 0)
            killTimer(m_id);

        m_repeat = 2 * repeat - 1;
        QAbstractButton* button = qobject_cast<QAbstractButton*>(parent());
        setPanelButtonPaletteRed(button);
        button->setIcon(renderColorizedIcon(button->icon(), button->iconSize(),
                                            button->palette().buttonText().color(), button));
        m_id = startTimer(timeout);
    }

private:
    void timerEvent(QTimerEvent*) override
    {
        if (!(m_repeat--)) {
            killTimer(m_id);
            m_id = 0;
            return;
        }

        QAbstractButton* button = qobject_cast<QAbstractButton*>(parent());
        if (button->property("ow_flashed").toBool())
            setPanelButtonPaletteDarkerShadows(button);
        else
            setPanelButtonPaletteRed(button);

        QIcon icon;
        if (!button->property("ow_flashed").toBool()) {
            QColor on = button->palette().buttonText().color().darker(180); // pressed
            QColor off = button->palette().buttonText().color().lighter(130); // not pressed
            icon.addPixmap(renderColorizedPixmap(button->icon().pixmap(UtilityFunctions::window(button),
                button->iconSize()), on, button), QIcon::Normal, QIcon::On);
            icon.addPixmap(renderColorizedPixmap(button->icon().pixmap(UtilityFunctions::window(button),
                button->iconSize()), off, button), QIcon::Normal, QIcon::Off);
        } else {
            icon = renderColorizedIcon(button->icon(), button->iconSize(),
                                       button->palette().buttonText().color(), button);
        }
        button->setIcon(icon);
    }
private:
    int m_repeat;
    int m_id;
};

BottomBar::BottomBar(QWidget* parent) : QWidget(parent)
  , m_layout(new QHBoxLayout(this))
  , m_buttonGroup(new QButtonGroup(this))
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

    m_buttonGroup->addButton(m_consoleButton);
    m_buttonGroup->addButton(m_issuesButton);
    m_buttonGroup->setExclusive(false);
    connect(m_buttonGroup, qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked),
            this, &BottomBar::onButtonClick);

    setPanelButtonPaletteDarkerShadows(m_consoleButton);
    setPanelButtonPaletteDarkerShadows(m_issuesButton);
    setPanelButtonPaletteDarkerShadows(m_showHideLeftPanesButton);
    setPanelButtonPaletteDarkerShadows(m_showHideRightPanesButton);

    QIcon consoleIcon;
    QColor on = m_consoleButton->palette().buttonText().color().darker(180); // pressed
    QColor off = m_consoleButton->palette().buttonText().color().lighter(130); // not pressed
    consoleIcon.addPixmap(renderColorizedPixmap(":/images/console.png", on, this), QIcon::Normal, QIcon::On);
    consoleIcon.addPixmap(renderColorizedPixmap(":/images/console.png", off, this), QIcon::Normal, QIcon::Off);

    m_consoleButton->setMaximumHeight(20);
    m_consoleButton->setCursor(Qt::PointingHandCursor);
    m_consoleButton->setCheckable(true);
    m_consoleButton->setText(tr("Console Output"));
    m_consoleButton->setToolTip(tr("Activate console output"));
    m_consoleButton->setIconSize({16, 16});
    m_consoleButton->setIcon(consoleIcon);

    QIcon issuesIcon;
    on = m_issuesButton->palette().buttonText().color().darker(180); // pressed
    off = m_issuesButton->palette().buttonText().color().lighter(130); // not pressed
    issuesIcon.addPixmap(renderColorizedPixmap(":/images/issues.png", on, this), QIcon::Normal, QIcon::On);
    issuesIcon.addPixmap(renderColorizedPixmap(":/images/issues.png", off, this), QIcon::Normal, QIcon::Off);

    m_issuesButton->setMaximumHeight(20);
    m_issuesButton->setCursor(Qt::PointingHandCursor);
    m_issuesButton->setCheckable(true);
    m_issuesButton->setText(tr("Issues") + " [0]");
    m_issuesButton->setToolTip(tr("Activate issues list"));
    m_issuesButton->setIconSize({16, 16});
    m_issuesButton->setIcon(issuesIcon);

    g_consoleFlasher = new ButtonFlasher(m_consoleButton);
    g_issuesFlasher = new ButtonFlasher(m_issuesButton);

    QIcon showHideLeftPanesIcon;
    on = palette().buttonText().color().darker(180); // pressed
    off = palette().buttonText().color().lighter(130); // not pressed
    showHideLeftPanesIcon.addPixmap(renderMaskedPixmap(":/utils/images/leftsidebaricon@2x.png",
        on, this), QIcon::Normal, QIcon::On);
    showHideLeftPanesIcon.addPixmap(renderMaskedPixmap(":/utils/images/leftsidebaricon@2x.png",
        off, this), QIcon::Normal, QIcon::Off);

    m_showHideLeftPanesButton->setMaximumHeight(20);
    m_showHideLeftPanesButton->setCursor(Qt::PointingHandCursor);
    m_showHideLeftPanesButton->setCheckable(true);
    m_showHideLeftPanesButton->setIconSize({16, 16});
    m_showHideLeftPanesButton->setIcon(showHideLeftPanesIcon);

    QIcon showHideRightPanesIcon;
    showHideRightPanesIcon.addPixmap(renderMaskedPixmap(":/utils/images/rightsidebaricon@2x.png",
        on, this), QIcon::Normal, QIcon::On);
    showHideRightPanesIcon.addPixmap(renderMaskedPixmap(":/utils/images/rightsidebaricon@2x.png",
        off, this), QIcon::Normal, QIcon::Off);

    m_showHideRightPanesButton->setMaximumHeight(20);
    m_showHideRightPanesButton->setCursor(Qt::PointingHandCursor);
    m_showHideRightPanesButton->setCheckable(true);
    m_showHideRightPanesButton->setIconSize({16, 16});
    m_showHideRightPanesButton->setIcon(showHideRightPanesIcon);

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
    if (button == m_consoleButton)
        g_consoleFlasher->flash(400, 3);
    else if (button == m_issuesButton)
        g_issuesFlasher->flash(400, 3);

    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    if (settings->bottomPanesPop && !button->isChecked())
        button->animateClick();
}

void BottomBar::setLeftShowHideButtonToolTip(bool checked)
{
    if (checked)
        m_showHideLeftPanesButton->setToolTip(QString::fromUtf8(g_tooltip).arg(tr("Hide left panes")));
    else
        m_showHideLeftPanesButton->setToolTip(QString::fromUtf8(g_tooltip).arg(tr("Show left panes")));
}

void BottomBar::setRightShowHideButtonToolTip(bool checked)
{
    if (checked)
        m_showHideRightPanesButton->setToolTip(QString::fromUtf8(g_tooltip).arg(tr("Hide right panes")));
    else
        m_showHideRightPanesButton->setToolTip(QString::fromUtf8(g_tooltip).arg(tr("Show right panes")));
}

void BottomBar::onButtonClick(QAbstractButton* button)
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

void BottomBar::discharge()
{
    m_issuesButton->setChecked(false);
    m_consoleButton->setChecked(false);
    m_showHideLeftPanesButton->setChecked(true);
    m_showHideRightPanesButton->setChecked(true);
    setLeftShowHideButtonToolTip(true);
    setRightShowHideButtonToolTip(true);

    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    if (settings->visibleBottomPane != "None") {
        if (settings->visibleBottomPane == "Console Pane")
            m_consoleButton->animateClick();
        else
            m_issuesButton->animateClick();
    }
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
    p.setBrush(Qt::NoBrush);
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

#include "bottombar.moc"