#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>

class UpdateManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UpdateManager)

    friend class ApplicationCore;

public:
    static UpdateManager* instance();

private:
    explicit UpdateManager(QObject* parent = nullptr);
    ~UpdateManager() override;

private:
    static UpdateManager* s_instance;
};

#endif // UPDATEMANAGER_H
