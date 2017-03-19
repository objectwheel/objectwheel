#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>

class FileManager : public QObject
{
		Q_OBJECT
	public:
		explicit FileManager(QObject *parent = 0);

	public slots:
		bool mkfile(const QString& name) const;
		bool mkdir(const QString& name) const;
		bool rm(const QString& name) const;
		bool rn(const QString& from, const QString& to) const;
		bool rmsuffix(const QString& dir, const QString& suffix) const;
		bool exists(const QString& name) const;
		bool mv(const QString& from, const QString& to) const;
		bool cp(const QString& from, const QString& toDir, const bool content = false) const;
		QStringList ls(const QString& dir) const;
		QStringList lsdir(const QString& dir) const;
		QStringList lsfile(const QString& dir) const;
		QString fname(const QString& name) const;
		QString dname(const QString& name) const;
		QString suffix(const QString& name) const;
		QByteArray rdfile(const QString& file) const;
		int wrfile(const QString& file, const QByteArray& data) const;
		QByteArray dlfile(const QString& url);
		bool isfile(const QString& name) const;
		bool isdir(const QString& name) const;
		QChar separator() const;

	public:
		#ifdef QT_QML_LIB
		static void registerQmlType();
		#endif

	private:
		static bool copyDir(QString from, QString to);

};

static inline bool mkfile(const QString& name) { return FileManager().mkfile(name); }
static inline bool mkdir(const QString& name) { return FileManager().mkdir(name); }
static inline bool rm(const QString& name) { return FileManager().rm(name); }
static inline bool rn(const QString& from, const QString& to) { return FileManager().rn(from, to); }
static inline bool rmsuffix(const QString& dir, const QString& suffix) { return FileManager().rmsuffix(dir, suffix); }
static inline bool exists(const QString& name) { return FileManager().exists(name); }
static inline bool mv(const QString& from, const QString& to) { return FileManager().mv(from, to); }
static inline bool cp(const QString& from, const QString& toDir, const bool content = false) { return FileManager().cp(from, toDir, content); }
static inline QStringList ls(const QString& dir) { return FileManager().ls(dir); }
static inline QStringList lsdir(const QString& dir) { return FileManager().lsdir(dir); }
static inline QStringList lsfile(const QString& dir) { return FileManager().lsfile(dir); }
static inline QString fname(const QString& name) { return FileManager().fname(name); }
static inline QString dname(const QString& name) { return FileManager().dname(name); }
static inline QString suffix(const QString& name) { return FileManager().suffix(name); }
static inline QByteArray rdfile(const QString& file) { return FileManager().rdfile(file); }
static inline int wrfile(const QString& file, const QByteArray& data) { return FileManager().wrfile(file, data); }
static inline QByteArray dlfile(const QString& url) { return FileManager().dlfile(url); }
static inline bool isfile(const QString& name) { return FileManager().isfile(name); }
static inline bool isdir(const QString& name) { return FileManager().isdir(name); }
static inline QChar separator() { return FileManager().separator(); }

#endif // FILEMANAGER_H
