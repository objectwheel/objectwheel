#include <QApplication>
#include <QIcon>
#include <QCborMap>
#include <QCborValue>
#include <QProgressDialog>
#include <QTimer>
#include <QDir>
#include <QProcess>
#include <csignal>
#include <appconstants.h>

static bool g_errorFlag = false;
static QFileInfo g_myInfo;
static QDir g_updateCacheDir;
static QDir g_installationDir;
static QString g_diffFilePath;
static QCborMap g_differences;
static QProgressDialog* g_progressDialog;

static QString detectInstallationDir();

static void load(int argc, char* argv[]);
static void unload(int exitCode);
static void readDiffFile();
static void updateFiles();
static void removeOldFiles();
static void launchAndExit();

int main(int argc, char* argv[])
{
    load(argc, argv);

    // Initialize application
    QApplication a(argc, argv);
    QApplication::setApplicationName(AppConstants::NAME);
    QApplication::setOrganizationName(AppConstants::COMPANY);
    QApplication::setApplicationVersion(AppConstants::VERSION);
    QApplication::setOrganizationDomain(AppConstants::ROOT_DOMAIN);
    QApplication::setApplicationDisplayName(AppConstants::LABEL);
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/images/icon.png")));

    if (argc != 3) {
        qWarning("There are missing arguments. Exiting...");
        return EXIT_FAILURE;
    }

    if (!QFileInfo::exists(argv[2])) {
        qWarning("The diff file is missing. Exiting...");
        return EXIT_FAILURE;
    }

    g_diffFilePath = argv[2];
    g_myInfo = QCoreApplication::applicationFilePath();
    g_updateCacheDir = QFileInfo(g_diffFilePath).dir().absoluteFilePath(QLatin1String("Downloads"));
    g_installationDir = detectInstallationDir();

    if (!g_updateCacheDir.exists()) {
        qWarning("No update cache dir. Exiting...");
        return EXIT_FAILURE;
    }

    if (!g_installationDir.exists()) {
        qWarning("Cannot detect installation dir properly. Exiting...");
        return EXIT_FAILURE;
    }

    g_progressDialog = new QProgressDialog(
                QObject::tr("Updating, please do not interrupt the process..."),
                QString(), 0, 0, nullptr,
                Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    g_progressDialog->setAutoReset(false);
    g_progressDialog->setAutoClose(false);
    g_progressDialog->setWindowTitle(AppConstants::LABEL);
    g_progressDialog->show();

    // Give the subject app some time to quit
    QTimer::singleShot(3500, [] {
        readDiffFile();
        updateFiles();
        removeOldFiles();
        launchAndExit();
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

static void load(int argc, char* argv[])
{
    qputenv("QT_FORCE_STDERR_LOGGING", "1");

    // The update shouldn't be blocked
    std::signal(SIGFPE, SIG_IGN);
    std::signal(SIGILL, SIG_IGN);
    std::signal(SIGSEGV, SIG_IGN);
    std::signal(SIGABRT, SIG_IGN);
    std::signal(SIGINT, SIG_IGN);
    std::signal(SIGTERM, SIG_IGN);

    if (argc > 1 && QString::fromUtf8(argv[1]).toInt()) {
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
        QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    }
}

static void unload(int exitCode)
{
    delete g_progressDialog;
    QCoreApplication::exit(exitCode);
}

static void readDiffFile()
{
    QFile file(g_diffFilePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("Cannot open the diff file. Exiting...");
        return unload(EXIT_FAILURE);
    }

    g_differences = QCborValue::fromCbor(file.readAll()).toMap();
    g_progressDialog->setMaximum(g_differences.size() + 1); // +1 for update caches removal

    if (g_differences.isEmpty()) {
        qWarning("The diff file is empty. Exiting...");
        return unload(EXIT_FAILURE);
    }
}

static void updateFiles()
{
    g_progressDialog->setLabelText(QObject::tr("Updating files..."));

    foreach (const QCborValue& key, g_differences.keys()) {
        const QString& relativePath = QDir::cleanPath(key.toString());
        const bool remove = g_differences.value(key).toBool(true);

        if (remove)
            continue;

        if (relativePath.isEmpty()) {
            qWarning("Empty relative path to update. Skipping...");
            continue;
        }

        const QFileInfo infoTo(g_installationDir.filePath(relativePath));
        const QString& canonicalFilePathTo = infoTo.canonicalFilePath();
        const QFileInfo infoFrom(g_updateCacheDir.filePath(relativePath));
        const QString& canonicalFilePathFrom = infoFrom.canonicalFilePath();

        if (canonicalFilePathFrom.isEmpty()) {
            g_errorFlag = true;
            qWarning("Empty file path to update from %s. Skipping...", canonicalFilePathFrom.toUtf8().constData());
            continue;
        }

        if (!canonicalFilePathTo.isEmpty()) {
            if (infoTo == g_myInfo) {
                if (!QFile::rename(canonicalFilePathTo, QCoreApplication::applicationDirPath() + QLatin1String("/Updater.bak"))) {
                    g_errorFlag = true;
                    qWarning("Unable to rename myself. Skipping...");
                    continue;
                }
            } else {
                if (!QFile::remove(canonicalFilePathTo)) {
                    g_errorFlag = true;
                    qWarning("Can't update an existing file %s. Skipping...", canonicalFilePathTo.toUtf8().constData());
                    continue;
                }
            }
        }

        if (!infoTo.dir().mkpath(QLatin1String("."))) {
            g_errorFlag = true;
            qWarning("Can't establish a new folder for a file %s. Skipping...", infoTo.dir().path().toUtf8().constData());
            continue;
        }

        if (!QFile::rename(canonicalFilePathFrom, canonicalFilePathTo)) {
            g_errorFlag = true;
            qWarning("Can't update a file %s. Skipping...", canonicalFilePathTo.toUtf8().constData());
        }

        g_progressDialog->setValue(g_progressDialog->value() + 1);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
}

static void removeOldFiles()
{
    g_progressDialog->setLabelText(QObject::tr("Removing old and redundant files..."));

    foreach (const QCborValue& key, g_differences.keys()) {
        const QString& relativePath = QDir::cleanPath(key.toString());
        const bool remove = g_differences.value(key).toBool(false);

        if (!remove)
            continue;

        if (relativePath.isEmpty()) {
            qWarning("Empty relative path to remove. Skipping...");
            continue;
        }

        const QFileInfo info(g_installationDir.filePath(relativePath));
        const QString& canonicalFilePath = info.canonicalFilePath();

        if (canonicalFilePath.isEmpty()) {
            qWarning("Empty file path to remove %s. Skipping...", canonicalFilePath.toUtf8().constData());
            continue;
        }

        if (info == g_myInfo) {
            if (!QFile::rename(canonicalFilePath, QCoreApplication::applicationDirPath() + QLatin1String("/Updater.bak")))
                qWarning("Unable to rename myself. Skipping...");
        } else {
            if (!QFile::remove(canonicalFilePath))
                qWarning("Unable to remove an old file %s. Skipping...", canonicalFilePath.toUtf8().constData());
        }

        g_progressDialog->setValue(g_progressDialog->value() + 1);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }

    g_updateCacheDir.cdUp(); // Cd up into updates dir (removing Checksums.cbor etc since it contains old info anymore)

    if (!g_updateCacheDir.removeRecursively()) {
        g_errorFlag = true;
        qWarning("Encountered a problem while removing update cache. Skipping...");
    }
}

static void launchAndExit()
{
    g_progressDialog->setValue(g_progressDialog->maximum());
    if (g_errorFlag) {
        g_progressDialog->setLabelText(QObject::tr("Update was completed with warnings. If you encounter<br>"
                                                   "some odd behavior, please re-install the application."));
        g_progressDialog->setCancelButtonText(QObject::tr("Close"));
        QObject::connect(g_progressDialog, &QProgressDialog::canceled, [] {
            QProcess process;
            process.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
            process.setProgram(QCoreApplication::applicationDirPath() + QStringLiteral("/Objectwheel"));
            process.startDetached();
            QTimer::singleShot(200, [] { unload(EXIT_SUCCESS); });
        });
    } else {
        g_progressDialog->setLabelText(QObject::tr("Successful"));
        QTimer::singleShot(2000, [] {
            QProcess process;
            process.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
            process.setProgram(QCoreApplication::applicationDirPath() + QStringLiteral("/Objectwheel"));
            process.startDetached();
            QTimer::singleShot(200, [] { unload(EXIT_SUCCESS); });
        });
    }
    QCoreApplication::processEvents();
}
