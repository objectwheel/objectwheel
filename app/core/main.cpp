#include <applicationcore.h>
#include <QtWidgets>
#include <hashfactory.h>
// 32
int main(int argc, char* argv[])
{
    QString e = "/Users/omergoktas/Desktop/İş/serverstuff/build/objectwheel-interpreter/components/";
    QElapsedTimer t; t.start();

    for (int i = 0; i < 10000; ++i)
        QDir(e).dirName();

    qDebug() <<QDir(e).dirName();
    return 0;

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
