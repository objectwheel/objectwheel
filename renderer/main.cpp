#include <applicationcore.h>
#include <commandlineparser.h>
#include <cleanexit.h>
#include <QApplication>

int main(int argc, char* argv[])
{
    // Parse commandline arguments
    CommandlineParser::init(argc, argv);

    // Prepare core
    ApplicationCore::prepare();

    // Initialize application
    QApplication app(argc, argv);

    // Run core
    ApplicationCore core;
    core.run();

    // Start main event loop
    return CleanExit::exec(app);
}