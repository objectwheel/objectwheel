#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QBasicTimer>

class QTimerEvent;
struct Settings;

class SettingsManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SettingsManager)

    friend class ApplicationCore;

    using VariantMap = QMap<const char*, QVariant>;
    using FunctionMap = QMap<const char*, std::function<QVariant()>>;

public:
    static SettingsManager* instance();

public slots:
    static void start(int msec);
    static void stop();
    static void save();
    static void insertValue(Settings* settings, const char* setting, const QVariant& value);
    static void insertFunction(Settings* settings, const char* setting, const std::function<QVariant()>& function);

private:
    void timerEvent(QTimerEvent* event) override;

private:
    explicit SettingsManager(QObject* parent = nullptr);
    ~SettingsManager() override;

private:
    static SettingsManager* s_instance;
    static QBasicTimer s_autoSaveTimer;
    static QMap<Settings*, VariantMap> s_values;
    static QMap<Settings*, FunctionMap> s_functions;
};

#endif // SETTINGSMANAGER_H
