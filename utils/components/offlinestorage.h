#ifndef OFFLINESTORAGE_H
#define OFFLINESTORAGE_H

#include <QObject>

class QQmlEngine;

class OfflineStorage : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OfflineStorage)
    Q_PROPERTY(QString offlineStoragePath READ offlineStoragePath WRITE setOfflineStoragePath NOTIFY offlineStoragePathChanged)

public:
    explicit OfflineStorage(QQmlEngine* engine, QObject* parent = nullptr);

    QString offlineStoragePath();
    Q_INVOKABLE QString offlineStorageDatabaseFilePath(const QString& databaseName);

public slots:
    void setOfflineStoragePath(const QString& dir);

signals:
    void offlineStoragePathChanged();

private:
    QQmlEngine* m_engine;
};

#endif // OFFLINESTORAGE_H