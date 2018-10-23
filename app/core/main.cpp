#include <applicationcore.h>
#include <bootsettings.h>

#include <QApplication>
#include <QMessageBox>
#include <QSharedMemory>

int main(int argc, char* argv[])
{
    // Apply boot Settings
    BootSettings::init(argv[0]);

    // Initialize application
    QApplication app(argc, argv);

    // Multiple instances protection
    QSharedMemory sharedMemory("T2JqZWN0d2hlZWxTaGFyZWRNZW1vcnlLZXk");
    if(!sharedMemory.create(1)) {
        sharedMemory.attach();
        sharedMemory.detach();
        if(!sharedMemory.create(1)) {
            QMessageBox::warning(nullptr,
                                 QObject::tr("Quitting"),
                                 QObject::tr("Another instance is already running."));
            return EXIT_FAILURE;
        }
    }

    // Initialize application core
    ApplicationCore::init(&app);

    // Start main event loop
    return app.exec();
}
