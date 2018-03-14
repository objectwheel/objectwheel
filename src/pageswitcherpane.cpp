#include <pageswitcherpane.h>
#include <flatbutton.h>
#include <fit.h>

#include <QPainter>
#include <QVBoxLayout>

PageSwitcherPane::PageSwitcherPane(QWidget *parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_qmlCodeEditorButton(new FlatButton)
  , m_designerButton(new FlatButton)
  , m_projectSettingsButton(new FlatButton)
  , m_buildsButton(new FlatButton)
  , m_documentsButton(new FlatButton)
  , m_splitViewButton(new FlatButton)
{
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_designerButton);
    m_layout->addWidget(m_qmlCodeEditorButton);
    m_layout->addWidget(m_projectSettingsButton);
    m_layout->addWidget(m_documentsButton);
    m_layout->addWidget(m_splitViewButton);
    m_layout->addStretch();
    m_layout->addWidget(m_buildsButton);

    QFont labelFont;
    labelFont.setWeight(QFont::Bold);
    labelFont.setPixelSize(labelFont.pixelSize() - fit::fx(2.5));

    m_designerButton->setText(tr("Designer"));
    m_qmlCodeEditorButton->setText(tr("Editor"));
    m_projectSettingsButton->setText(tr("Project"));
    m_documentsButton->setText(tr("Documents"));
    m_splitViewButton->setText(tr("Split View"));
    m_buildsButton->setText(tr("Cloud Builds"));

    m_buildsButton->setFont(labelFont);
    m_splitViewButton->setFont(labelFont);
    m_documentsButton->setFont(labelFont);
    m_projectSettingsButton->setFont(labelFont);
    m_qmlCodeEditorButton->setFont(labelFont);
    m_designerButton->setFont(labelFont);

    m_designerButton->setAutoExclusive(true);
    m_qmlCodeEditorButton->setAutoExclusive(true);
    m_projectSettingsButton->setAutoExclusive(true);
    m_documentsButton->setAutoExclusive(true);
    m_splitViewButton->setAutoExclusive(true);
    m_buildsButton->setAutoExclusive(true);

    m_designerButton->setFixedHeight(fit::fx(65));
    m_qmlCodeEditorButton->setFixedHeight(fit::fx(65));
    m_projectSettingsButton->setFixedHeight(fit::fx(65));
    m_documentsButton->setFixedHeight(fit::fx(65));
    m_splitViewButton->setFixedHeight(fit::fx(65));
    m_buildsButton->setFixedHeight(fit::fx(65));

    m_designerButton->setCheckable(true);
    m_qmlCodeEditorButton->setCheckable(true);
    m_projectSettingsButton->setCheckable(true);
    m_documentsButton->setCheckable(true);
    m_splitViewButton->setCheckable(true);
    m_buildsButton->setCheckable(true);

    m_designerButton->setIconSize(fit::fx(QSizeF(30, 30)).toSize());
    m_qmlCodeEditorButton->setIconSize(fit::fx(QSizeF(30, 30)).toSize());
    m_projectSettingsButton->setIconSize(fit::fx(QSizeF(30, 30)).toSize());
    m_documentsButton->setIconSize(fit::fx(QSizeF(30, 30)).toSize());
    m_splitViewButton->setIconSize(fit::fx(QSizeF(30, 30)).toSize());
    m_buildsButton->setIconSize(fit::fx(QSizeF(30, 30)).toSize());

    connect(m_designerButton, &FlatButton::pressed, [=] { m_designerButton->setChecked(true); });
    connect(m_qmlCodeEditorButton, &FlatButton::pressed, [=] { m_qmlCodeEditorButton->setChecked(true); });
    connect(m_projectSettingsButton, &FlatButton::pressed, [=] { m_projectSettingsButton->setChecked(true); });
    connect(m_documentsButton, &FlatButton::pressed, [=] { m_documentsButton->setChecked(true); });
    connect(m_splitViewButton, &FlatButton::pressed, [=] { m_splitViewButton->setChecked(true); });
    connect(m_buildsButton, &FlatButton::pressed, [=] { m_buildsButton->setChecked(true); });

    m_designerButton->setIcon(QIcon("/users/omergoktas/desktop/designer.png"));
    m_qmlCodeEditorButton->setIcon(QIcon("/users/omergoktas/desktop/editor.png"));
    m_projectSettingsButton->setIcon(QIcon("/users/omergoktas/desktop/settings.png"));
    m_documentsButton->setIcon(QIcon("/users/omergoktas/desktop/help.png"));
    m_splitViewButton->setIcon(QIcon("/users/omergoktas/desktop/split.png"));
    m_buildsButton->setIcon(QIcon("/users/omergoktas/desktop/helmet.png"));

    m_buildsButton->settings().textColor = "#f0f4f7";
    m_splitViewButton->settings().textColor = "#f0f4f7";
    m_documentsButton->settings().textColor = "#f0f4f7";
    m_projectSettingsButton->settings().textColor = "#f0f4f7";
    m_qmlCodeEditorButton->settings().textColor = "#f0f4f7";
    m_designerButton->settings().textColor = "#f0f4f7";

    m_designerButton->settings().showShadow = false;
    m_qmlCodeEditorButton->settings().showShadow = false;
    m_projectSettingsButton->settings().showShadow = false;
    m_documentsButton->settings().showShadow = false;
    m_splitViewButton->settings().showShadow = false;
    m_buildsButton->settings().showShadow = false;

    m_designerButton->settings().textUnderIcon = true;
    m_qmlCodeEditorButton->settings().textUnderIcon = true;
    m_projectSettingsButton->settings().textUnderIcon = true;
    m_documentsButton->settings().textUnderIcon = true;
    m_splitViewButton->settings().textUnderIcon = true;
    m_buildsButton->settings().textUnderIcon = true;

    m_designerButton->settings().topColor = "#465059";
    m_qmlCodeEditorButton->settings().topColor = "#465059";
    m_projectSettingsButton->settings().topColor = "#465059";
    m_documentsButton->settings().topColor = "#465059";
    m_splitViewButton->settings().topColor = "#465059";
    m_buildsButton->settings().topColor = "#465059";

    m_designerButton->settings().bottomColor = "#465059";
    m_qmlCodeEditorButton->settings().bottomColor = "#465059";
    m_projectSettingsButton->settings().bottomColor = "#465059";
    m_documentsButton->settings().bottomColor = "#465059";
    m_splitViewButton->settings().bottomColor = "#465059";
    m_buildsButton->settings().bottomColor = "#465059";
}

void PageSwitcherPane::reset()
{
    m_designerButton->click();
}

void PageSwitcherPane::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), "#465059");
    painter.setPen("#1e2326");
    painter.drawLine(QRectF(rect()).topRight() + QPointF(-0.5, 0.5), QRectF(rect()).bottomRight() + QPointF(-0.5, -0.5));
}
