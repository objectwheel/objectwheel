#include <apiai.h>
#include <QWebSocket>
#include <QNetworkReply>
#include <QDateTime>
#include <QByteArray>
#include <QDebug>

#ifdef QT_QML_LIB
#include <QQmlEngine>
#endif

#define COMPONENT_URI "com.objectwheel.components"
#define COMPONENT_NAME "ApiAi"
#define COMPONENT_DIR "/components/api-ai"
#define COMPONENT_VERSION_MAJOR 1
#define COMPONENT_VERSION_MINOR 0

#define URL "wss://api-ws.api.ai:4435/api/ws/query?v=20150910&" \
	"content-type=audio/x-raw,+layout=(string)interleaved,+rate=(int)" \
	"16000,+format=(string)S16LE,+channels=(int)1&access_token=%1&sessionId=%2"

ApiAi::ApiAi(QObject *parent)
	: QObject(parent)
	, m_webSocket(new QWebSocket("", QWebSocketProtocol::VersionLatest, this))
	, m_id(generateRandomId())
	, m_language("en")
{
	connect(m_webSocket, SIGNAL(connected()), this, SLOT(handleConnection()));
	connect(m_webSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(handleResponse(QString)));
	connect(m_webSocket, SIGNAL(sslErrors(QList<QSslError>)), m_webSocket, SLOT(ignoreSslErrors()));
	connect(m_webSocket, static_cast<void(QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error),
			[=](QAbstractSocket::SocketError error) {
		qWarning() << "Api.Ai : Websocket Error " << error;
	});
	connect(m_webSocket, &QWebSocket::disconnected, [=] {
		qWarning() << "Api.Ai : Websocket Disconnected!";
	});

}

const QString& ApiAi::language() const
{
	return m_language;
}

void ApiAi::setLanguage(const QString& language)
{
	m_language = language;
}

const QString& ApiAi::token() const
{
	return m_token;
}

void ApiAi::setToken(const QString& token)
{
	m_token = token;
}

const QString& ApiAi::response() const
{
	return m_response;
}

void ApiAi::handleConnection()
{
	commitVoice();
}

void ApiAi::handleResponse(const QString& response)
{
	m_response = response;
	emit responseReady();
}

void ApiAi::openWebSocket()
{
	QString url = QString(URL).arg(m_token).arg(m_id);
	m_webSocket->open(QUrl(url));
}

void ApiAi::commitVoice()
{
	QString config = "{\"timezone\":\"America/New_York\", \"lang\":\"%1\", \"sessionId\":\"%2\"}";
	config = config.arg(m_language).arg(m_id);
	m_webSocket->sendTextMessage(config);
	int buffSize = 1024;
	for (int i = 0; i < m_voiceData.size(); i+=buffSize) {
		auto slice = m_voiceData.mid(i, buffSize);
		m_webSocket->sendBinaryMessage(slice);
	}
	m_webSocket->sendTextMessage("EOS");
}

void ApiAi::sendVoiceData(const QByteArray& data)
{
	m_voiceData = data;
	if (m_webSocket->state() == QAbstractSocket::UnconnectedState) {
		openWebSocket();
		return;
	}
	commitVoice();
}

QString ApiAi::generateRandomId() const
{
	auto s4 = [] {
		return QString::number(65536 + (qrand() % 65536), 16).remove(0, 1);
	};
	qsrand(QDateTime::currentMSecsSinceEpoch());
	return (s4() + s4() + "-" + s4() + "-" + s4() + "-" + s4() + "-" + s4() + s4() + s4());
}

void ApiAi::registerQmlType()
{
#ifdef QT_QML_LIB
	qmlRegisterType<ApiAi>(COMPONENT_URI, COMPONENT_VERSION_MAJOR, COMPONENT_VERSION_MINOR, COMPONENT_NAME);
#else
	qWarning("WARNING! ApiAi::registerQmlType() : QtQml module not included.");
#endif
}
