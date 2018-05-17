#ifndef BACKENDMANAGER_H
#define BACKENDMANAGER_H

#include <QObject>
#include <QSettings>

#include <theme/theme_p.h>
#include <coreplugin/themechooser.h>
#include <coreplugin/helpmanager.h>

class EditorBackend;

namespace TextEditor { class TextEditorSettings; }
namespace Core { class HelpManager; }

class BackendManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BackendManager)

    BackendManager();
    ~BackendManager();

public:
    static BackendManager* instance();

public slots:
    static void init();
    static QString resourcePath();
    static QString userResourcePath();
    static QSettings* settings(QSettings::Scope scope = QSettings::UserScope);

private slots:
    void handleSessionStop() const;
    void handleProjectStart() const;

private:
    Core::HelpManager m_helpManager;
    static EditorBackend* m_editorBackend;
};

#endif // BACKENDMANAGER_H
