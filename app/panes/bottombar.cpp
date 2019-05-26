#include <bottombar.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <generalsettings.h>
#include <interfacesettings.h>

#include <QHBoxLayout>
#include <QStylePainter>
#include <QButtonGroup>
#include <QApplication>

using namespace PaintUtils;

class ButtonFlasher final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ButtonFlasher)

    static void setBright(QWidget* widget, bool bright)
    {
        QPalette palette(widget->palette());
        palette.setBrush(QPalette::Active, QPalette::ButtonText, bright
                         ? Qt::white : qApp->palette().color(QPalette::Active, QPalette::ButtonText));
        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, bright
                         ? Qt::white : qApp->palette().color(QPalette::Inactive, QPalette::ButtonText));
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, bright
                         ? QColor("#cccccc") : qApp->palette().color(QPalette::Disabled, QPalette::ButtonText));
        widget->setPalette(palette);
        widget->setProperty("ow_bottombar_bright", bright);
    }

    static bool isBright(QWidget* widget)
    {
        return widget->property("ow_bottombar_bright").toBool();
    }

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
        setBright(button, true);
        button->setIcon(renderOverlaidIcon(button->icon(), button->iconSize(),
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
        if (isBright(button))
            setBright(button, false);
        else
            setBright(button, true);

        QIcon icon;
        if (!isBright(button)) {
            QColor up = button->palette().buttonText().color().lighter(130); // not pressed
            QColor down = button->palette().buttonText().color().darker(180); // pressed
            Q_ASSERT(UtilityFunctions::window(button));
            icon.addPixmap(renderOverlaidPixmap(button->icon().pixmap(UtilityFunctions::window(button),
                                                                      button->iconSize()), up, button),
                           QIcon::Normal, QIcon::Off);
            icon.addPixmap(renderOverlaidPixmap(button->icon().pixmap(UtilityFunctions::window(button),
                                                                      button->iconSize()), down, button),
                           QIcon::Normal, QIcon::On);
        } else {
            icon = renderOverlaidIcon(button->icon(), button->iconSize(),
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
  , m_consoleFlasher(new ButtonFlasher(m_consoleButton))
  , m_issuesFlasher(new ButtonFlasher(m_issuesButton))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_layout->setSpacing(4);
    m_layout->setContentsMargins(4, 2, 4, 2);
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

    QIcon consoleIcon;
    QColor up = m_consoleButton->palette().buttonText().color().lighter(130); // not pressed
    QColor down = m_consoleButton->palette().buttonText().color().darker(180); // pressed
    consoleIcon.addPixmap(renderOverlaidPixmap(":/images/console.svg", up, this), QIcon::Normal, QIcon::Off);
    consoleIcon.addPixmap(renderOverlaidPixmap(":/images/console.svg", down, this), QIcon::Normal, QIcon::On);

    m_consoleButton->setMaximumHeight(22);
    m_consoleButton->setCursor(Qt::PointingHandCursor);
    m_consoleButton->setCheckable(true);
    m_consoleButton->setText(tr("Console Output"));
    m_consoleButton->setToolTip(tr("Activate console output"));
    m_consoleButton->setIconSize({14, 14});
    m_consoleButton->setIcon(consoleIcon);

    QIcon issuesIcon;
    up = m_issuesButton->palette().buttonText().color().lighter(130); // not pressed
    down = m_issuesButton->palette().buttonText().color().darker(180); // pressed
    issuesIcon.addPixmap(renderOverlaidPixmap(":/images/issues.svg", up, this), QIcon::Normal, QIcon::Off);
    issuesIcon.addPixmap(renderOverlaidPixmap(":/images/issues.svg", down, this), QIcon::Normal, QIcon::On);

    m_issuesButton->setMaximumHeight(22);
    m_issuesButton->setCursor(Qt::PointingHandCursor);
    m_issuesButton->setCheckable(true);
    m_issuesButton->setText(tr("Issues") + " [0]");
    m_issuesButton->setToolTip(tr("Activate issues list"));
    m_issuesButton->setIconSize({14, 14});
    m_issuesButton->setIcon(issuesIcon);

    QIcon showHideLeftPanesIcon;
    up = palette().buttonText().color().lighter(130); // not pressed
    down = palette().buttonText().color().darker(180); // pressed
    showHideLeftPanesIcon.addPixmap(renderMaskedPixmap(":/utils/images/leftsidebaricon@2x.png",
                                                       up, this), QIcon::Normal, QIcon::Off);
    showHideLeftPanesIcon.addPixmap(renderMaskedPixmap(":/utils/images/leftsidebaricon@2x.png",
                                                       down, this), QIcon::Normal, QIcon::On);

    m_showHideLeftPanesButton->setMaximumHeight(22);
    m_showHideLeftPanesButton->setCursor(Qt::PointingHandCursor);
    m_showHideLeftPanesButton->setCheckable(true);
    m_showHideLeftPanesButton->setIconSize({16, 16});
    m_showHideLeftPanesButton->setIcon(showHideLeftPanesIcon);

    QIcon showHideRightPanesIcon;
    showHideRightPanesIcon.addPixmap(renderMaskedPixmap(":/utils/images/rightsidebaricon@2x.png",
                                                        up, this), QIcon::Normal, QIcon::Off);
    showHideRightPanesIcon.addPixmap(renderMaskedPixmap(":/utils/images/rightsidebaricon@2x.png",
                                                        down, this), QIcon::Normal, QIcon::On);

    m_showHideRightPanesButton->setMaximumHeight(22);
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

QSize BottomBar::sizeHint() const
{
    return {100, 26};
}

QSize BottomBar::minimumSizeHint() const
{
    return {0, 26};
}

void BottomBar::discharge()
{
    m_issuesButton->setChecked(false);
    m_consoleButton->setChecked(false);
    m_showHideLeftPanesButton->setChecked(true);
    m_showHideRightPanesButton->setChecked(true);
    setLeftShowHideButtonToolTip(true);
    setRightShowHideButtonToolTip(true);

    const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    if (settings->visibleBottomPane != "None") {
        if (settings->visibleBottomPane == "Console Pane")
            m_consoleButton->animateClick();
        else
            m_issuesButton->animateClick();
    }
}

void BottomBar::flash(QAbstractButton* button)
{
    if (button == m_consoleButton)
        m_consoleFlasher->flash(400, 3);
    else if (button == m_issuesButton)
        m_issuesFlasher->flash(400, 3);

    const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    if (settings->bottomPanesPop && !button->isChecked())
        button->animateClick();
}

void BottomBar::setLeftShowHideButtonToolTip(bool checked)
{
    if (checked)
        m_showHideLeftPanesButton->setToolTip(UtilityFunctions::toToolTip(tr("Hide left panes")));
    else
        m_showHideLeftPanesButton->setToolTip(UtilityFunctions::toToolTip(tr("Show left panes")));
}

void BottomBar::setRightShowHideButtonToolTip(bool checked)
{
    if (checked)
        m_showHideRightPanesButton->setToolTip(UtilityFunctions::toToolTip(tr("Hide right panes")));
    else
        m_showHideRightPanesButton->setToolTip(UtilityFunctions::toToolTip(tr("Show right panes")));
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

void BottomBar::paintEvent(QPaintEvent*)
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

#include "bottombar.moc"