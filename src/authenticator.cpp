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

Authenticator::Authenticator()
{
    connect(this, SIGNAL(disconnected()),
      SLOT(onDisconnected()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)),
      SLOT(onError(QAbstractSocket::SocketError)));
    connect(this, SIGNAL(sslErrors(QList<QSslError>)),
      SLOT(onSslErrors(QList<QSslError>)));
    connect(this, SIGNAL(textMessageReceived(QString)),
      SLOT(onTextMessageReceived(QString)));
}

Authenticator* Authenticator::instance()
{
    static Authenticator instance;
    return &instance;
}

void Authenticator::init(const QUrl& host)
{
    _host = host;
}

bool Authenticator::connect(int timeout)
{    
    open(_host);
    ignoreSslErrors();

    Delayer::delay([=] () -> bool {
        return state() == QAbstractSocket::ConnectedState;
    }, true, timeout);

    if (state() != QAbstractSocket::ConnectedState)
        close();

    return state() == QAbstractSocket::ConnectedState;
}

QString Authenticator::readSync(int timeout)
{
     QString incoming;
     if (state() != QAbstractSocket::ConnectedState)
         return incoming;

     Delayer::delay(std::bind(&QString::isEmpty, &_message), false, timeout);

     incoming = _message;
     _message.clear();

     return incoming;
}

void Authenticator::onDisconnected()
{
    _message.clear();
}

void Authenticator::onError(QAbstractSocket::SocketError error)
{
    _message.clear();
}
void Authenticator::onSslErrors(const QList<QSslError> &errors)
{
    _message.clear();
}

void Authenticator::onTextMessageReceived(const QString& message)
{
    _message = message;
}

bool Authenticator::signup(
    const QString& recaptcha,
    const QString& first,
    const QString& last,
    const QString& email,
    const QString& password,
    const QString& country,
    const QString& company,
    const QString& title,
    const QString& phone
    )
{
    if (!connect(TIMEOUT))
        return false;

    sendTextMessage(
        TYPE_SIGNUP + ENDL +
        recaptcha + ENDL +
        first + ENDL +
        last + ENDL +
        email + ENDL +
        password + ENDL +
        country + ENDL +
        company + ENDL +
        title + ENDL +
        phone + ENDL
    );

    const auto& incoming = readSync(TIMEOUT);

    close();

    return incoming == TYPE_SUCCESS;
}

bool Authenticator::verify(const QString& email, const QString& code)
{
    if (!connect(TIMEOUT))
        return false;

    sendTextMessage(
        TYPE_VERIFY + ENDL +
        email + ENDL +
        code + ENDL
    );

    const auto& incoming = readSync(TIMEOUT);

    close();

    return incoming == TYPE_SUCCESS;
}

bool Authenticator::forget(const QString& email)
{
    if (!connect(TIMEOUT))
        return false;

    sendTextMessage(
        TYPE_FORGET + ENDL +
        email + ENDL
    );

    const auto& incoming = readSync(TIMEOUT);

    close();

    return incoming == TYPE_SUCCESS;
}

bool Authenticator::resend(const QString& email)
{
    if (!connect(TIMEOUT))
        return false;

    sendTextMessage(
        TYPE_RESEND + ENDL +
        email + ENDL
    );

    const auto& incoming = readSync(TIMEOUT);

    close();

    return incoming == TYPE_SUCCESS;

}

bool Authenticator::reset(const QString& email, const QString& password, const QString& code)
{
    if (!connect(TIMEOUT))
        return false;

    sendTextMessage(
        TYPE_RESET + ENDL +
        email + ENDL +
        password + ENDL +
        code + ENDL
    );

    const auto& incoming = readSync(TIMEOUT);

    close();

    return incoming == TYPE_SUCCESS;
}

bool Authenticator::login(const QString& email, const QString& password)
{

}