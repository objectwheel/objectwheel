#include <filesystemcompletermodel.h>
#include <QFileSystemWatcher>
#include <QFileIconProvider>
#include <QDirIterator>

FileSystemCompleterModel::FileSystemCompleterModel(QObject* parent) : QStringListModel(parent)
  , m_fileIconProvider(new QFileIconProvider)
  , m_fileSystemWatcher(new QFileSystemWatcher(this))
{
    connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged,
            this, &FileSystemCompleterModel::updateModel);
    connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged,
            this, &FileSystemCompleterModel::updateModel);
}

FileSystemCompleterModel::~FileSystemCompleterModel()
{
    delete m_fileIconProvider;
}

QString FileSystemCompleterModel::rootPath() const
{
    return m_rootPath;
}

void FileSystemCompleterModel::setRootPath(const QString& rootPath)
{
    if (m_rootPath != rootPath) {
        m_rootPath = rootPath;
        if (!m_fileSystemWatcher->files().isEmpty())
            m_fileSystemWatcher->removePaths(m_fileSystemWatcher->files());
        if (!m_fileSystemWatcher->directories().isEmpty())
            m_fileSystemWatcher->removePaths(m_fileSystemWatcher->directories());
        if (!m_rootPath.isEmpty())
            m_fileSystemWatcher->addPath(m_rootPath);
        updateModel();
    }
}

void FileSystemCompleterModel::updateModel()
{
    QStringList files;
    if (!m_rootPath.isEmpty()) {
        QDirIterator it(m_rootPath, {"*"}, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
        while (it.hasNext())
            files.append(it.next());
    }
    setStringList(files);
}

QVariant FileSystemCompleterModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::EditRole) {
        QString path = QDir::toNativeSeparators(QStringListModel::data(index, Qt::EditRole).toString());
        if (path.endsWith(QDir::separator()))
            path.chop(1);
        return QFileInfo(path).fileName();
    }

    if (role == Qt::DisplayRole) {
        QString path = QDir::toNativeSeparators(QStringListModel::data(index, Qt::EditRole).toString());
        if (path.endsWith(QDir::separator()))
            path.chop(1);
        return QDir(m_rootPath).relativeFilePath(path);
    }

    if (role == Qt::DecorationRole) {
        QString path = QDir::toNativeSeparators(QStringListModel::data(index, Qt::EditRole).toString());
        if (path.endsWith(QDir::separator()))
            path.chop(1);
        return m_fileIconProvider->icon(QFileInfo(path));
    }

    return QStringListModel::data(index, role);
}

