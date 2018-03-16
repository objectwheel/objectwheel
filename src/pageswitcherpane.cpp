#include <pageswitcherpane.h>
#include <flatbutton.h>
#include <fit.h>

#include <QPainter>
#include <QVBoxLayout>

PageSwitcherPane::PageSwitcherPane(QWidget *parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_qmlCodeEditorButton(new FlatButton)
  , m_designerButton(new FlatButton)
  , m_projectOptionsButton(new FlatButton)
  , m_buildsButton(new FlatButton)
  , m_documentsButton(new FlatButton)
  , m_splitViewButton(new FlatButton)
{
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_designerButton);
    m_layout->addWidget(m_qmlCodeEditorButton);
    m_layout->addWidget(m_projectOptionsButton);
    m_layout->addWidget(m_documentsButton);
    m_layout->addWidget(m_splitViewButton);
    m_layout->addStretch();
    m_layout->addWidget(m_buildsButton);

    QFont labelFont;
    labelFont.setWeight(QFont::DemiBold);
    labelFont.setPixelSize(labelFont.pixelSize() - fit::fx(1.5));

    m_designerButton->setText(tr("Designer"));
    m_qmlCodeEditorButton->setText(tr("Editor"));
    m_projectOptionsButton->setText(tr("Options"));
    m_documentsButton->setText(tr("Help"));
    m_splitViewButton->setText(tr("Split View"));
    m_buildsButton->setText(tr("Builds"));

    m_buildsButton->setFont(labelFont);
    m_splitViewButton->setFont(labelFont);
    m_documentsButton->setFont(labelFont);
    m_projectOptionsButton->setFont(labelFont);
    m_qmlCodeEditorButton->setFont(labelFont);
    m_designerButton->setFont(labelFont);

    m_designerButton->setAutoExclusive(true);
    m_qmlCodeEditorButton->setAutoExclusive(true);
    m_projectOptionsButton->setAutoExclusive(true);
    m_documentsButton->setAutoExclusive(true);
    m_splitViewButton->setAutoExclusive(true);
    m_buildsButton->setAutoExclusive(true);

    m_designerButton->setFixedHeight(fit::fx(55));
    m_qmlCodeEditorButton->setFixedHeight(fit::fx(55));
    m_projectOptionsButton->setFixedHeight(fit::fx(55));
    m_documentsButton->setFixedHeight(fit::fx(55));
    m_splitViewButton->setFixedHeight(fit::fx(55));
    m_buildsButton->setFixedHeight(fit::fx(55));

    m_designerButton->setCheckable(true);
    m_qmlCodeEditorButton->setCheckable(true);
    m_projectOptionsButton->setCheckable(true);
    m_documentsButton->setCheckable(true);
    m_splitViewButton->setCheckable(true);
    m_buildsButton->setCheckable(true);

    m_designerButton->setIconSize(fit::fx(QSizeF(30, 30)).toSize());
    m_qmlCodeEditorButton->setIconSize(fit::fx(QSizeF(30, 30)).toSize());
    m_projectOptionsButton->setIconSize(fit::fx(QSizeF(30, 30)).toSize());
    m_documentsButton->setIconSize(fit::fx(QSizeF(30, 30)).toSize());
    m_splitViewButton->setIconSize(fit::fx(QSizeF(30, 30)).toSize());
    m_buildsButton->setIconSize(fit::fx(QSizeF(30, 30)).toSize());

    m_designerButton->setIcon(QIcon(":/resources/images/designer.png"));
    m_qmlCodeEditorButton->setIcon(QIcon(":/resources/images/editor.png"));
    m_projectOptionsButton->setIcon(QIcon(":/resources/images/projectoptions.png"));
    m_documentsButton->setIcon(QIcon(":/resources/images/help.png"));
    m_splitViewButton->setIcon(QIcon(":/resources/images/split.png"));
    m_buildsButton->setIcon(QIcon(":/resources/images/helmet.png"));

    m_designerButton->settings().showShadow = false;
    m_qmlCodeEditorButton->settings().showShadow = false;
    m_projectOptionsButton->settings().showShadow = false;
    m_documentsButton->settings().showShadow = false;
    m_splitViewButton->settings().showShadow = false;
    m_buildsButton->settings().showShadow = false;

    m_designerButton->settings().textUnderIcon = true;
    m_qmlCodeEditorButton->settings().textUnderIcon = true;
    m_projectOptionsButton->settings().textUnderIcon = true;
    m_documentsButton->settings().textUnderIcon = true;
    m_splitViewButton->settings().textUnderIcon = true;
    m_buildsButton->settings().textUnderIcon = true;

    m_designerButton->settings().verticalGradient = false;
    m_qmlCodeEditorButton->settings().verticalGradient = false;
    m_projectOptionsButton->settings().verticalGradient = false;
    m_documentsButton->settings().verticalGradient = false;
    m_splitViewButton->settings().verticalGradient = false;
    m_buildsButton->settings().verticalGradient = false;

    m_buildsButton->settings().textColor = "#F5F9FC";
    m_splitViewButton->settings().textColor = "#F5F9FC";
    m_documentsButton->settings().textColor = "#F5F9FC";
    m_projectOptionsButton->settings().textColor = "#F5F9FC";
    m_qmlCodeEditorButton->settings().textColor = "#F5F9FC";
    m_designerButton->settings().textColor = "#F5F9FC";

    m_designerButton->settings().topColor = "#6f7e8c";
    m_qmlCodeEditorButton->settings().topColor = "#6f7e8c";
    m_projectOptionsButton->settings().topColor = "#6f7e8c";
    m_documentsButton->settings().topColor = "#6f7e8c";
    m_splitViewButton->settings().topColor = "#6f7e8c";
    m_buildsButton->settings().topColor = "#6f7e8c";

    m_designerButton->settings().bottomColor = "#475059";
    m_qmlCodeEditorButton->settings().bottomColor = "#475059";
    m_projectOptionsButton->settings().bottomColor = "#475059";
    m_documentsButton->settings().bottomColor = "#475059";
    m_splitViewButton->settings().bottomColor = "#475059";
    m_buildsButton->settings().bottomColor = "#475059";

    connect(m_designerButton, &FlatButton::pressed, [=] { setCurrentPage(Page_Designer); });
    connect(m_qmlCodeEditorButton, &FlatButton::pressed, [=] { setCurrentPage(Page_QmlCodeEditor); });
    connect(m_projectOptionsButton, &FlatButton::pressed, [=] { setCurrentPage(Page_ProjectOptions); });
    connect(m_documentsButton, &FlatButton::pressed, [=] { setCurrentPage(Page_Help); });
    connect(m_splitViewButton, &FlatButton::pressed, [=] { setCurrentPage(Page_SplitView); });
    connect(m_buildsButton, &FlatButton::pressed, [=] { setCurrentPage(Page_Builds); });
}

Pages PageSwitcherPane::currentPage() const
{
    if (m_buildsButton->isChecked())
        return Page_Builds;
    if (m_designerButton->isChecked())
        return Page_Designer;
    if (m_splitViewButton->isChecked())
        return Page_SplitView;
    if (m_documentsButton->isChecked())
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
            return m_documentsButton->isEnabled();

        case Page_QmlCodeEditor:
            return m_qmlCodeEditorButton->isEnabled();

        case Page_ProjectOptions:
            return m_projectOptionsButton->isEnabled();
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
            m_documentsButton->setChecked(true);
            emit documentsActivated();
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
            return m_documentsButton->setEnabled(true);

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
            return m_documentsButton->setDisabled(true);

        case Page_QmlCodeEditor:
            return m_qmlCodeEditorButton->setDisabled(true);

        case Page_ProjectOptions:
            return m_projectOptionsButton->setDisabled(true);
    }
}

void PageSwitcherPane::reset()
{
    setCurrentPage(Page_Designer);
    setPageDisabled(Page_QmlCodeEditor);
    setPageDisabled(Page_SplitView);
}

void PageSwitcherPane::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient gradient(rect().topRight(), rect().topLeft());
    gradient.setColorAt(0, "#6f7e8c");
    gradient.setColorAt(1, "#475059");

    painter.fillRect(rect(), gradient);
    painter.setPen("#475059");
    painter.drawLine(QRectF(rect()).topRight() + QPointF(-0.5, 0.5), QRectF(rect()).bottomRight() + QPointF(-0.5, -0.5));
}
