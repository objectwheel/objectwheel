#include <bootsettings.h>
#include <filemanager.h>
#include <interfacesettings.h>

#include <QApplication>
#include <QLoggingCategory>

BootSettings* BootSettings::s_instance = nullptr;
BootSettings::BootSettings(const char* filePath)
    : m_settings(dname(filePath) + "/settings.ini", QSettings::IniFormat)
{
    s_instance = this;
    m_generalSettings.read();

    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    if (m_generalSettings.interfaceSettings()->hdpiEnabled)
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    /* Disable Qml Parser warnings */
    QLoggingCategory::setFilterRules(QStringLiteral("qtc*.info=false\n"
                                                    "qtc*.debug=false\n"
                                                    "qtc*.warning=false\n"
                                                    "qtc*.critical=false\n"
                                                    "qtc*=false"));
}

BootSettings::~BootSettings()
{
    s_instance = nullptr;
}

void BootSettings::init(const char* filePath)
{
    static BootSettings instance(filePath);
    Q_UNUSED(instance);
}

QSettings* BootSettings::settings()
{
    return &s_instance->m_settings;
}

const char* BootSettings::resourcePath()
{
    return ":";
}

const char* BootSettings::userResourcePath()
{
    return ":";
}