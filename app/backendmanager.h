#ifndef BACKENDMANAGER_H
#define BACKENDMANAGER_H

#include <QObject>
#include <QSettings>

class Authenticator;
class ProjectBackend;
class EditorBackend;

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
    static ProjectBackend* s_projectBackend;
    static Core::HelpManager* s_helpManager;
    static EditorBackend* s_editorBackend;
};

#endif // BACKENDMANAGER_H
