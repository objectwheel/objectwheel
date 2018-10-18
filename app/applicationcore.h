#ifndef APPLICATIONCORE_H
#define APPLICATIONCORE_H

#include <QObject>

class Authenticator;
class UserManager;
class DocumentManager;
class ProjectManager;
class ProjectExposingManager;
class ControlPreviewingManager;
class ControlCreationManager;
class ControlRemovingManager;
class ControlPropertyManager;
class RunManager;
class SaveManager;
class WindowManager;
class MenuManager;

namespace Core { class HelpManager; }

class ApplicationCore final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ApplicationCore)

public:
    static void init(QObject* parent);
    static ApplicationCore* instance();
    static QString resourcePath();
    static QString userResourcePath();
    static void setApplicationPalette();

private slots:
    void onUserSessionStart();
    void onUserSessionStop();
    void onProjectStart();
    void onProjectStop();

private:
    explicit ApplicationCore(QObject* parent = nullptr);

private:
    static ApplicationCore* s_instance;
    static Authenticator* s_authenticator;
    static UserManager* s_userManager;
    static ControlPreviewingManager* s_controlPreviewingManager;
    static SaveManager* s_saveManager;
    static ProjectManager* s_projectManager;
    static ProjectExposingManager* s_projectExposingManager;
    static ControlCreationManager* s_controlExposingManager;
    static ControlRemovingManager* s_controlRemovingManager;
    static ControlPropertyManager* s_controlPropertyManager;
    static RunManager* s_runManager;
    static Core::HelpManager* s_helpManager;
    static DocumentManager* s_documentManager;
    static WindowManager* s_windowManager;
    static MenuManager* s_menuManager;
};

#endif // APPLICATIONCORE_H
