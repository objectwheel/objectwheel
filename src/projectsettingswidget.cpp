#include <projectsettingswidget.h>
#include <fit.h>

ProjectSettingsWidget::ProjectSettingsWidget(QWidget *parent) : QWidget(parent)
{

}

QSize ProjectSettingsWidget::sizeHint() const
{
    return fit::fx(QSizeF(680, 680)).toSize();
}

void ProjectSettingsWidget::reset()
{
    //TODO
}
