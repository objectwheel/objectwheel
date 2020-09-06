#include <apimanager.h>
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
    ServerManager::send(Login, email, password);
}

void ApiManager::signup(const QString& first, const QString& last, const QString& email,
                        const QString& password, const QString& country, const QString& company,
                        const QString& title, const QString& phone)
{
    ServerManager::send(Signup, first, last, email, password, country, company, title, phone);
}

void ApiManager::resendSignupCode(const QString& email)
{
    ServerManager::send(ResendSignupCode, email);
}

void ApiManager::completeSignup(const QString& email, const QString& code)
{
    ServerManager::send(CompleteSignup, email, code);
}

void ApiManager::resetPassword(const QString& email)
{
    ServerManager::send(ResetPassword, email);
}

void ApiManager::resendPasswordResetCode(const QString& email)
{
    ServerManager::send(ResendPasswordResetCode, email);
}

void ApiManager::completePasswordReset(const QString& email, const QString& password,
                                       const QString& code)
{
    ServerManager::send(CompletePasswordReset, email, password, code);
}

void ApiManager::subscribe(const QString& email, const QString& password, qint64 plan,
                           const QString& cardNumber, const QString& cardCvv,
                           const QDate& cardExpDate)
{
    ServerManager::send(Subscribe, email, password, plan, cardNumber,
                        cardCvv, QCborValue(QDateTime(cardExpDate)));
}

void ApiManager::requestCloudBuild(const QString& email, const QString& password, const QString& payloadUid)
{
    ServerManager::send(RequestCloudBuild, email, password, payloadUid);
}

void ApiManager::abortCloudBuild(const QString& buildUid)
{
    ServerManager::send(AbortCloudBuild, buildUid);
}

void ApiManager::requestCouponTest(const QString& email, const QString& password, const QString& code)
{
    ServerManager::send(RequestCouponTest, email, password, code);
}

void ApiManager::requestSubscriptionPlans(const QString& email, const QString& password)
{
    ServerManager::send(RequestSubscriptionPlans, email, password);
}

void ApiManager::onServerResponse(const QByteArray& data)
{
    Commands command = Invalid;
    UtilityFunctions::pullCbor(data, command);

    switch (command) {
    case LoginSuccessful: {
        qint64 plan;
        UtilityFunctions::pullCbor(data, command, plan);
        emit loginSuccessful(plan);
    } break;

    case LoginFailure:
        emit loginFailure();
        break;

    case SignupSuccessful:
        emit signupSuccessful();
        break;

    case SignupFailure:
        emit signupFailure();
        break;

    case ResendSignupCodeSuccessful:
        emit resendSignupCodeSuccessful();
        break;

    case ResendSignupCodeFailure:
        emit resendSignupCodeFailure();
        break;

    case CompleteSignupSuccessful:
        emit completeSignupSuccessful();
        break;

    case CompleteSignupFailure:
        emit completeSignupFailure();
        break;

    case ResetPasswordSuccessful:
        emit resetPasswordSuccessful();
        break;

    case ResetPasswordFailure:
        emit resetPasswordFailure();
        break;

    case ResendPasswordResetCodeSuccessful:
        emit resendPasswordResetCodeSuccessful();
        break;

    case ResendPasswordResetCodeFailure:
        emit resendPasswordResetCodeFailure();
        break;

    case CompletePasswordResetSuccessful:
        emit completePasswordResetSuccessful();
        break;

    case CompletePasswordResetFailure:
        emit completePasswordResetFailure();
        break;

    case ResponseCloudBuild:
        emit responseCloudBuild(data);
        break;

    case ResponseCouponTest: {
        int discountPercentage;
        UtilityFunctions::pullCbor(data, command, discountPercentage);
        emit responseCouponTest(discountPercentage);
    } break;

    case ResponseSubscriptionPlans: {
        QByteArray plans;
        UtilityFunctions::pullCbor(data, command, plans);
        emit responseSubscriptionPlans(plans);
    } break;

    case SubscriptionSuccessful:
        emit subscriptionSuccessful();
        break;

    case SubscriptionFailure:
        emit subscriptionFailure();
        break;

    default:
        break;
    }
}
