#include <applicationcore.h>
#include <QApplication>

#include <QResource>
int main(int argc, char* argv[])
{
    // Prepare application core
    ApplicationCore::prepare(argv[0]);

    // Initialize application
    QApplication app(argc, argv);

    QResource::registerResource(
                "/Users/omergoktas/Projeler/Git/objectwheel/utils/resources/internal/internal.rcc");

    QResource::registerResource(
                "/Users/omergoktas/Projeler/Git/objectwheel/utils/resources/internal/internal.rcc");

    if (ApplicationCore::locked())
        return EXIT_FAILURE;

    // Initialize application core
    ApplicationCore::run(&app);

    // Start main event loop
    return app.exec();
}
