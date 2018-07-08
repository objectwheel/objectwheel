#include <scalingwidget.h>
#include <saveutils.h>
#include <projectmanager.h>

#include <QVBoxLayout>
#include <QRadioButton>

ScalingWidget::ScalingWidget(QWidget *parent) : QGroupBox(parent)
    , m_layout(new QVBoxLayout(this))
    , m_highDpiScalingButton(new QRadioButton)
    , m_noScalingButton(new QRadioButton)
{
    setTitle(tr("Scaling"));
    m_highDpiScalingButton->setText(tr("High Dpi Scaling"));
    m_noScalingButton->setText(tr("No Scaling"));

    m_layout->addWidget(m_highDpiScalingButton);
    m_layout->addWidget(m_noScalingButton);

    connect(m_highDpiScalingButton, SIGNAL(clicked(bool)), SLOT(saveTheme()));
    connect(m_noScalingButton, SIGNAL(clicked(bool)), SLOT(saveTheme()));
}

void ScalingWidget::sweep()
{
    auto scaling = SaveUtils::projectScaling(ProjectManager::dir());

    if (scaling == "noScaling")
        m_noScalingButton->setChecked(true);
    else
        m_highDpiScalingButton->setChecked(true);
}

void ScalingWidget::saveTheme()
{
    QString scaling;

    if (m_highDpiScalingButton->isChecked())
        scaling = "highDpiScaling";
    else
        scaling = "noScaling";

    SaveUtils::setProjectProperty(ProjectManager::dir(), PTAG_SCALING, scaling);
}
