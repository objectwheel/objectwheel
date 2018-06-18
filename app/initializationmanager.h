#ifndef INITIALIZATIONMANAGER_H
#define INITIALIZATIONMANAGER_H

#include <QObject>
#include <QSettings>

class Authenticator;
class UserManager;
class ProjectManager;
class DocumentManager;
class ControlPreviewingManager;
class ControlExposingManager;
class ControlRemovingManager;
class RunManager;
class SaveManager;
class ControlMonitoringManager;
class WindowManager;
class MenuManager;
class ControlTransactionManager;

namespace Core { class HelpManager; }

class InitializationManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(InitializationManager)

public:
    static void init(QObject* parent);
    static InitializationManager* instance();
    static QString resourcePath();
    static QString userResourcePath();
    static QSettings* settings(QSettings::Scope scope = QSettings::UserScope);

private slots:
    void onSessionStop();
    void onProjectStart();

private:
    explicit InitializationManager(QObject* parent = nullptr);
    ~InitializationManager();

private:
    static InitializationManager* s_instance;
    static Authenticator* s_authenticator;
    static UserManager* s_userManager;
    static ControlPreviewingManager* s_controlPreviewingManager;
    static SaveManager* s_saveManager;
    static ProjectManager* s_projectManager;
    static ControlExposingManager* s_controlExposingManager;
    static ControlRemovingManager* s_controlRemovingManager;
    static RunManager* s_runManager;
    static ControlMonitoringManager* s_controlMonitoringManager;
    static ControlTransactionManager* s_controlTransactionManager;
    static Core::HelpManager* s_helpManager;
    static DocumentManager* s_documentManager;
    static WindowManager* s_windowManager;
    static MenuManager* s_menuManager;
};

#endif // INITIALIZATIONMANAGER_H
