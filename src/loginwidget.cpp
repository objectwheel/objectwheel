#include <loginwidget.h>
#include <fit.h>
#include <delayer.h>
#include <usermanager.h>

#include <QtQml>
#include <QQuickItem>
#include <QtConcurrent>

static bool autologin = false;
static QFutureWatcher<bool> watcher;
const QString password = "ntvmsnbc21";
const QString email = "kozmon@hotmail.com";

LoginWidget::LoginWidget(QWidget *parent)
	: QQuickWidget(parent)
{
	rootContext()->setContextProperty("dpi", fit::ratio());
    setSource(QUrl("qrc:/resources/qmls/loginScreen/main.qml"));
	setResizeMode(SizeRootObjectToView);

    auto autoLoginButton = QQmlProperty::read(rootObject(), "loginScreen.autologinSwitch", engine()).value<QQuickItem*>();
    auto loginButton = QQmlProperty::read(rootObject(), "loginScreen.loginButton", engine()).value<QQuickItem*>();

    connect(loginButton, SIGNAL(loginButtonClicked(QVariant)), this, SLOT(handleLoginButtonClick(QVariant)));
    connect(autoLoginButton, SIGNAL(clicked()), this, SLOT(handleAutoLoginButtonClick()));
    connect(&watcher, SIGNAL(finished()), this, SLOT(handleSessionStart()));
}

void LoginWidget::handleAutoLoginButtonClick()
{
    autologin = !autologin;
}

void LoginWidget::handleLoginButtonClick(const QVariant& /*json*/)
{
    QTimer::singleShot(0, this, &LoginWidget::startSession);
    emit busy(tr("Decryption in progress"));
}

void LoginWidget::handleSessionStart()
{
    if (watcher.result()) {
        if (autologin)
            UserManager::setAutoLogin(password);
        else
            UserManager::clearAutoLogin();
    } else
        qFatal("Fatal : LoginWidget");

    emit done();
}

void LoginWidget::startSession()
{
    typedef bool (*Fn)(const QString&,const QString&);

    UserManager::buildNewUser(email);
    QFuture<bool> future = QtConcurrent::run(
      (Fn) &UserManager::startUserSession, email, password);

    watcher.setFuture(future);
}
