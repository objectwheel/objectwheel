#include <apimanager.h>
#include <planmanager.h>
#include <servermanager.h>

ApiManager* ApiManager::s_instance = nullptr;
ApiManager::ApiManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    connect(ServerManager::instance(), &ServerManager::binaryMessageReceived,
            this, &ApiManager::onServerResponse, Qt::QueuedConnection);
}

ApiManager::~ApiManager()
{
    s_instance = nullptr;
}

ApiManager* ApiManager::instance()
{
    return s_instance;
}

void ApiManager::login(const QString& email, const QString& password)
{
    ServerManager::send(ServerManager::Login, email, password);
}

void ApiManager::signup(const QString& first, const QString& last, const QString& email,
                        const QString& password, const QString& country, const QString& company,
                        const QString& title, const QString& phone)
{
    ServerManager::send(ServerManager::Signup, first, last, email,
                        password, country, company, title, phone);
}

void ApiManager::resendSignupCode(const QString& email)
{
    ServerManager::send(ServerManager::ResendSignupCode, email);
}

void ApiManager::completeSignup(const QString& email, const QString& code)
{
    ServerManager::send(ServerManager::CompleteSignup, email, code);
}

void ApiManager::resetPassword(const QString& email)
{
    ServerManager::send(ServerManager::ResetPassword, email);
}

void ApiManager::resendPasswordResetCode(const QString& email)
{
    ServerManager::send(ServerManager::ResendPasswordResetCode, email);
}

void ApiManager::completePasswordReset(const QString& email, const QString& password,
                                       const QString& code)
{
    ServerManager::send(ServerManager::CompletePasswordReset, email, password, code);
}

void ApiManager::subscribe(const QString& email, const QString& password, PlanManager::Plans plan,
                           const QString& creditCardNumber, const QString& creditCardCcv,
                           const QDate& creditCardDate)
{
    ServerManager::send(ServerManager::Subscribe, email, password, plan, creditCardNumber,
                        creditCardCcv, QCborValue(QDateTime(creditCardDate)));
}

void ApiManager::requestCloudBuild(const QString& email, const QString& password, const QString& payloadUid)
{
    ServerManager::send(ServerManager::RequestCloudBuild, email, password, payloadUid);
}

void ApiManager::abortCloudBuild(const QString& buildUid)
{
    ServerManager::send(ServerManager::AbortCloudBuild, buildUid);
}

void ApiManager::onServerResponse(const QByteArray& data)
{
    ServerManager::ServerCommands command = ServerManager::Invalid;
    UtilityFunctions::pullCbor(data, command);

    switch (command) {
    case ServerManager::LoginSuccessful: {
        PlanManager::Plans plan;
        UtilityFunctions::pullCbor(data, command, plan);
        emit loginSuccessful({plan});
    } break;
    case ServerManager::LoginFailure:
        emit loginFailure();
        break;
    case ServerManager::SignupSuccessful:
        emit signupSuccessful();
        break;
    case ServerManager::SignupFailure:
        emit signupFailure();
        break;
    case ServerManager::ResendSignupCodeSuccessful:
        emit resendSignupCodeSuccessful();
        break;
    case ServerManager::ResendSignupCodeFailure:
        emit resendSignupCodeFailure();
        break;
    case ServerManager::CompleteSignupSuccessful:
        emit completeSignupSuccessful();
        break;
    case ServerManager::CompleteSignupFailure:
        emit completeSignupFailure();
        break;
    case ServerManager::ResetPasswordSuccessful:
        emit resetPasswordSuccessful();
        break;
    case ServerManager::ResetPasswordFailure:
        emit resetPasswordFailure();
        break;
    case ServerManager::ResendPasswordResetCodeSuccessful:
        emit resendPasswordResetCodeSuccessful();
        break;
    case ServerManager::ResendPasswordResetCodeFailure:
        emit resendPasswordResetCodeFailure();
        break;
    case ServerManager::CompletePasswordResetSuccessful:
        emit completePasswordResetSuccessful();
        break;
    case ServerManager::CompletePasswordResetFailure:
        emit completePasswordResetFailure();
        break;
    case ServerManager::ResponseCloudBuild:
        emit responseCloudBuild(data);
        break;
    case ServerManager::SubscriptionSuccessful:
        emit subscriptionSuccessful();
        break;
    case ServerManager::SubscriptionFailure:
        emit subscriptionFailure();
        break;
    default:
        break;
    }
}
