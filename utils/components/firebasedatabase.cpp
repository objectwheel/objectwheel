/********************************************************************************
**
** MIT License
**
** Copyright (C) 2018 Ömer Göktaş
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

/*!
 * For more information, FirebaseDatabase Database REST API:
 * https://firebase.google.com/docs/database/rest/save-data
 * https://firebase.google.com/docs/reference/rest/database
*/

#include <firebasedatabase.h>
#include <QNetworkAccessManager>
#include <QRegularExpression>

namespace {

const char* ROOT_PROPERTY = "firebaseDatabaseRoot";

FirebaseDatabase::Event detectEvent(const QByteArray& line)
{
    QByteArray value;
    int index = line.indexOf(':');
    if (index > 0)
        value = line.right(line.size() - index  - 1);
    value = value.trimmed();
    if (value == "put")
        return FirebaseDatabase::Put;
    if (value == "patch")
        return FirebaseDatabase::Patch;
    if (value == "keep-alive")
        return FirebaseDatabase::KeepAlive;
    if (value == "cancel")
        return FirebaseDatabase::Cancel;
    if (value == "auth_revoked")
        return FirebaseDatabase::AuthRevoked;
    return FirebaseDatabase::Unknown;
}
}

FirebaseDatabase::FirebaseDatabase(const QString& baseAddress, QObject *parent) : QObject(parent)
  , m_baseAddress(baseAddress)
  , m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &FirebaseDatabase::onRequestFinished);
}

FirebaseDatabase::FirebaseDatabase(QObject *parent) : FirebaseDatabase(QString(), parent)
{
}

void FirebaseDatabase::listen(const QString& path)
{
    listen(QUrl(m_baseAddress + path), path);
}

void FirebaseDatabase::stop(const QString& path)
{
    QNetworkReply* reply = m_activeConnections.take(path);
    if (!reply)
        return;

    reply->abort();
    reply->deleteLater();
}

void FirebaseDatabase::get(const QString& path, RequestMode mode)
{
    if (mode == Asynchronous) {
        request(QUrl(m_baseAddress + path), "GET");
    } else {
        std::function<void()> request = std::bind(&FirebaseDatabase::get, this, path, Asynchronous);
        m_syncronousRequests.append(request);
        if (m_syncronousRequests.size() == 1)
            m_syncronousRequests.first()();
    }
}

void FirebaseDatabase::put(const QString& path, const QByteArray& data, RequestMode mode)
{
    if (mode == Asynchronous) {
        request(QUrl(m_baseAddress + path), "PUT", data);
    } else {
        std::function<void()> request = std::bind(&FirebaseDatabase::put, this, path, data, Asynchronous);
        m_syncronousRequests.append(request);
        if (m_syncronousRequests.size() == 1)
            m_syncronousRequests.first()();
    }
}

void FirebaseDatabase::post(const QString& path, const QByteArray& data, RequestMode mode)
{
    if (mode == Asynchronous) {
        request(QUrl(m_baseAddress + path), "POST", data);
    } else {
        std::function<void()> request = std::bind(&FirebaseDatabase::post, this, path, data, Asynchronous);
        m_syncronousRequests.append(request);
        if (m_syncronousRequests.size() == 1)
            m_syncronousRequests.first()();
    }
}

void FirebaseDatabase::patch(const QString& path, const QByteArray& data, RequestMode mode)
{
    if (mode == Asynchronous) {
        request(QUrl(m_baseAddress + path), "PATCH", data);
    } else {
        std::function<void()> request = std::bind(&FirebaseDatabase::patch, this, path, data, Asynchronous);
        m_syncronousRequests.append(request);
        if (m_syncronousRequests.size() == 1)
            m_syncronousRequests.first()();
    }
}

void FirebaseDatabase::remove(const QString& path, RequestMode mode)
{
    if (mode == Asynchronous) {
        request(QUrl(m_baseAddress + path), "DELETE");
    } else {
        std::function<void()> request = std::bind(&FirebaseDatabase::remove, this, path, Asynchronous);
        m_syncronousRequests.append(request);
        if (m_syncronousRequests.size() == 1)
            m_syncronousRequests.first()();
    }
}

void FirebaseDatabase::listen(const QUrl& url, const QString& rootPath)
{
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "text/event-stream");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply* reply = m_networkManager->get(request);
    reply->setProperty(ROOT_PROPERTY, rootPath);
    connect(reply, &QNetworkReply::readyRead, this, &FirebaseDatabase::onEventReadReady);
    connect(reply, &QNetworkReply::finished, this, &FirebaseDatabase::onEventFinished);
    m_activeConnections.insert(rootPath, reply);
}

void FirebaseDatabase::request(const QUrl& url, const QByteArray& verb, const QByteArray& data)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    m_networkManager->sendCustomRequest(request, verb, data);
}

void FirebaseDatabase::onEventReadReady()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);

    const QByteArray& line = reply->readLine();
    Q_ASSERT(!line.isEmpty());

    Event event = detectEvent(line);
    if (event == Unknown) {
        qWarning() << tr("Unknown event arrieved.");
        return;
    }

    const QByteArray& data = reply->readAll();
    const QString& rootPath = reply->property(ROOT_PROPERTY).toString();

    QRegularExpression exp("\\{.*\\}");
    QRegularExpressionMatch match = exp.match(data);

    if (!match.hasMatch())
        return;

    emit eventOccured(event, rootPath, match.captured(0).toUtf8());
}

void FirebaseDatabase::onEventFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);

    const QString& rootPath = reply->property(ROOT_PROPERTY).toString();
    const QUrl& redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (!redirectUrl.isEmpty())
        listen(redirectUrl, rootPath);

    reply->deleteLater();
}

void FirebaseDatabase::onRequestFinished(QNetworkReply* reply)
{
    const QString& contentTypeHeader = reply->header(QNetworkRequest::ContentTypeHeader).toString();

    // We don't want event responses here, cause we handle them above.
    if (reply->property(ROOT_PROPERTY).isValid()
            || contentTypeHeader.contains("text/event-stream")) {
        return;
    }

    if (!m_syncronousRequests.isEmpty())
        m_syncronousRequests.removeFirst();

    emit requestFinished(reply->readAll());

    if (!m_syncronousRequests.isEmpty())
        m_syncronousRequests.first()();
}

QString FirebaseDatabase::baseAddress() const
{
    return m_baseAddress;
}

void FirebaseDatabase::setBaseAddress(const QString& baseAddress)
{
    m_baseAddress = baseAddress;
}

QDebug operator<<(QDebug debug, FirebaseDatabase::Event event)
{
    switch (event) {
    case FirebaseDatabase::Unknown:
        debug << "FirebaseDatabase::Unknown";
        break;
    case FirebaseDatabase::Put:
        debug << "FirebaseDatabase::Put";
        break;
    case FirebaseDatabase::Patch:
        debug << "FirebaseDatabase::Patch";
        break;
    case FirebaseDatabase::KeepAlive:
        debug << "FirebaseDatabase::KeepAlive";
        break;
    case FirebaseDatabase::Cancel:
        debug << "FirebaseDatabase::Cancel";
        break;
    case FirebaseDatabase::AuthRevoked:
        debug << "FirebaseDatabase::AuthRevoked";
        break;
    }
    return debug;
}
