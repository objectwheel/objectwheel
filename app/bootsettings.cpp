#include <bootsettings.h>

#include <QApplication>
#include <QLoggingCategory>

void BootSettings::apply()
{
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL); // For reCaptcha
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    /* Disable Qml Parser warnings */
    QLoggingCategory::setFilterRules(
                QStringLiteral("qtc*.info=false\n"
                               "qtc*.debug=false\n"
                               "qtc*.warning=false\n"
                               "qtc*.critical=false\n"
                               "qtc*=false"));
}
