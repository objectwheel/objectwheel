#include <applicationcore.h>
#include <cleanexit.h>
#include <QApplication>

int main(int argc, char* argv[])
{
    // Prepare application core
    ApplicationCore::prepare();

    // Initialize application
    QApplication app(argc, argv);

    if (ApplicationCore::locked())
        return EXIT_FAILURE;

    // Initialize application core
    ApplicationCore::run(&app);

    // Start main event loop
    return CleanExit::exec(app);
}
