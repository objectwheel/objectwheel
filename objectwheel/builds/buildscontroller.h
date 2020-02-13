#ifndef BUILDSCONTROLLER_H
#define BUILDSCONTROLLER_H

#include <QObject>

class BuildsPane;
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
    void onNextButtonClick();
    void onBackButtonClick();

private:
    BuildsPane* m_buildsPane;
    AndroidPlatformController* m_androidPlatformController;
};

#endif // BUILDSCONTROLLER_H
