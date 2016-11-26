#ifndef APIAI_H
#define APIAI_H

#include <QObject>

class QWebSocket;
class QByteArray;

class ApiAi : public QObject
{
		Q_OBJECT
		Q_PROPERTY(QString response READ response NOTIFY responseReady)
		Q_PROPERTY(QString token READ token WRITE setToken)
		Q_PROPERTY(QString language READ language WRITE setLanguage)

	private:
		QWebSocket* m_webSocket;
		QString m_token;
		QString m_id;
		QString m_language;
		QByteArray m_voiceData;
		QString m_response;

	public:
		explicit ApiAi(QObject *parent = 0);
		const QString& token() const;
		void setToken(const QString& token);
		static void registerQmlType();
		const QString& language() const;
		void setLanguage(const QString& language);
		const QString& response() const;

	private:
		void commitVoice();
		void openWebSocket();
		QString generateRandomId() const;

	signals:
		void responseReady();

	private slots:
		void handleConnection();
		void handleResponse(const QString& response);

	public slots:
		void sendVoiceData(const QByteArray& data);
};

#endif // APIAI_H
