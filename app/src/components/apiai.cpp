#include <apiai.h>
#include <QWebSocket>
#include <QByteArray>
#include <QDataStream>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#define URL "wss://api-ws.api.ai:4435/api/ws/query?v=20150910&" \
	"content-type=audio/x-raw,+layout=(string)interleaved,+rate=(int)" \
	"16000,+format=(string)S16LE,+channels=(int)1&access_token=%1&sessionId=%2"

#define CONFIG "{\"timezone\":\"America/New_York\", \"lang\":\"%1\", \"sessionId\":\"%2\"}"

class ApiAiPrivate
{
	public:
		QString id = ApiAi::generateRandomId();
		bool flushed = true;
};

ApiAi::ApiAi(QObject *parent)
	: QObject(parent)
	, m_d(new ApiAiPrivate)
	, m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
	, m_language("en")
	, m_error(false)
{
	connect(m_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(handleError(QAbstractSocket::SocketError)));
	connect(m_webSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(handleStateChanges(QAbstractSocket::SocketState)));
	connect(m_webSocket, SIGNAL(textMessageReceived(QString)), this, SIGNAL(readyResponse(QString)));
	connect(m_webSocket, SIGNAL(sslErrors(QList<QSslError>)), m_webSocket, SLOT(ignoreSslErrors()));
}

const QString& ApiAi::token() const
{
	return m_token;
}

void ApiAi::setToken(const QString& token)
{
	m_token = token;
}

const QString& ApiAi::language() const
{
	return m_language;
}

void ApiAi::setLanguage(const QString& language)
{
	m_language = language;
}

bool ApiAi::state() const
{
	if (m_webSocket->state() != QAbstractSocket::ConnectedState || m_error) {
		return false;
	} else {
		return true;
	}
}

bool ApiAi::error() const
{
	return m_error;
}

void ApiAi::open()
{
	if (m_webSocket->state() == QAbstractSocket::UnconnectedState) {
		m_error = false;
		m_d->flushed = true;
		QString url = QString(URL).arg(m_token).arg(m_d->id);
		m_webSocket->open(QUrl(url));
		emit stateChanged();
	}
}

void ApiAi::send(const QByteArray& data)
{
	if (m_webSocket->state() == QAbstractSocket::ConnectedState && !m_error) {
		if (m_d->flushed) {
			m_d->flushed = false;
			QString config = QString(CONFIG).arg(m_language).arg(m_d->id);
			m_webSocket->sendTextMessage(config);
		}
		int buffSize = 4096;
		for (int i = 0; i < data.size(); i+=buffSize) {
			auto slice = data.mid(i, buffSize);
			m_webSocket->sendBinaryMessage(slice);
		}
	} else {
		qWarning() << "ApiAi::send() : Send failed, socket is closed.";
	}
}

void ApiAi::speak(const QString& text)
{
	auto url(QUrl("https://api.api.ai/v1/tts?v=20150910&text=" + text));
	QNetworkRequest http(url);
	http.setRawHeader("Authorization", QString("Bearer " + m_token).toStdString().c_str());
	http.setRawHeader("Accept-Language", m_language.toStdString().c_str());
	auto manager = new QNetworkAccessManager;
	auto reply = manager->get(http);

	connect(reply, SIGNAL(sslErrors(QList<QSslError>)), reply, SLOT(ignoreSslErrors()));
	connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
			[](QNetworkReply::NetworkError code) {
		qWarning() << "ApiAi::speak() :" << code;
	});

	connect(reply, &QNetworkReply::finished, [=] {
		emit readySpeak(reply->readAll());
		manager->deleteLater();
	});
}

void ApiAi::flush()
{
	if (m_webSocket->state() == QAbstractSocket::ConnectedState && !m_error) {
		m_webSocket->sendTextMessage("EOS");
		m_d->flushed = true;
	} else {
		qWarning() << "ApiAi::flush() : Flush failed, socket is closed.";
	}
}

void ApiAi::close()
{
	if (m_webSocket->state() != QAbstractSocket::UnconnectedState && !m_error) {
		m_d->flushed = true;
		m_webSocket->close();
		emit stateChanged();
	}
}

void ApiAi::handleError(QAbstractSocket::SocketError error)
{
	m_error = true;
	m_d->flushed = true;
	qWarning() << "ApiAi::handleError() : Connection error." << error;
	emit stateChanged();
}

void ApiAi::handleStateChanges(QAbstractSocket::SocketState state)
{
	if (!m_error && state == QAbstractSocket::UnconnectedState) {
		m_d->flushed = true;
		qWarning() << "ApiAi::handleStateChanges() : Time limit reached or connection lost.";
		emit stateChanged();
	}
}

QString ApiAi::generateRandomId()
{
	auto s4 = [] {
		return QString::number(65536 + (qrand() % 65536), 16).remove(0, 1);
	};
	qsrand(QDateTime::currentMSecsSinceEpoch());
	return (s4() + s4() + "-" + s4() + "-" + s4() + "-" + s4() + "-" + s4() + s4() + s4());
}

#ifdef QT_QML_LIB
#include <QQmlEngine>
#define COMPONENT_URI "com.objectwheel.components"
#define COMPONENT_NAME "ApiAi"
#define COMPONENT_VERSION_MAJOR 1
#define COMPONENT_VERSION_MINOR 0
void ApiAi::registerQmlType()
{
	qmlRegisterType<ApiAi>(COMPONENT_URI, COMPONENT_VERSION_MAJOR, COMPONENT_VERSION_MINOR, COMPONENT_NAME);
}
#endif
