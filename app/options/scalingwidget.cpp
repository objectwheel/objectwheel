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

    connect(m_highDpiScalingButton, &QRadioButton::clicked, this, &ScalingWidget::saveTheme);
    connect(m_noScalingButton, &QRadioButton::clicked, this, &ScalingWidget::saveTheme);
}

void ScalingWidget::discharge()
{
    if (ProjectManager::uid().isEmpty())
        return;

    if (SaveUtils::projectHdpiScaling(ProjectManager::dir()))
        m_highDpiScalingButton->setChecked(true);
    else
        m_noScalingButton->setChecked(true);
}

void ScalingWidget::saveTheme()
{
    SaveUtils::setProperty(ProjectManager::dir(), SaveUtils::ProjectHdpiScaling, m_highDpiScalingButton->isChecked());
}
