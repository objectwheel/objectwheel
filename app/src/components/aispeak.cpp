#include <aispeak.h>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QDebug>

#define URL "https://api.api.ai/v1/tts?v=20150910&text="

class AiSpeakPrivate
{
	public:
		QString id = AiSpeak::generateRandomId();
		QNetworkAccessManager* manager = new QNetworkAccessManager;
		QNetworkReply* reply;
};

AiSpeak::AiSpeak(QObject *parent)
	: QObject(parent)
	, m_d(new AiSpeakPrivate)
	, m_language("en")
{
}

AiSpeak::~AiSpeak()
{
	delete m_d;
}

const QString& AiSpeak::token() const
{
	return m_token;
}

void AiSpeak::setToken(const QString& token)
{
	m_token = token;
}

const QString& AiSpeak::language() const
{
	return m_language;
}

void AiSpeak::setLanguage(const QString& language)
{
	m_language = language;
}

void AiSpeak::speak(const QString& text)
{
	auto url(QUrl(URL + text));
	QNetworkRequest http(url);
	http.setRawHeader("Authorization", QString("Bearer " + m_token).toStdString().c_str());
	http.setRawHeader("Accept-Language", m_language.toStdString().c_str());

	m_d->reply = m_d->manager->get(http);

	connect(m_d->reply, SIGNAL(sslErrors(QList<QSslError>)), m_d->reply, SLOT(ignoreSslErrors()));
	connect(m_d->reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
	connect(m_d->reply, SIGNAL(finished()), this, SLOT(handleResponse()));
}

void AiSpeak::handleResponse()
{
	auto data = m_d->reply->readAll();
	if (m_d->reply->error() == QNetworkReply::NoError)
		emit readySpeak(data);
	m_d->reply->deleteLater();
}

void AiSpeak::handleError(QNetworkReply::NetworkError error)
{
	qWarning() << "AiSpeak::speak() :" << error;
	m_d->reply->deleteLater();
	emit errorOccurred();
}

QString AiSpeak::generateRandomId()
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
#define COMPONENT_NAME "AiSpeak"
#define COMPONENT_VERSION_MAJOR 1
#define COMPONENT_VERSION_MINOR 0
void AiSpeak::registerQmlType()
{
	qmlRegisterType<AiSpeak>(COMPONENT_URI, COMPONENT_VERSION_MAJOR, COMPONENT_VERSION_MINOR, COMPONENT_NAME);
}
#endif
