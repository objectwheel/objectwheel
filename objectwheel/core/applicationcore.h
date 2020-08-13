#ifndef APPLICATIONCORE_H
#define APPLICATIONCORE_H

#include <QPalette>

class QSettings;
class GeneralSettings;
class DesignerSettings;
class CodeEditorSettings;
class SystemSettings;
class ModeManager;
class ApiManager;
class UserManager;
class DocumentManager;
class ProjectManager;
class ProjectExposingManager;
class ControlRenderingManager;
class ControlProductionManager;
class ControlRemovingManager;
class ControlPropertyManager;
class RunManager;
class SaveManager;
class WindowManager;
class MenuManager;
class HelpManager;
class ServerManager;
class PayloadManager;
class UpdateManager;
class InactivityWatcher;

class ApplicationCore final
{
public:
    ApplicationCore();
   ~ApplicationCore();
    ApplicationCore(const ApplicationCore&) = delete;
    ApplicationCore &operator=(const ApplicationCore&) = delete;

    static bool locked();
    static void prepare();

    static QPalette palette();
    static QSettings* settings();
    static QString modulesPath();
    static QString settingsPath();
    static QString resourcePath();
    static QString documentsPath();
    static QString updatesPath();
    static QString stylesPath();
    static QString resourceStylesPath();
    static QString appDataPath();

private:
    static void onActivated();
    static void onDeactivated();
    static void onProjectStop();
    static void onProjectStart();
    static void onUserSessionStop();
    static void onUserSessionStart();

private:
    static QSettings* s_settings;
    static GeneralSettings* s_generalSettings;
    static DesignerSettings* s_designerSettings;
    static CodeEditorSettings* s_codeEditorSettings;
    static SystemSettings* s_systemSettings;
    static ModeManager* s_modeManager;
    static ServerManager* s_serverManager;
    static PayloadManager* s_payloadManager;
    static UpdateManager* s_updateManager;
    static ApiManager* s_apiManager;
    static UserManager* s_userManager;
    static ControlRenderingManager* s_controlRenderingManager;
    static SaveManager* s_saveManager;
    static ProjectManager* s_projectManager;
    static ProjectExposingManager* s_projectExposingManager;
    static ControlProductionManager* s_controlProductionManager;
    static ControlRemovingManager* s_controlRemovingManager;
    static ControlPropertyManager* s_controlPropertyManager;
    static RunManager* s_runManager;
    static HelpManager* s_helpManager;
    static DocumentManager* s_documentManager;
    static WindowManager* s_windowManager;
    static MenuManager* s_menuManager;
    static InactivityWatcher* s_inactivityWatcher;
};

#endif // APPLICATIONCORE_H
