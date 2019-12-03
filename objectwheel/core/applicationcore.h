#ifndef APPLICATIONCORE_H
#define APPLICATIONCORE_H

#include <QPalette>

class QSettings;
class GeneralSettings;
class DesignerSettings;
class CodeEditorSettings;
class ModeManager;
class RegistrationApiManager;
class UserManager;
class DocumentManager;
class ProjectManager;
class ProjectExposingManager;
class ControlRenderingManager;
class ControlCreationManager;
class ControlRemovingManager;
class ControlPropertyManager;
class RunManager;
class SaveManager;
class WindowManager;
class MenuManager;
class HelpManager;
class ServerManager;

class ApplicationCore final
{
public:
    static bool locked();
    static void prepare();
    static void run(QApplication* app);

    static QPalette palette();
    static QSettings* settings();
    static QString modulesPath();
    static QString settingsPath();
    static QString resourcePath();
    static QString documentsPath();
    static QString appDataLocation();

private:
    static void onProjectStop();
    static void onProjectStart();
    static void onUserSessionStop();
    static void onUserSessionStart();

private:
    ApplicationCore(QApplication* app);
    ApplicationCore(const ApplicationCore&) = delete;
    ApplicationCore &operator=(const ApplicationCore&) = delete;

private:
    static QSettings* s_settings;
    static GeneralSettings* s_generalSettings;
    static DesignerSettings* s_designerSettings;
    static CodeEditorSettings* s_codeEditorSettings;
    static ModeManager* s_modeManager;
    static ServerManager* s_serverManager;
    static RegistrationApiManager* s_accountManager;
    static UserManager* s_userManager;
    static ControlRenderingManager* s_controlRenderingManager;
    static SaveManager* s_saveManager;
    static ProjectManager* s_projectManager;
    static ProjectExposingManager* s_projectExposingManager;
    static ControlCreationManager* s_controlCreationManager;
    static ControlRemovingManager* s_controlRemovingManager;
    static ControlPropertyManager* s_controlPropertyManager;
    static RunManager* s_runManager;
    static HelpManager* s_helpManager;
    static DocumentManager* s_documentManager;
    static WindowManager* s_windowManager;
    static MenuManager* s_menuManager;
};

#endif // APPLICATIONCORE_H
