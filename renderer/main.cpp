#include <applicationcore.h>
#include <commandlineparser.h>
#include <QApplication>

int main(int argc, char* argv[])
{
    // Parse commandline arguments
    CommandlineParser::init(argc, argv);

    // Apply boot settings
    ApplicationCore::prepare();

    // Initialize application
    QApplication app(argc, argv);

    // Initialize application core
    ApplicationCore::init(&app);

    // Start main event loop
    return app.exec();
}
