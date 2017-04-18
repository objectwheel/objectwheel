#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>

class UserManagerPrivate;

class UserManager : public QObject
{
		Q_OBJECT
		Q_DISABLE_COPY(UserManager)

	public:
		explicit UserManager(QObject *parent = 0);
        ~UserManager();
		static UserManager* instance();
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
		static UserManagerPrivate* m_d;
};

#endif // USERMANAGER_H
