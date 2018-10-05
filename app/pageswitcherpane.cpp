#include <pageswitcherpane.h>
#include <flatbutton.h>
#include <appfontsettings.h>
#include <utilsicons.h>

#include <QPainter>
#include <QVBoxLayout>

namespace {
const QString TOOLTIP = QObject::tr("<span style=\"font-size: 12px !important;\">Open <b>%1</b></span>");
const QString TOOLTIP_2 = QObject::tr("<span style=\"font-size: 12px !important;\">%1</span>");
}

using namespace Utils;
using namespace Icons;

PageSwitcherPane::PageSwitcherPane(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_qmlCodeEditorButton(new FlatButton)
  , m_designerButton(new FlatButton)
  , m_projectOptionsButton(new FlatButton)
  , m_buildsButton(new FlatButton)
  , m_helpButton(new FlatButton)
  , m_splitViewButton(new FlatButton)
  , m_hideShowLeftPanesButton(new FlatButton(this))
  , m_hideShowRightPanesButton(new FlatButton(this))
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

    m_designerButton->setToolTip(TOOLTIP.arg(tr("Designer")));
    m_qmlCodeEditorButton->setToolTip(TOOLTIP.arg(tr("Qml Code Editor")));
    m_projectOptionsButton->setToolTip(TOOLTIP.arg(tr("Project Options")));
    m_helpButton->setToolTip(TOOLTIP.arg(tr("Help and Documentations")));
    m_splitViewButton->setToolTip(TOOLTIP.arg(tr("Splitted View")));
    m_buildsButton->setToolTip(TOOLTIP.arg(tr("Cloud Builds")));

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
    m_hideShowLeftPanesButton->setFixedHeight(22);
    m_hideShowRightPanesButton->setFixedHeight(22);
    m_hideShowLeftPanesButton->setFixedWidth(35);
    m_hideShowRightPanesButton->setFixedWidth(35);

    m_designerButton->setCheckable(true);
    m_qmlCodeEditorButton->setCheckable(true);
    m_projectOptionsButton->setCheckable(true);
    m_helpButton->setCheckable(true);
    m_splitViewButton->setCheckable(true);
    m_buildsButton->setCheckable(true);
    m_hideShowLeftPanesButton->setCheckable(true);
    m_hideShowRightPanesButton->setCheckable(true);

    m_designerButton->setIconSize(QSize(23, 23));
    m_qmlCodeEditorButton->setIconSize(QSize(23, 23));
    m_projectOptionsButton->setIconSize(QSize(23, 23));
    m_helpButton->setIconSize(QSize(23, 23));
    m_splitViewButton->setIconSize(QSize(23, 23));
    m_buildsButton->setIconSize(QSize(23, 23));
    m_hideShowLeftPanesButton->setIconSize(QSize(20, 20));
    m_hideShowRightPanesButton->setIconSize(QSize(20, 20));

    m_designerButton->setIcon(QIcon(":/images/designer.png"));
    m_qmlCodeEditorButton->setIcon(QIcon(":/images/editor.png"));
    m_projectOptionsButton->setIcon(QIcon(":/images/projectoptions.png"));
    m_helpButton->setIcon(QIcon(":/images/help.png"));
    m_splitViewButton->setIcon(QIcon(":/images/split.png"));
    m_buildsButton->setIcon(QIcon(":/images/helmet.png"));
    m_hideShowLeftPanesButton->setIcon(Icon({{TOGGLE_LEFT_SIDEBAR_TOOLBAR.imageFileName(),
                                                   Theme::FancyToolButtonSelectedColor}}).icon());
    m_hideShowRightPanesButton->setIcon(Icon({{TOGGLE_RIGHT_SIDEBAR_TOOLBAR.imageFileName(),
                                                    Theme::FancyToolButtonSelectedColor}}).icon());

    m_designerButton->settings().showShadow = false;
    m_qmlCodeEditorButton->settings().showShadow = false;
    m_projectOptionsButton->settings().showShadow = false;
    m_helpButton->settings().showShadow = false;
    m_splitViewButton->settings().showShadow = false;
    m_buildsButton->settings().showShadow = false;
    m_hideShowLeftPanesButton->settings().showShadow = false;
    m_hideShowRightPanesButton->settings().showShadow = false;

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
    m_hideShowLeftPanesButton->settings().textMargin = 0;
    m_hideShowRightPanesButton->settings().textMargin = 0;

    m_buildsButton->settings().textColor = Qt::white;
    m_splitViewButton->settings().textColor = Qt::white;
    m_helpButton->settings().textColor = Qt::white;
    m_projectOptionsButton->settings().textColor = Qt::white;
    m_qmlCodeEditorButton->settings().textColor = Qt::white;
    m_designerButton->settings().textColor = Qt::white;

    m_designerButton->settings().topColor = "#384047";
    m_qmlCodeEditorButton->settings().topColor = "#384047";
    m_projectOptionsButton->settings().topColor = "#384047";
    m_helpButton->settings().topColor = "#384047";
    m_splitViewButton->settings().topColor = "#384047";
    m_buildsButton->settings().topColor = "#384047";
    m_hideShowLeftPanesButton->settings().topColor = "#384047";
    m_hideShowRightPanesButton->settings().topColor = "#384047";

    m_designerButton->settings().bottomColor = "#424c54";
    m_qmlCodeEditorButton->settings().bottomColor = "#424c54";
    m_projectOptionsButton->settings().bottomColor = "#424c54";
    m_helpButton->settings().bottomColor = "#424c54";
    m_splitViewButton->settings().bottomColor = "#424c54";
    m_buildsButton->settings().bottomColor = "#424c54";
    m_hideShowLeftPanesButton->settings().bottomColor = "#424c54";
    m_hideShowRightPanesButton->settings().bottomColor = "#424c54";

    connect(m_designerButton, &FlatButton::pressed, [=] { setCurrentPage(Page_Designer); });
    connect(m_qmlCodeEditorButton, &FlatButton::pressed, [=] { setCurrentPage(Page_QmlCodeEditor); });
    connect(m_projectOptionsButton, &FlatButton::pressed, [=] { setCurrentPage(Page_ProjectOptions); });
    connect(m_helpButton, &FlatButton::pressed, [=] { setCurrentPage(Page_Help); });
    connect(m_splitViewButton, &FlatButton::pressed, [=] { setCurrentPage(Page_SplitView); });
    connect(m_buildsButton, &FlatButton::pressed, [=] { setCurrentPage(Page_Builds); });
    connect(m_hideShowLeftPanesButton, &FlatButton::toggled,
            this, &PageSwitcherPane::leftPanesShowChanged);
    connect(m_hideShowRightPanesButton, &FlatButton::toggled,
            this, &PageSwitcherPane::rightPanesShowChanged);
    connect(this, &PageSwitcherPane::leftPanesShowChanged,
            this, &PageSwitcherPane::changeLeftShowHideButtonToolTip);
    connect(this, &PageSwitcherPane::rightPanesShowChanged,
            this, &PageSwitcherPane::changeRightShowHideButtonToolTip);
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
        m_hideShowLeftPanesButton->setEnabled(false);
        m_hideShowRightPanesButton->setEnabled(false);
        emit buildsActivated();
        emit currentPageChanged(Page_Builds);
        break;

    case Page_Designer:
        m_designerButton->setChecked(true);
        m_hideShowLeftPanesButton->setEnabled(true);
        m_hideShowRightPanesButton->setEnabled(true);
        emit designerActivated();
        emit currentPageChanged(Page_Designer);
        break;

    case Page_SplitView:
        m_splitViewButton->setChecked(true);
        m_hideShowLeftPanesButton->setEnabled(true);
        m_hideShowRightPanesButton->setEnabled(true);
        emit splitViewActivated();
        emit currentPageChanged(Page_SplitView);
        break;

    case Page_Help:
        m_helpButton->setChecked(true);
        m_hideShowLeftPanesButton->setEnabled(false);
        m_hideShowRightPanesButton->setEnabled(false);
        emit helpActivated();
        emit currentPageChanged(Page_Help);
        break;

    case Page_QmlCodeEditor:
        m_qmlCodeEditorButton->setChecked(true);
        m_hideShowLeftPanesButton->setEnabled(false);
        m_hideShowRightPanesButton->setEnabled(false);
        emit qmlCodeEditorActivated();
        emit currentPageChanged(Page_QmlCodeEditor);
        break;

    case Page_ProjectOptions:
        m_projectOptionsButton->setChecked(true);
        m_hideShowLeftPanesButton->setEnabled(false);
        m_hideShowRightPanesButton->setEnabled(false);
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

void PageSwitcherPane::setRightPanesShow(bool value)
{
    if (m_hideShowRightPanesButton->isEnabled() && m_hideShowRightPanesButton->isChecked() != value)
        m_hideShowRightPanesButton->setChecked(value);
}

void PageSwitcherPane::setLeftPanesShow(bool value)
{
    if (m_hideShowLeftPanesButton->isEnabled() && m_hideShowLeftPanesButton->isChecked() != value)
        m_hideShowLeftPanesButton->setChecked(value);
}

void PageSwitcherPane::sweep()
{
    setCurrentPage(Page_Designer);
    setRightPanesShow(true);
    setLeftPanesShow(true);
}

void PageSwitcherPane::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient gradient(QRectF(rect()).topRight(), QRectF(rect()).topLeft());
    gradient.setColorAt(0, "#384047");
    gradient.setColorAt(1, "#424c54");

    painter.fillRect(rect(), gradient);
    painter.setPen("#2f353c");
    painter.drawLine(QRectF(rect()).topRight() + QPointF(-0.5, 0.5),
                     QRectF(rect()).bottomRight() + QPointF(-0.5, -0.5));
}

void PageSwitcherPane::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    m_hideShowLeftPanesButton->move(rect().center().x() - m_hideShowLeftPanesButton->width(),
                                         rect().bottom() - m_hideShowLeftPanesButton->height());
    m_hideShowRightPanesButton->move(rect().center().x(),
                                          rect().bottom() - m_hideShowRightPanesButton->height());
}

void PageSwitcherPane::changeLeftShowHideButtonToolTip(bool showed)
{
    if (showed)
        m_hideShowLeftPanesButton->setToolTip(TOOLTIP_2.arg(tr("Hide Left Panes")));
    else
        m_hideShowLeftPanesButton->setToolTip(TOOLTIP_2.arg(tr("Show Left Panes")));
}

void PageSwitcherPane::changeRightShowHideButtonToolTip(bool showed)
{
    if (showed)
        m_hideShowRightPanesButton->setToolTip(TOOLTIP_2.arg(tr("Hide Right Panes")));
    else
        m_hideShowRightPanesButton->setToolTip(TOOLTIP_2.arg(tr("Show Right Panes")));
}
