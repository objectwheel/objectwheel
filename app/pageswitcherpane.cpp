#include <pageswitcherpane.h>
#include <flatbutton.h>
#include <appfontsettings.h>
#include <generalsettings.h>
#include <interfacesettings.h>

#include <QPainter>
#include <QVBoxLayout>

namespace {
const char* g_tooltip = "<span style=\"font-size: 12px !important;\">%1 <b>%2</b></span>";
}

PageSwitcherPane::PageSwitcherPane(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_qmlCodeEditorButton(new FlatButton)
  , m_designerButton(new FlatButton)
  , m_projectOptionsButton(new FlatButton)
  , m_buildsButton(new FlatButton)
  , m_helpButton(new FlatButton)
  , m_splitViewButton(new FlatButton)
{
    setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_designerButton);
    m_layout->addWidget(m_qmlCodeEditorButton);
    m_layout->addWidget(m_splitViewButton);
    m_layout->addWidget(m_projectOptionsButton);
    m_layout->addWidget(m_buildsButton);
    m_layout->addWidget(m_helpButton);
    m_layout->addStretch();

    QFont labelFont;
    labelFont.setWeight(QFont::DemiBold);
    labelFont.setPixelSize(AppFontSettings::defaultPixelSize() - 2.0);

    m_buildsButton->setFont(labelFont);
    m_splitViewButton->setFont(labelFont);
    m_helpButton->setFont(labelFont);
    m_projectOptionsButton->setFont(labelFont);
    m_qmlCodeEditorButton->setFont(labelFont);
    m_designerButton->setFont(labelFont);

    m_designerButton->setText(tr("Designer"));
    m_qmlCodeEditorButton->setText(tr("Editor"));
    m_projectOptionsButton->setText(tr("Options"));
    m_helpButton->setText(tr("Help"));
    m_splitViewButton->setText(tr("Split View"));
    m_buildsButton->setText(tr("Builds"));

    m_designerButton->setToolTip(QString::fromUtf8(g_tooltip).arg(tr("Open")).arg(tr("Designer")));
    m_qmlCodeEditorButton->setToolTip(QString::fromUtf8(g_tooltip).arg(tr("Open")).arg(tr("Qml Code Editor")));
    m_projectOptionsButton->setToolTip(QString::fromUtf8(g_tooltip).arg(tr("Open")).arg(tr("Project Options")));
    m_helpButton->setToolTip(QString::fromUtf8(g_tooltip).arg(tr("Open")).arg(tr("Help and Documentations")));
    m_splitViewButton->setToolTip(QString::fromUtf8(g_tooltip).arg(tr("Open")).arg(tr("Splitted View")));
    m_buildsButton->setToolTip(QString::fromUtf8(g_tooltip).arg(tr("Open")).arg(tr("Cloud Builds")));

    m_designerButton->setAutoExclusive(true);
    m_qmlCodeEditorButton->setAutoExclusive(true);
    m_projectOptionsButton->setAutoExclusive(true);
    m_helpButton->setAutoExclusive(true);
    m_splitViewButton->setAutoExclusive(true);
    m_buildsButton->setAutoExclusive(true);

    m_designerButton->setFixedHeight(60);
    m_qmlCodeEditorButton->setFixedHeight(60);
    m_projectOptionsButton->setFixedHeight(60);
    m_helpButton->setFixedHeight(60);
    m_splitViewButton->setFixedHeight(60);
    m_buildsButton->setFixedHeight(60);

    m_designerButton->setCheckable(true);
    m_qmlCodeEditorButton->setCheckable(true);
    m_projectOptionsButton->setCheckable(true);
    m_helpButton->setCheckable(true);
    m_splitViewButton->setCheckable(true);
    m_buildsButton->setCheckable(true);

    m_designerButton->setIconSize(QSize(23, 23));
    m_qmlCodeEditorButton->setIconSize(QSize(23, 23));
    m_projectOptionsButton->setIconSize(QSize(23, 23));
    m_helpButton->setIconSize(QSize(23, 23));
    m_splitViewButton->setIconSize(QSize(23, 23));
    m_buildsButton->setIconSize(QSize(23, 23));

    m_designerButton->setIcon(QIcon(":/images/designer.png"));
    m_qmlCodeEditorButton->setIcon(QIcon(":/images/editor.png"));
    m_projectOptionsButton->setIcon(QIcon(":/images/projectoptions.png"));
    m_helpButton->setIcon(QIcon(":/images/help.png"));
    m_splitViewButton->setIcon(QIcon(":/images/split.png"));
    m_buildsButton->setIcon(QIcon(":/images/helmet.png"));

    m_designerButton->settings().showShadow = false;
    m_qmlCodeEditorButton->settings().showShadow = false;
    m_projectOptionsButton->settings().showShadow = false;
    m_helpButton->settings().showShadow = false;
    m_splitViewButton->settings().showShadow = false;
    m_buildsButton->settings().showShadow = false;

    m_designerButton->settings().textUnderIcon = true;
    m_qmlCodeEditorButton->settings().textUnderIcon = true;
    m_projectOptionsButton->settings().textUnderIcon = true;
    m_helpButton->settings().textUnderIcon = true;
    m_splitViewButton->settings().textUnderIcon = true;
    m_buildsButton->settings().textUnderIcon = true;

    m_designerButton->settings().verticalGradient = false;
    m_qmlCodeEditorButton->settings().verticalGradient = false;
    m_projectOptionsButton->settings().verticalGradient = false;
    m_helpButton->settings().verticalGradient = false;
    m_splitViewButton->settings().verticalGradient = false;
    m_buildsButton->settings().verticalGradient = false;

    m_designerButton->settings().textMargin = 0;
    m_qmlCodeEditorButton->settings().textMargin = 0;
    m_projectOptionsButton->settings().textMargin = 0;
    m_helpButton->settings().textMargin = 0;
    m_splitViewButton->settings().textMargin = 0;
    m_buildsButton->settings().textMargin = 0;

    updateColors();

    connect(m_designerButton, &FlatButton::pressed,
            this, [=] { setCurrentPage(Page_Designer); });
    connect(m_qmlCodeEditorButton, &FlatButton::pressed,
            this, [=] { setCurrentPage(Page_QmlCodeEditor); });
    connect(m_projectOptionsButton, &FlatButton::pressed,
            this, [=] { setCurrentPage(Page_ProjectOptions); });
    connect(m_helpButton, &FlatButton::pressed,
            this, [=] { setCurrentPage(Page_Help); });
    connect(m_splitViewButton, &FlatButton::pressed,
            this, [=] { setCurrentPage(Page_SplitView); });
    connect(m_buildsButton, &FlatButton::pressed,
            this, [=] { setCurrentPage(Page_Builds); });
    connect(GeneralSettings::interfaceSettings(), &InterfaceSettings::changed,
            this, &PageSwitcherPane::updateColors);
}

Pages PageSwitcherPane::currentPage() const
{
    if (m_buildsButton->isChecked())
        return Page_Builds;
    if (m_designerButton->isChecked())
        return Page_Designer;
    if (m_splitViewButton->isChecked())
        return Page_SplitView;
    if (m_helpButton->isChecked())
        return Page_Help;
    if (m_qmlCodeEditorButton->isChecked())
        return Page_QmlCodeEditor;
    else
        return Page_ProjectOptions;
}

bool PageSwitcherPane::isPageEnabled(const Pages& page) const
{
    switch (page) {
    case Page_Builds:
        return m_buildsButton->isEnabled();

    case Page_Designer:
        return m_designerButton->isEnabled();

    case Page_SplitView:
        return m_splitViewButton->isEnabled();

    case Page_Help:
        return m_helpButton->isEnabled();

    case Page_QmlCodeEditor:
        return m_qmlCodeEditorButton->isEnabled();

    case Page_ProjectOptions:
        return m_projectOptionsButton->isEnabled();
    default:
        Q_ASSERT(false);
        return false;
    }
}

void PageSwitcherPane::setCurrentPage(const Pages& page)
{
    switch (page) {
    case Page_Builds:
        m_buildsButton->setChecked(true);
        emit buildsActivated();
        emit currentPageChanged(Page_Builds);
        break;

    case Page_Designer:
        m_designerButton->setChecked(true);
        emit designerActivated();
        emit currentPageChanged(Page_Designer);
        break;

    case Page_SplitView:
        m_splitViewButton->setChecked(true);
        emit splitViewActivated();
        emit currentPageChanged(Page_SplitView);
        break;

    case Page_Help:
        m_helpButton->setChecked(true);
        emit helpActivated();
        emit currentPageChanged(Page_Help);
        break;

    case Page_QmlCodeEditor:
        m_qmlCodeEditorButton->setChecked(true);
        emit qmlCodeEditorActivated();
        emit currentPageChanged(Page_QmlCodeEditor);
        break;

    case Page_ProjectOptions:
        m_projectOptionsButton->setChecked(true);
        emit projectOptionsActivated();
        emit currentPageChanged(Page_ProjectOptions);
        break;
    }
}

void PageSwitcherPane::setPageEnabled(const Pages& page)
{
    switch (page) {
    case Page_Builds:
        return m_buildsButton->setEnabled(true);

    case Page_Designer:
        return m_designerButton->setEnabled(true);

    case Page_SplitView:
        return m_splitViewButton->setEnabled(true);

    case Page_Help:
        return m_helpButton->setEnabled(true);

    case Page_QmlCodeEditor:
        return m_qmlCodeEditorButton->setEnabled(true);

    case Page_ProjectOptions:
        return m_projectOptionsButton->setEnabled(true);
    }
}

void PageSwitcherPane::setPageDisabled(const Pages& page)
{
    switch (page) {
    case Page_Builds:
        return m_buildsButton->setDisabled(true);

    case Page_Designer:
        return m_designerButton->setDisabled(true);

    case Page_SplitView:
        return m_splitViewButton->setDisabled(true);

    case Page_Help:
        return m_helpButton->setDisabled(true);

    case Page_QmlCodeEditor:
        return m_qmlCodeEditorButton->setDisabled(true);

    case Page_ProjectOptions:
        return m_projectOptionsButton->setDisabled(true);
    }
}

void PageSwitcherPane::updateColors()
{
    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    const QColor& lighter = settings->leftBarColor.lighter(106);
    const QColor& darker = settings->leftBarColor.darker(110);

    m_buildsButton->settings().textColor = Qt::white;
    m_splitViewButton->settings().textColor = Qt::white;
    m_helpButton->settings().textColor = Qt::white;
    m_projectOptionsButton->settings().textColor = Qt::white;
    m_qmlCodeEditorButton->settings().textColor = Qt::white;
    m_designerButton->settings().textColor = Qt::white;

    m_designerButton->settings().topColor = lighter;
    m_qmlCodeEditorButton->settings().topColor = lighter;
    m_projectOptionsButton->settings().topColor = lighter;
    m_helpButton->settings().topColor = lighter;
    m_splitViewButton->settings().topColor = lighter;
    m_buildsButton->settings().topColor = lighter;

    m_designerButton->settings().bottomColor = darker;
    m_qmlCodeEditorButton->settings().bottomColor = darker;
    m_projectOptionsButton->settings().bottomColor = darker;
    m_helpButton->settings().bottomColor = darker;
    m_splitViewButton->settings().bottomColor = darker;
    m_buildsButton->settings().bottomColor = darker;

    update();
}

void PageSwitcherPane::discharge()
{
    setCurrentPage(Page_Designer);
}

void PageSwitcherPane::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    QLinearGradient gradient(QRectF(rect()).topRight(), QRectF(rect()).topLeft());
    gradient.setColorAt(0, settings->leftBarColor.lighter(106));
    gradient.setColorAt(1, settings->leftBarColor.darker(110));
    painter.fillRect(rect(), gradient);
}