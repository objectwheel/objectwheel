#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <utils_global.h>
#include <QObject>
#include <QVariant>

class UTILS_EXPORT FileManager : public QObject
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
        bool cp(const QString& from, const QString& toDir, const bool content = false, const bool qrc = false) const;
		QStringList ls(const QString& dir) const;
		QStringList lsdir(const QString& dir) const;
		QStringList lsfile(const QString& dir) const;
        QStringList fps(const QString& file, const QString& dir) const;
		QString fname(const QString& name) const;
		QString dname(const QString& name) const;
		qint64 fsize(const QString& name) const;
		qint64 dsize(const QString& name) const;
		QString suffix(const QString& name) const;
		QByteArray rdfile(const QString& file) const;
		int wrfile(const QString& file, const QByteArray& data) const;
        #ifdef QT_NETWORK_LIB
          QByteArray dlfile(const QString& url);
        #endif
		bool isfile(const QString& name) const;
		bool isdir(const QString& name) const;
		QChar separator() const;

	private:
        static bool copyDir(QString from, QString to, bool qrc = false);

};

static inline bool mkfile(const QString& name) { return FileManager().mkfile(name); }
static inline bool mkdir(const QString& name) { return FileManager().mkdir(name); }
static inline bool rm(const QString& name) { return FileManager().rm(name); }
static inline bool rn(const QString& from, const QString& to) { return FileManager().rn(from, to); }
static inline bool rmsuffix(const QString& dir, const QString& suffix) { return FileManager().rmsuffix(dir, suffix); }
static inline bool exists(const QString& name) { return FileManager().exists(name); }
static inline bool mv(const QString& from, const QString& to) { return FileManager().mv(from, to); }
static inline bool cp(const QString& from, const QString& toDir, const bool content = false, const bool qrc = false) { return FileManager().cp(from, toDir, content, qrc); }
static inline QStringList ls(const QString& dir) { return FileManager().ls(dir); }
static inline QStringList lsdir(const QString& dir) { return FileManager().lsdir(dir); }
static inline QStringList lsfile(const QString& dir) { return FileManager().lsfile(dir); }
static inline QStringList fps(const QString& file, const QString& dir) { return FileManager().fps(file, dir); }
static inline QString fname(const QString& name) { return FileManager().fname(name); }
static inline QString dname(const QString& name) { return FileManager().dname(name); }
static inline qint64 fsize(const QString& name) { return FileManager().fsize(name); }
static inline qint64 dsize(const QString& name) { return FileManager().dsize(name); }
static inline QString suffix(const QString& name) { return FileManager().suffix(name); }
static inline QByteArray rdfile(const QString& file) { return FileManager().rdfile(file); }
static inline int wrfile(const QString& file, const QByteArray& data) { return FileManager().wrfile(file, data); }
#ifdef QT_NETWORK_LIB
static inline QByteArray dlfile(const QString& url) { return FileManager().dlfile(url); }
#endif
static inline bool isfile(const QString& name) { return FileManager().isfile(name); }
static inline bool isdir(const QString& name) { return FileManager().isdir(name); }
static inline QChar separator() { return FileManager().separator(); }

#endif // FILEMANAGER_H
