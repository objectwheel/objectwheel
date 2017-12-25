#ifndef BACKENDMANAGER_H
#define BACKENDMANAGER_H

#include <QObject>

class UserBackend;
class SaveBackend;
class ToolsBackend;
class ProjectBackend;
class PreviewBackend;

class BackendManager : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(BackendManager)

    public:
        static BackendManager* instance();

    private:
        BackendManager();

    private:
        ToolsBackend* _toolsBackend;
        UserBackend* _userBackend;
        ProjectBackend* _projectBackend;
        SaveBackend* _saveBackend;
        PreviewBackend* _previewBackend;

};

#endif // BACKENDMANAGER_H