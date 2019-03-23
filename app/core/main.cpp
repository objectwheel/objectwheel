#include <applicationcore.h>
#include <QApplication>

int main(int argc, char* argv[])
{
    // Prepare application core
    ApplicationCore::prepare(argv[0]);

    // Initialize application
    QApplication app(argc, argv);

    if (ApplicationCore::locked())
        return EXIT_FAILURE;

    // Initialize application core
    ApplicationCore::run(&app);

    // Start main event loop
    return app.exec();
}
