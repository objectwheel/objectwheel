#include <authenticator.h>
#include <delayer.h>
#include <QWebSocket>

#define TIMEOUT 10000
#define ENDL tr("\r\n")
#define TYPE_LOGIN   tr("0x0000")
#define TYPE_SIGNUP  tr("0x0001")
#define TYPE_VERIFY  tr("0x0002")
#define TYPE_FORGET  tr("0x0003")
#define TYPE_RESET   tr("0x0004")
#define TYPE_RESEND  tr("0x0005")
#define TYPE_FAIL    tr("0x0006")
#define TYPE_SUCCESS tr("0x0007")

typedef QCryptographicHash Hasher;

Authenticator* Authenticator::s_instance = nullptr;
QUrl Authenticator::s_host;
QString Authenticator::s_message;

Authenticator::Authenticator(QObject* parent) : QWebSocket(QString(), QWebSocketProtocol::VersionLatest, parent)
{
    s_instance = this;
    connect(this, &Authenticator::disconnected, this, &Authenticator::onDisconnected);
    connect(this, qOverload<QAbstractSocket::SocketError>(&Authenticator::error),
            this, &Authenticator::onError);
    connect(this, &Authenticator::sslErrors, this, &Authenticator::onSslErrors);
    connect(this, &Authenticator::textMessageReceived, this, &Authenticator::onTextMessageReceived);
}

Authenticator::~Authenticator()
{
    s_instance = nullptr;
}

Authenticator* Authenticator::instance()
{
    return s_instance;
}

void Authenticator::setHost(const QUrl& host)
{
    s_host = host;
}

bool Authenticator::connect(int timeout)
{    
    if (instance()->state() == QAbstractSocket::ConnectedState)
        return true;

    instance()->open(s_host);
    instance()->ignoreSslErrors(); // FIXME: Remove this

    Delayer::delay([=] () -> bool {
        return instance()->state() == QAbstractSocket::ConnectedState;
    }, true, timeout);

    if (instance()->state() != QAbstractSocket::ConnectedState)
        instance()->close();

    return instance()->state() == QAbstractSocket::ConnectedState;
}

QString Authenticator::readSync(int timeout)
{
    QString incoming;
    if (instance()->state() != QAbstractSocket::ConnectedState)
        return incoming;

    Delayer::delay(std::bind(&QString::isEmpty, &s_message), false, timeout);

    incoming = s_message;
    s_message.clear();

    return incoming;
}

void Authenticator::onDisconnected()
{
    s_message.clear();
}

void Authenticator::onError(QAbstractSocket::SocketError)
{
    s_message.clear();
}
void Authenticator::onSslErrors(const QList<QSslError>&)
{
    s_message.clear();
}

void Authenticator::onTextMessageReceived(const QString& message)
{
    s_message = message;
}

bool Authenticator::signup(const QString& recaptcha, const QString& first, const QString& last,
                           const QString& email, const QString& password, const QString& country,
                           const QString& company, const QString& title, const QString& phone)
{
    if (!connect(TIMEOUT))
        return false;

    instance()->sendTextMessage(
                TYPE_SIGNUP + ENDL +
                recaptcha + ENDL +
                first + ENDL +
                last + ENDL +
                email + ENDL +
                Hasher::hash(password.toUtf8(), Hasher::Sha256).toHex() + ENDL +
                country + ENDL +
                company + ENDL +
                title + ENDL +
                phone + ENDL
                );

    const auto& incoming = readSync(TIMEOUT);

    instance()->close();

    return incoming == TYPE_SUCCESS;
}

bool Authenticator::verify(const QString& email, const QString& code)
{
    if (!connect(TIMEOUT))
        return false;

    instance()->sendTextMessage(
                TYPE_VERIFY + ENDL +
                email + ENDL +
                code + ENDL
                );

    const auto& incoming = readSync(TIMEOUT);

    instance()->close();

    return incoming == TYPE_SUCCESS;
}

bool Authenticator::forget(const QString& email)
{
    if (!connect(TIMEOUT))
        return false;

    instance()->sendTextMessage(
                TYPE_FORGET + ENDL +
                email + ENDL
                );

    const auto& incoming = readSync(TIMEOUT);

    instance()->close();

    return incoming == TYPE_SUCCESS;
}

bool Authenticator::resend(const QString& email)
{
    if (!connect(TIMEOUT))
        return false;

    instance()->sendTextMessage(
                TYPE_RESEND + ENDL +
                email + ENDL
                );

    const auto& incoming = readSync(TIMEOUT);

    instance()->close();

    return incoming == TYPE_SUCCESS;

}

bool Authenticator::reset(const QString& email, const QString& password, const QString& code)
{
    if (!connect(TIMEOUT))
        return false;

    instance()->sendTextMessage(
                TYPE_RESET + ENDL +
                email + ENDL +
                Hasher::hash(password.toUtf8(), Hasher::Sha256).toHex() + ENDL +
                code + ENDL
                );

    const auto& incoming = readSync(TIMEOUT);

    instance()->close();

    return incoming == TYPE_SUCCESS;
}

QString Authenticator::login(const QString& email, const QString& password)
{
    if (!connect(TIMEOUT))
        return QString();

    instance()->sendTextMessage(
                TYPE_LOGIN + ENDL +
                email + ENDL +
                Hasher::hash(password.toUtf8(), Hasher::Sha256).toHex() + ENDL
                );

    auto incoming = readSync(TIMEOUT);
    QTextStream body(&incoming);

    auto succeed = body.readLine() == TYPE_SUCCESS;
    auto plan = body.readLine();

    if (!succeed) {
        plan = "";
        instance()->close();
    }

    return plan;
}