/********************************************************************************
**
** MIT License
**
** Copyright (c) 2018 Ömer Göktaş
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
********************************************************************************/

#ifndef FIREBASEDATABASE_H
#define FIREBASEDATABASE_H

#include <QObject>
#include <QPointer>
#include <QNetworkReply>

class QNetworkAccessManager;

class FirebaseDatabase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString baseAddress READ baseAddress WRITE setBaseAddress)

public:
    enum RequestMode { Syncronous, Asynchronous };
    enum Event { Unknown, Put, Patch, KeepAlive, Cancel, AuthRevoked };

    Q_ENUM(RequestMode)
    Q_ENUM(Event)

public:
    explicit FirebaseDatabase(QObject *parent = nullptr);
    explicit FirebaseDatabase(const QString& baseAddress, QObject *parent = nullptr);

    QString baseAddress() const;
    void setBaseAddress(const QString& baseAddress);

public slots:
    void listen(const QString& path = {});
    void stop(const QString& path = {});
    void get(const QString& path, RequestMode mode = Syncronous);
    void put(const QString& path, const QByteArray& data, RequestMode mode = Syncronous);
    void post(const QString& path, const QByteArray& data, RequestMode mode = Syncronous);
    void patch(const QString& path, const QByteArray& data, RequestMode mode = Syncronous);
    void remove(const QString& path, RequestMode mode = Syncronous);

signals:
    void requestFinished(const QByteArray& response);
    void eventOccured(Event event, const QString& rootPath, const QByteArray& response);

private:
    void listen(const QUrl& url, const QString& rootPath);
    void request(const QUrl& url, const QByteArray& verb, const QByteArray& data = {});

private slots:
    void onEventReadReady();
    void onEventFinished();
    void onRequestFinished(QNetworkReply* reply);

private:
    QString m_baseAddress;
    QNetworkAccessManager* m_networkManager;
    QHash<QString, QPointer<QNetworkReply>> m_activeConnections;
    QList<std::function<void()>> m_syncronousRequests;
};

QDebug operator<<(QDebug debug, FirebaseDatabase::Event event);

#endif // FIREBASEDATABASE_H