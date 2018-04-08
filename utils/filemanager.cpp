#include <filemanager.h>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QEventLoop>

#ifdef QT_NETWORK_LIB
#include <QNetworkAccessManager>
#include <QNetworkReply>
#endif

FileManager::FileManager(QObject *parent)
    : QObject(parent)
{
}

bool FileManager::mkfile(const QString& name) const
{
    QFileInfo info(name);
    if (info.fileName().isEmpty()) return false;
    if (exists(name)) return true;
    if (!info.dir().mkpath(".")) return false;
    QFile file(name);
    if (!file.open(QFile::WriteOnly)) return false;
    file.close();
    return true;
}

bool FileManager::mkdir(const QString& name) const
{
    QDir dir(name);
    return dir.mkpath(".");
}

bool FileManager::rm(const QString& name) const
{
    if (!exists(name)) return true;
    if (QFileInfo(name).isDir()) {
        return QDir(name).removeRecursively();
    } else {
        return QFile::remove(name);
    }
}

bool FileManager::rn(const QString& from, const QString& to) const
{
    return QDir().rename(from, to);
}

bool FileManager::rmsuffix(const QString& dir, const QString& suffix) const
{
    for (auto file : lsfile(dir)) {
        auto filePath = dir + separator() + file;
        if (this->suffix(filePath) == suffix) {
            if(!QFile::remove(filePath)) {
                return false;
            }
        }
    }
    return true;
}

bool FileManager::exists(const QString& name) const
{
    return QFileInfo::exists(name);
}

bool FileManager::mv(const QString& from, const QString& to) const
{
    if (from == to) return true;
    if (!exists(from)) return false;
    if (!exists(to)) {
        if (QFileInfo(from).isDir()) {
            return QDir().rename(from, to);
        } else {
            return QFile::rename(from, to);
        }
    } else {
        if (QFileInfo(from).isDir() && QFileInfo(to).isDir()) {
            if (QFileInfo::exists(to + QDir::separator() + QFileInfo(from).fileName())) return false;
            if (!mkdir(to + QDir::separator() + QFileInfo(from).fileName())) return false;
            if (!copyDir(from, to + QDir::separator() + QFileInfo(from).fileName())) return false;
            return rm(from);
        } else if (QFileInfo(from).isDir() && !QFileInfo(to).isDir()) {
            return false;
        } else if (!QFileInfo(from).isDir() && QFileInfo(to).isDir()) {
            if (!QFile::copy(from, to + QDir::separator() + QFileInfo(from).fileName())) return false;
            return QFile::remove(from);
        } else if (!QFileInfo(from).isDir() && !QFileInfo(to).isDir()) {
            if (!QFile::remove(to)) return false;
            return QFile::rename(from, to);
        } else {
            Q_ASSERT(0); // Unhandled state
            return false;
        }
    }
}

bool FileManager::cp(const QString& from, const QString& toDir, const bool content, const bool qrc) const
{
    if (from == toDir) return true;
    if (!exists(from) || !exists(toDir)) return false;
    if (QFileInfo(from).isDir()) {
        if (content) {
            return copyDir(from, toDir, qrc);
        } else {
            if (!mkdir(toDir + separator() + fname(from))) return false;
            return copyDir(from, toDir + separator() + fname(from), qrc);
        }
    } else {
        if (!QFile::copy(from, toDir + separator() + fname(from)))
            return false;
        if (qrc)
            QFile::setPermissions(toDir + separator() + fname(from), QFile::WriteUser | QFile::ReadUser);
        return true;
    }
}

QStringList FileManager::ls(const QString& dir) const
{
    return QDir(dir).entryList(QDir::AllEntries | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot);
}

QStringList FileManager::lsdir(const QString& dir) const
{
    return QDir(dir).entryList(QDir::AllDirs| QDir::System | QDir::Hidden | QDir::NoDotAndDotDot);
}

QStringList FileManager::lsfile(const QString& dir) const
{
    return QDir(dir).entryList(QDir::Files| QDir::System | QDir::Hidden | QDir::NoDotAndDotDot);
}

QStringList FileManager::fps(const QString& file, const QString& dir) const
{
    QStringList paths;
    for (auto fn : lsfile(dir))
        if (fn == file)
            paths << dir + separator() + fn;

    for (auto dr : lsdir(dir))
        paths << fps(file, dir + separator() + dr);

    return paths;
}

QString FileManager::fname(const QString& name) const
{
    return QFileInfo(name).fileName();
}

QString FileManager::dname(const QString& name) const
{
    return QFileInfo(name).path();
}

qint64 FileManager::fsize(const QString& name) const
{
    return QFileInfo(name).size();
}

qint64 FileManager::dsize(const QString& name) const
{
    quint64 sz = 0;
    for (auto filename : lsfile(name)) {
        sz += fsize(name + separator() + filename);
    }
    for (auto dirname : lsdir(name)) {
        sz += dsize(name + separator() + dirname);
    }
    return sz;
}

QString FileManager::suffix(const QString& name) const
{
    return QFileInfo(name).suffix();
}

QByteArray FileManager::rdfile(const QString& file) const
{
    QByteArray cache;
    QFile reader(file);
    if (!reader.open(QFile::ReadOnly)) return cache;
    cache = reader.readAll();
    reader.close();
    return cache;
}

int FileManager::wrfile(const QString& file, const QByteArray& data) const
{
    if (data.isEmpty()) return -1;
    if (!exists(file) && !mkfile(file)) return -1;
    QFile writer(file);
    if (!writer.open(QFile::WriteOnly)) return -1;
    int ret = writer.write(data);
    writer.close();
    return ret;
}

#ifdef QT_NETWORK_LIB
QByteArray FileManager::dlfile(const QString& url)
{
    QByteArray data;
    QEventLoop loop;
    QNetworkReply* reply;
    QNetworkRequest request;
    QNetworkAccessManager manager;

    request.setUrl(QUrl::fromUserInput(url));
    request.setRawHeader("User-Agent", "Objectwheel");
    reply = manager.get(request);

    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), reply,
      SLOT(ignoreSslErrors()));
    connect(reply, (void(QNetworkReply::*)(QNetworkReply::NetworkError))
      &QNetworkReply::error, this, [&loop] {
        loop.quit();
    });
    connect(reply, &QNetworkReply::finished,
      this, [reply, &data, &loop] {
        data = reply->readAll();
        loop.quit();
    });
    loop.exec();
    return data;
}
#endif

bool FileManager::isfile(const QString& name) const
{
    return QFileInfo(name).isFile();
}

bool FileManager::isdir(const QString& name) const
{
    return QFileInfo(name).isDir();
}

QChar FileManager::separator() const
{
    return '/'/*QDir::separator()*/;
}

bool FileManager::copyDir(QString from, QString to, bool qrc)
{
    QDir directory;
    directory.setPath(from);

    from += QDir::separator();
    to += QDir::separator();

    for (QString file : directory.entryList(QDir::Files)) {
        QString f = from + file;
        QString t = to + file;
        if (!QFile::copy(f, t))
            return false;
        if (qrc)
            QFile::setPermissions(t, QFile::WriteUser | QFile::ReadUser);
    }

    for (QString dir : directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString f = from + dir;
        QString t = to + dir;
        if (!QDir().mkpath(t))
            return false;
        if (!copyDir(f, t, qrc))
            return false;
    }

    return true;
}
