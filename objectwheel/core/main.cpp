#include <applicationcore.h>

int main(int argc, char* argv[])
{
    // Prepare application core
    ApplicationCore::prepare();

    // Initialize application
    QApplication app(argc, argv);

    if (ApplicationCore::locked())
        return EXIT_FAILURE;

    // Initialize application core
    ApplicationCore core;
    Q_UNUSED(core);

    // Start main event loop
    return app.exec();
}
