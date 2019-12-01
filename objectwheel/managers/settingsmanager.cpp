#include <settingsmanager.h>
#include <settings.h>

#include <QTimerEvent>
#include <QVariantMap>

SettingsManager* SettingsManager::s_instance = nullptr;
QBasicTimer SettingsManager::s_autoSaveTimer;
QMap<Settings*, SettingsManager::VariantMap> SettingsManager::s_values;
QMap<Settings*, SettingsManager::FunctionMap> SettingsManager::s_functions;

SettingsManager::SettingsManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

SettingsManager::~SettingsManager()
{
    s_instance = nullptr;
}

SettingsManager* SettingsManager::instance()
{
    return s_instance;
}

void SettingsManager::start(int msec)
{
    s_autoSaveTimer.start(msec, Qt::VeryCoarseTimer, s_instance);
}

void SettingsManager::stop()
{
    s_autoSaveTimer.stop();
}

void SettingsManager::save()
{
    foreach (Settings* settings, s_functions.keys()) {
        settings->begin();
        foreach (const char* key, s_functions[settings].keys())
            settings->setValue(key, s_functions[settings][key]());
        settings->end();
    }
    // Values should be processed later, since we
    // might want to override a value manually.
    foreach (Settings* settings, s_values.keys()) {
        settings->begin();
        foreach (const char* key, s_values[settings].keys())
            settings->setValue(key, s_values[settings][key]);
        settings->end();
    }
    s_values.clear();
    // s_functions.clear(); Functions don't go away
}

void SettingsManager::insertValue(Settings* settings, const char* setting, const QVariant& value)
{
    s_values[settings].insert(setting, value);
}

void SettingsManager::insertFunction(Settings* settings, const char* setting, const std::function<QVariant()>& function)
{
    s_functions[settings].insert(setting, function);
}

void SettingsManager::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == s_autoSaveTimer.timerId())
        save();
    else
        QObject::timerEvent(event);
}
