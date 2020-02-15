#ifndef BUILDSCONTROLLER_H
#define BUILDSCONTROLLER_H

#include <platform.h>
#include <platformselectionwidget.h>

class BuildsPane;
class DownloadController;
class AndroidPlatformController;

class BuildsController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildsController)

public:
    explicit BuildsController(BuildsPane* buildsPane, QObject* parent = nullptr);

public slots:
    void charge();
    void discharge();

private slots:
    void onNewButtonClick();
    void onBackButtonClick();
    void onNextButtonClick();
    void onBuildButtonClick();

private:
    QWidget* widgetForPlatform(Platform platform) const;

private:
    BuildsPane* m_buildsPane;
    DownloadController* m_downloadController;
    AndroidPlatformController* m_androidPlatformController;
};

#endif // BUILDSCONTROLLER_H
