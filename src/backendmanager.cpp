#include <backendmanager.h>
#include <toolsbackend.h>
#include <userbackend.h>
#include <projectbackend.h>
#include <savebackend.h>
#include <previewbackend.h>

BackendManager::BackendManager()
{
//    _toolsBackend = new ToolsBackend;
    _userBackend = new UserBackend;
    _projectBackend = new ProjectBackend;
    _saveBackend = new SaveBackend;
    _previewBackend = new PreviewBackend;
}

BackendManager* BackendManager::instance()
{
    static BackendManager instance;
    return &instance;
}
