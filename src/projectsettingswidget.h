#ifndef PROJECTSETTINGSWIDGET_H
#define PROJECTSETTINGSWIDGET_H

#include <QWidget>

class ProjectSettingsWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit ProjectSettingsWidget(QWidget *parent = nullptr);

    public slots:
        void reset();
};

#endif // PROJECTSETTINGSWIDGET_H