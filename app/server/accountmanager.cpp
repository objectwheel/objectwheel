#include <accountmanager.h>
#include <servermanager.h>

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
