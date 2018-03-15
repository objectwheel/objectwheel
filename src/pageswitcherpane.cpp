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
    m_documentsButton->setText(tr("Documents"));
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

    m_designerButton->setFixedHeight(fit::fx(65));
    m_qmlCodeEditorButton->setFixedHeight(fit::fx(65));
    m_projectOptionsButton->setFixedHeight(fit::fx(65));
    m_documentsButton->setFixedHeight(fit::fx(65));
    m_splitViewButton->setFixedHeight(fit::fx(65));
    m_buildsButton->setFixedHeight(fit::fx(65));

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

    connect(m_designerButton, &FlatButton::pressed, [=] { m_designerButton->setChecked(true); });
    connect(m_qmlCodeEditorButton, &FlatButton::pressed, [=] { m_qmlCodeEditorButton->setChecked(true); });
    connect(m_projectOptionsButton, &FlatButton::pressed, [=] { m_projectOptionsButton->setChecked(true); });
    connect(m_documentsButton, &FlatButton::pressed, [=] { m_documentsButton->setChecked(true); });
    connect(m_splitViewButton, &FlatButton::pressed, [=] { m_splitViewButton->setChecked(true); });
    connect(m_buildsButton, &FlatButton::pressed, [=] { m_buildsButton->setChecked(true); });

    connect(m_designerButton, SIGNAL(pressed()), SIGNAL(designerActivated()));
    connect(m_qmlCodeEditorButton, SIGNAL(pressed()), SIGNAL(qmlCodeEditorActivated()));
    connect(m_projectOptionsButton, SIGNAL(pressed()), SIGNAL(projectOptionsActivated()));
    connect(m_documentsButton, SIGNAL(pressed()), SIGNAL(documentsActivated()));
    connect(m_splitViewButton, SIGNAL(pressed()), SIGNAL(splitViewActivated()));
    connect(m_buildsButton, SIGNAL(pressed()), SIGNAL(buildsActivated()));
}

void PageSwitcherPane::setPage(const PageSwitcherPane::Pages& page)
{
    switch (page) {
        case Builds:
            return m_buildsButton->click();
        case Designer:
            return m_designerButton->click();
        case SplitView:
            return m_splitViewButton->click();
        case Documents:
            return m_documentsButton->click();
        case QmlCodeEditor:
            return m_qmlCodeEditorButton->click();
        case ProjectOptions:
            return m_projectOptionsButton->click();
    }
}

PageSwitcherPane::Pages PageSwitcherPane::page() const
{
    if (m_buildsButton->isVisible())
        return Builds;
    if (m_designerButton->isVisible())
        return Designer;
    if (m_splitViewButton->isVisible())
        return SplitView;
    if (m_documentsButton->isVisible())
        return Documents;
    if (m_qmlCodeEditorButton->isVisible())
        return QmlCodeEditor;
    else
        return ProjectOptions;
}

void PageSwitcherPane::reset()
{
    m_designerButton->click();
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
