#ifndef BACKENDMANAGER_H
#define BACKENDMANAGER_H

#include <QObject>
#include <QSettings>

class Authenticator;
class UserBackend;
class ProjectBackend;
class EditorBackend;
class PreviewerBackend;
class ExposerBackend;
class InterpreterBackend;
class SaveBackend;

namespace Core { class HelpManager; }

class BackendManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BackendManager)

public:
    static void init();
    static BackendManager* instance();
    static QString resourcePath();
    static QString userResourcePath();
    static QSettings* settings(QSettings::Scope scope = QSettings::UserScope);

public slots:
    void onSessionStop();
    void onProjectStart();

private:
    BackendManager();
    ~BackendManager();

private:
    static BackendManager* s_instance;
    static Authenticator* s_authenticator;
    static UserBackend* s_userBackend;
    static PreviewerBackend* s_previewerBackend;
    static SaveBackend* s_saveBackend;
    static ProjectBackend* s_projectBackend;
    static ExposerBackend* s_exposerBackend;
    static InterpreterBackend* s_interpreterBackend;
    static Core::HelpManager* s_helpManager;
    static EditorBackend* s_editorBackend;
};

#endif // BACKENDMANAGER_H
