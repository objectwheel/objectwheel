#ifndef APIMANAGER_H
#define APIMANAGER_H

#include <QObject>
#include <QDateTime>
#include <planmanager.h>

class ApiManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ApiManager)

    friend class ApplicationCore;

public:
    static ApiManager* instance();

    static void login(const QString& email, const QString& password);
    static void signup(const QString& first, const QString& last, const QString& email,
                       const QString& password, const QString& country, const QString& company,
                       const QString& title, const QString& phone);
    static void resendSignupCode(const QString& email);
    static void completeSignup(const QString& email, const QString& code);
    static void resetPassword(const QString& email);
    static void resendPasswordResetCode(const QString& email);
    static void completePasswordReset(const QString& email, const QString& password,
                                      const QString& code);
    static void subscribe(const QString& email, const QString& password, PlanManager::Plans plan,
                          const QString& creditCardNumber = QString(),
                          const QString& creditCardCcv = QString(),
                          const QDate& creditCardDate = QDate());
    static void requestCloudBuild(const QString& email, const QString& password,
                                  const QString& payloadUid);
    static void abortCloudBuild(const QString& buildUid);

private slots:
    void onServerResponse(const QByteArray& data);

signals:
    void loginSuccessful(const QVariantList& userInfo);
    void loginFailure();
    void signupSuccessful();
    void signupFailure();
    void resendSignupCodeSuccessful();
    void resendSignupCodeFailure();
    void completeSignupSuccessful();
    void completeSignupFailure();
    void resetPasswordSuccessful();
    void resetPasswordFailure();
    void resendPasswordResetCodeSuccessful();
    void resendPasswordResetCodeFailure();
    void completePasswordResetSuccessful();
    void completePasswordResetFailure();
    void subscriptionSuccessful();
    void subscriptionFailure();
    void responseCloudBuild(const QByteArray& data);

private:
    explicit ApiManager(QObject* parent = nullptr);
    ~ApiManager() override;

private:
    static ApiManager* s_instance;
};

#endif // APIMANAGER_H