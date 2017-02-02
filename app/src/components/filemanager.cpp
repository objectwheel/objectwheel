#include <filemanager.h>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>

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
		}
	}
}

bool FileManager::cp(const QString& from, const QString& toDir) const
{
	if (from == toDir) return true;
	if (!exists(from) || !exists(toDir)) return false;
	if (QFileInfo(from).isDir()) {
		if (!mkdir(toDir + separator() + fname(from))) return false;
		return copyDir(from, toDir + separator() + fname(from));
	} else {
		return QFile::copy(from, toDir + separator() + fname(from));
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

QString FileManager::fname(const QString& name) const
{
	return QFileInfo(name).fileName();
}

QString FileManager::dname(const QString& name) const
{
	return QFileInfo(name).path();
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
	if (!mkfile(file)) return -1;
	QFile writer(file);
	if (!writer.open(QFile::WriteOnly)) return -1;
	int ret = writer.write(data);
	writer.close();
	return ret;
}

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

	connect(reply, SIGNAL(sslErrors(QList<QSslError>)), reply, SLOT(ignoreSslErrors()));
	connect(reply, (void(QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error), [&loop] {
		loop.quit();
	});
	connect(reply, &QNetworkReply::finished, [reply, &data, &loop] {
		data = reply->readAll();
		loop.quit();
	});
	loop.exec();
	return data;
}

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
	return QDir::separator();
}

bool FileManager::copyDir(QString from, QString to)
{
	QDir directory;
	directory.setPath(from);

	from += QDir::separator();
	to += QDir::separator();

	for (QString file : directory.entryList(QDir::Files)) {
		QString f = from + file;
		QString t = to + file;
		if (!QFile::copy(f, t)) return false;
	}

	for (QString dir : directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
		QString f = from + dir;
		QString t = to + dir;
		if (!directory.mkpath(t)) return false;
		if (!copyDir(f, t)) return false;
	}

	return true;
}

#ifdef QT_QML_LIB
#include <QQmlEngine>
#define COMPONENT_URI "com.objectwheel.components"
#define COMPONENT_NAME "FileManager"
#define COMPONENT_VERSION_MAJOR 1
#define COMPONENT_VERSION_MINOR 0
void FileManager::registerQmlType()
{
	qmlRegisterType<FileManager>(COMPONENT_URI, COMPONENT_VERSION_MAJOR, COMPONENT_VERSION_MINOR, COMPONENT_NAME);
}
#endif
