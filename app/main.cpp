#include <applicationcore.h>
#include <bootsettings.h>

#include <QApplication>

int main(int argc, char* argv[])
{
    // Apply boot Settings
    BootSettings::apply();

    // Initialize application
    QApplication app(argc, argv);

    // Initialize application core
    ApplicationCore::init(&app);

    // Start main event loop
    return app.exec();
}
