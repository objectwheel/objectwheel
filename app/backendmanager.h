#ifndef BACKENDMANAGER_H
#define BACKENDMANAGER_H

#include <QObject>

class BackendManager : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(BackendManager)

    public:
        static BackendManager* instance();

    public slots:
        inline void init() const {}

    private slots:
        void handleSessionStop() const;
        void handleProjectStart() const;

    private:
        BackendManager();
};

#endif // BACKENDMANAGER_H
