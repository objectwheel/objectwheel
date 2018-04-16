#include <scalingwidget.h>
#include <saveutils.h>
#include <projectbackend.h>

#include <QVBoxLayout>
#include <QRadioButton>

ScalingWidget::ScalingWidget(QWidget *parent) : QGroupBox(parent)
    , m_layout(new QVBoxLayout(this))
    , m_smartScalingButton(new QRadioButton)
    , m_highDpiScalingButton(new QRadioButton)
    , m_noScalingButton(new QRadioButton)
{
    setTitle(tr("Scaling"));
    m_smartScalingButton->setText(tr("Smart Scaling"));
    m_highDpiScalingButton->setText(tr("High Dpi Scaling"));
    m_noScalingButton->setText(tr("No Scaling"));

    m_layout->addWidget(m_smartScalingButton);
    m_layout->addWidget(m_highDpiScalingButton);
    m_layout->addWidget(m_noScalingButton);

    connect(m_smartScalingButton, SIGNAL(clicked(bool)), SLOT(saveTheme()));
    connect(m_highDpiScalingButton, SIGNAL(clicked(bool)), SLOT(saveTheme()));
    connect(m_noScalingButton, SIGNAL(clicked(bool)), SLOT(saveTheme()));
}

void ScalingWidget::reset()
{
    auto scaling = SaveUtils::scaling(ProjectBackend::instance()->dir());

    if (scaling == "noScaling")
        m_noScalingButton->setChecked(true);
    else if (scaling == "highDpiScaling")
        m_highDpiScalingButton->setChecked(true);
    else
        m_smartScalingButton->setChecked(true);
}

void ScalingWidget::saveTheme()
{
    QString scaling;

    if (m_smartScalingButton->isChecked())
        scaling = "smartScaling";
    else if (m_highDpiScalingButton->isChecked())
        scaling = "highDpiScaling";
    else
        scaling = "noScaling";

    SaveUtils::setProjectProperty(ProjectBackend::instance()->dir(), PTAG_SCALING, scaling);
}
