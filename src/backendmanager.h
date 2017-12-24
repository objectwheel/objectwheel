#ifndef BACKENDMANAGER_H
#define BACKENDMANAGER_H

#include <QObject>

class BackendManager : public QObject
{
        Q_OBJECT
    public:
        explicit BackendManager(QObject *parent = nullptr);

    signals:

    public slots:
};

#endif // BACKENDMANAGER_H