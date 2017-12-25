#ifndef USERBACKEND_H
#define USERBACKEND_H

#include <QObject>

class UserBackendPrivate;

class UserBackend : public QObject
{
		Q_OBJECT
		Q_DISABLE_COPY(UserBackend)

	public:
		explicit UserBackend(QObject *parent = 0);
        ~UserBackend();
		static UserBackend* instance();
		static QString dataDirictory();
		static QString userDirectory(const QString& username);
        static void setAutoLogin(const QString& password);
        static void clearAutoLogin();
        static bool hasAutoLogin();
        static bool tryAutoLogin();
		static bool exists(const QString& username);
		static bool buildNewUser(const QString& username);
		static bool startUserSession(const QString& username, const QString& password);
        static bool startUserSessionWithHash(const QString& username, const QByteArray& hash);
		static QString currentSessionsUser();
        static QString currentSessionsToken();
		static QString currentSessionsKey();
        static QString generateToken(const QString& username, const QString& password);

    public slots:
		static void stopUserSession();

	private:
		static UserBackendPrivate* _d;
};

#endif // USERBACKEND_H
