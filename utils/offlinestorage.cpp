#include <offlinestorage.h>
#include <QQmlEngine>

OfflineStorage::OfflineStorage(QQmlEngine* engine) : QObject(engine)
  , m_engine(engine)
{
}

QString OfflineStorage::offlineStoragePath()
{
    return m_engine->offlineStoragePath();
}

QString OfflineStorage::offlineStorageDatabaseFilePath(const QString& databaseName)
{
    return m_engine->offlineStorageDatabaseFilePath(databaseName);
}

void OfflineStorage::setOfflineStoragePath(const QString& dir)
{
    m_engine->setOfflineStoragePath(dir);
    emit offlineStoragePathChanged();
}
