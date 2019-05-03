#include <accountmanager.h>

AccountManager* AccountManager::s_instance = nullptr;
AccountManager::AccountManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

AccountManager::~AccountManager()
{
    s_instance = nullptr;
}

AccountManager* AccountManager::instance()
{
    return s_instance;
}

void AccountManager::login(const QString& email, const QString& password)
{
    ServerManager::send(ServerManager::Login, email, password);
}

void AccountManager::signup(const QString& first, const QString& last, const QString& email,
                            const QString& password, const QString& country, const QString& company,
                            const QString& title, const QString& phone)
{
    ServerManager::send(ServerManager::Signup, first, last, email,
                        password, country, company, title, phone);
}

void AccountManager::resendSignupCode(const QString& email)
{
    ServerManager::send(ServerManager::ResendSignupCode, email);
}

void AccountManager::completeSignup(const QString& email, const QString& code)
{
    ServerManager::send(ServerManager::CompleteSignup, email, code);
}

void AccountManager::resetPassword(const QString& email)
{
    ServerManager::send(ServerManager::ResetPassword, email);
}

void AccountManager::completePasswordReset(const QString& email, const QString& password,
                                           const QString& code)
{
    ServerManager::send(ServerManager::CompletePasswordReset, email, password, code);
}

void AccountManager::onDataArrival(ServerManager::ServerCommands command, const QByteArray& data)
{
    switch (command) {
    case ServerManager::LoginSuccessful: {
        Plans plan;
        UtilityFunctions::pull(data, plan);
        if (plan != Pro && plan != Enterprise)
            plan = Free;
        emit loginSuccessful(plan);
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
    case ServerManager::CompletePasswordResetSuccessful:
        emit completePasswordResetSuccessful();
        break;
    case ServerManager::CompletePasswordResetFailure:
        emit completePasswordResetFailure();
        break;
    default:
        qWarning("AccountManager: Unrecognized api response");
        break;
    }
}
