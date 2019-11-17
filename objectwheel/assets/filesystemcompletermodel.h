#ifndef FILESYSTEMCOMPLETERMODEL_H
#define FILESYSTEMCOMPLETERMODEL_H

#include <QStringListModel>

class QFileIconProvider;
class QFileSystemWatcher;

class FileSystemCompleterModel final : public QStringListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(FileSystemCompleterModel)

public:
    explicit FileSystemCompleterModel(QObject* parent = nullptr);
    ~FileSystemCompleterModel() override;

    QString rootPath() const;
    void setRootPath(const QString& rootPath);

private slots:
    void updateModel();

private:
    QVariant data(const QModelIndex& index, int role) const override;

private:
    QString m_rootPath;
    QFileIconProvider* m_fileIconProvider;
    QFileSystemWatcher* m_fileSystemWatcher;
};

#endif // FILESYSTEMCOMPLETERMODEL_H
