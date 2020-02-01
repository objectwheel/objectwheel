#ifndef BUILDSCONTROLLER_H
#define BUILDSCONTROLLER_H

#include <QObject>

class BuildsPane;
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

private:
    BuildsPane* m_buildsPane;
};

#endif // BUILDSCONTROLLER_H
