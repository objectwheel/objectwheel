#include <QApplication>
#include <QIcon>
#include <QFileInfo>
#include <QCborMap>
#include <QCborValue>
#include <QProgressDialog>
#include <QTimer>
#include <QDir>

#include <csignal>

#ifdef Q_OS_MACOS
#  include <macoperations.h>
#endif

static QDir g_filesDir;
static QDir g_installationDir;
static QString g_diffFilePath;
static QCborMap g_differences;
static QProgressDialog* g_progressDialog;

static QString detectInstallationDir();

static void unload();
static void readDiffFile();
static void removeOldFiles();

int main(int argc, char *argv[])
{
    std::signal(SIGFPE, SIG_IGN);
    std::signal(SIGILL, SIG_IGN);
    std::signal(SIGSEGV, SIG_IGN);
    std::signal(SIGABRT, SIG_IGN);
    std::signal(SIGINT, SIG_IGN);
    std::signal(SIGTERM, SIG_IGN);

    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Initialize application
    QApplication a(argc, argv);
    QApplication::setApplicationName(QStringLiteral(APP_NAME));
    QApplication::setOrganizationName(QStringLiteral(APP_CORP));
    QApplication::setApplicationVersion(QStringLiteral(APP_VER));
    QApplication::setOrganizationDomain(QStringLiteral(APP_DOMAIN));
    QApplication::setApplicationDisplayName(QStringLiteral(APP_NAME) + QObject::tr(" Updater"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/images/icon.png")));

    if (argc != 2) {
        qWarning("There are missing arguments. Exiting...");
        return EXIT_FAILURE;
    }

    if (!QFileInfo::exists(argv[1])) {
        qWarning("The diff file is missing. Exiting...");
        return EXIT_FAILURE;
    }

    g_diffFilePath = argv[1];
    g_filesDir = QFileInfo(g_diffFilePath).dir().absoluteFilePath(QLatin1String("Files"));
    g_installationDir = detectInstallationDir();

    if (!g_filesDir.exists()) {
        qWarning("No Files dir. Exiting...");
        return EXIT_FAILURE;
    }

    if (!g_installationDir.exists()) {
        qWarning("Cannot detect installation dir properly. Exiting...");
        return EXIT_FAILURE;
    }

    g_progressDialog = new QProgressDialog(
                QObject::tr("Updating, please do not interrupt the process..."),
                QString(), 0, g_differences.size(), nullptr,
                Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    g_progressDialog->setWindowTitle(QObject::tr("Objectwheel Updater"));
    g_progressDialog->show();

    QTimer::singleShot(2000, [] {
        readDiffFile();
        removeOldFiles();
    });

    return a.exec();
}

static QString detectInstallationDir()
{
    // TODO: Handle other OSes
#if defined(Q_OS_MACOS)
    return QFileInfo(QCoreApplication::applicationDirPath() + QLatin1String("/../..")).canonicalFilePath();
#elif defined(Q_OS_WINDOWS)
    return QCoreApplication::applicationDirPath();
#elif defined(Q_OS_LINUX)
    return QCoreApplication::applicationDirPath();
#endif
    return QString();
}

static void unload()
{
    delete g_progressDialog;
    QCoreApplication::exit(EXIT_FAILURE);
}

static void readDiffFile()
{
    QFile file(g_diffFilePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("Cannot open the diff file. Exiting...");
        return unload();
    }

    g_differences = QCborValue::fromCbor(file.readAll()).toMap();

    if (g_differences.isEmpty()) {
        qWarning("The diff file is empty. Exiting...");
        return unload();
    }
}

static void removeOldFiles()
{
    g_progressDialog->setLabelText(QObject::tr("Removing old and redunant files..."));

    foreach (const QCborValue& key, g_differences.keys()) {
        const QString& relativePath = QDir::cleanPath(key.toString());
        const bool remove = g_differences.value(key).toBool(false);

        if (!remove)
            continue;

        if (relativePath.isEmpty()) {
            qWarning("Empty relative path to remove. Skipping...");
            continue;
        }

        const QString& canonicalFilePath = QFileInfo(g_installationDir.filePath(relativePath)).canonicalFilePath();

        if (canonicalFilePath.isEmpty()) {
            qWarning("Empty file path to remove. Skipping...");
            continue;
        }

        if (!QFile::remove(canonicalFilePath))
            qWarning("Unable to remove an old file. Skipping...");

        g_progressDialog->setValue(g_progressDialog->value() + 1);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
}
