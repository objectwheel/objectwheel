#include <apiai.h>
#include <limits>
#include <random>

#include <QWebSocket>
#include <QByteArray>
#include <QDataStream>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDebug>

#define URL "wss://api-ws.api.ai:4435/api/ws/query?v=20150910&" \
	"content-type=audio/x-raw,+layout=(string)interleaved,+rate=(int)" \
	"16000,+format=(string)S16LE,+channels=(int)1&access_token=%1&sessionId=%2"

#define CONFIG "{\"timezone\":\"America/New_York\", \"lang\":\"%1\", \"sessionId\":\"%2\"}"

static std::random_device rd;
static std::mt19937 mt(rd());
static std::uniform_int_distribution<int> rand_dist(INT_MIN, INT_MIN);

class ApiAiPrivate
{
	public:
		QString id = ApiAi::generateRandomId();
		bool flushed = true;
};

ApiAi::ApiAi(QObject *parent)
	: QObject(parent)
	, _d(new ApiAiPrivate)
	, m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
	, m_language("en")
	, m_error(false)
{
	connect(m_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(handleError(QAbstractSocket::SocketError)));
	connect(m_webSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(handleStateChanges(QAbstractSocket::SocketState)));
	connect(m_webSocket, SIGNAL(textMessageReceived(QString)), this, SIGNAL(readyResponse(QString)));
	connect(m_webSocket, SIGNAL(sslErrors(QList<QSslError>)), m_webSocket, SLOT(ignoreSslErrors()));
}

ApiAi::~ApiAi()
{
	delete _d;
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
		_d->flushed = true;
		QString url = QString(URL).arg(m_token).arg(_d->id);
		m_webSocket->open(QUrl(url));
		emit stateChanged();
	}
}

void ApiAi::send(const QByteArray& data)
{
	if (m_webSocket->state() == QAbstractSocket::ConnectedState && !m_error) {
		if (_d->flushed) {
			_d->flushed = false;
			QString config = QString(CONFIG).arg(m_language).arg(_d->id);
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

void ApiAi::flush()
{
	if (m_webSocket->state() == QAbstractSocket::ConnectedState && !m_error) {
		m_webSocket->sendTextMessage("EOS");
		_d->flushed = true;
	} else {
		qWarning() << "ApiAi::flush() : Flush failed, socket is closed.";
	}
}

void ApiAi::close()
{
	if (m_webSocket->state() != QAbstractSocket::UnconnectedState && !m_error) {
		_d->flushed = true;
		m_webSocket->close();
		emit stateChanged();
	}
}

void ApiAi::handleError(QAbstractSocket::SocketError error)
{
	m_error = true;
	_d->flushed = true;
	qWarning() << "ApiAi::handleError() : Connection error." << error;
	emit stateChanged();
}

void ApiAi::handleStateChanges(QAbstractSocket::SocketState state)
{
	if (!m_error && state == QAbstractSocket::UnconnectedState) {
		_d->flushed = true;
		qWarning() << "ApiAi::handleStateChanges() : Time limit reached or connection lost.";
		emit stateChanged();
	}
}

QString ApiAi::generateRandomId()
{
	auto s4 = [] {
        return QString::number(65536 + (rand_dist(mt) % 65536), 16).remove(0, 1);
	};
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
