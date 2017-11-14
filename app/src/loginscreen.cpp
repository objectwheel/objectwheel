#include <loginscreen.h>
#include <fit.h>
#include <delayer.h>
#include <projectmanager.h>
#include <usermanager.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <usermanager.h>
#include <savemanager.h>
#include <projectsscreen.h>
#include <dirlocker.h>
#include <mainwindow.h>
#include <global.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtConcurrent>
#include <QQmlEngine>
#include <QDateTime>
#include <QTimer>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>

#define cW (MainWindow::instance()->centralWidget())
#define pW (MainWindow::instance()->progressWidget())

using namespace Fit;

static bool autologin = false;
QQuickItem* loginScreen;
QQuickItem* emailTextInput;
QQuickItem* passwordTextInput;
QQuickItem* autoLoginButton;
QQuickItem* loginButton;
QQuickItem* aboutButton;
QQuickItem* lostPasswordButton;
QQuickItem* toast;

LoginScreen::LoginScreen(QWidget *parent)
	: QQuickWidget(parent)
{
	rootContext()->setContextProperty("dpi", Fit::ratio());
    setSource(QUrl("qrc:/resources/qmls/loginScreen/main.qml"));
	setResizeMode(SizeRootObjectToView);

    toast = (QQuickItem*)QQmlProperty::read(rootObject(), "toast", engine()).value<QObject*>();
    loginScreen = (QQuickItem*)QQmlProperty::read(rootObject(), "loginScreen", engine()).value<QObject*>();
    autoLoginButton = (QQuickItem*)QQmlProperty::read(rootObject(), "loginScreen.autologinSwitch", engine()).value<QObject*>();
    loginButton = (QQuickItem*)QQmlProperty::read(rootObject(), "loginScreen.loginButton", engine()).value<QObject*>();
    aboutButton = (QQuickItem*)QQmlProperty::read(rootObject(), "loginScreen.aboutButton", engine()).value<QObject*>();
    lostPasswordButton = (QQuickItem*)QQmlProperty::read(rootObject(), "loginScreen.lostPassButton", engine()).value<QObject*>();
    emailTextInput = (QQuickItem*)QQmlProperty::read(rootObject(), "loginScreen.emailTextInput", engine()).value<QObject*>();
    passwordTextInput = (QQuickItem*)QQmlProperty::read(rootObject(), "loginScreen.passwordTextInput", engine()).value<QObject*>();

    connect(aboutButton, SIGNAL(clicked()), this, SLOT(handleAboutButtonClicked()));
    connect(lostPasswordButton, SIGNAL(clicked()), this, SLOT(handleLostPasswordButtonClicked()));
    connect(loginButton, SIGNAL(loginButtonClicked(QVariant)), this, SLOT(handleLoginButtonClicked(QVariant)));
    connect(autoLoginButton, SIGNAL(clicked()), this, SLOT(handleAutoLoginButtonClicked()));
}

void LoginScreen::handleAutoLoginButtonClicked()
{
    autologin = !autologin;
    if (autologin) {
        QQmlProperty::write(toast, "text.text", "We do not recommend automatic login for security reasons.");
        QQmlProperty::write(toast, "base.width", qFloor(fit(280)));
        QQmlProperty::write(toast, "base.height", qFloor(fit(65)));
        QQmlProperty::write(toast, "duration", 5000);
        QMetaObject::invokeMethod(toast, "show");
    }
}

void LoginScreen::handleAboutButtonClicked()
{
    cW->showWidget(Screen::About);
}

void LoginScreen::handleLostPasswordButtonClicked()
{
    //TODO:
}

void LoginScreen::handleLoginButtonClicked(const QVariant& json)
{
    auto jobj = QJsonDocument::fromJson(json.toByteArray()).object();
    auto email = jobj["email"].toString();
    auto password = jobj["password"].toString();
    auto userManager = UserManager::instance();

    auto keyHash = QCryptographicHash::hash(QByteArray().insert(0, password), QCryptographicHash::Sha3_512);
    keyHash = QCryptographicHash::hash(keyHash, QCryptographicHash::Md5).toHex();

//    email = "kozmon@hotmail.com", password="ntvmsnbc21";
//    userManager->buildNewUser(email);
//    auto ret = QtConcurrent::run((bool (*)(const QString&,const QString&))(&UserManager::startUserSession), email, password);
//    while(ret.isRunning()) qApp->processEvents(QEventLoop::AllEvents, 10);
//    if (autologin) userManager->setAutoLogin(password); else userManager->clearAutoLogin();
//    ProjectsScreen::refreshProjectList();
//    pW->hideProgress();
//    cW->showWidget(Screen::Projects);
//    clearGUI();
//    return;

    if (userManager->exists(email)) {
        if (DirLocker::locked(userManager->userDirectory(email))) {
            if (DirLocker::canUnlock(userManager->userDirectory(email), keyHash)) {
                pW->showProgress();
                auto ret = QtConcurrent::run((bool (*)(const QString&,const QString&))(&UserManager::startUserSession), email, password);
                Delayer::delay(&ret, &QFuture<bool>::isRunning);
                if (ret.result()) {
                    if (autologin) userManager->setAutoLogin(password); else userManager->clearAutoLogin();
                    ProjectsScreen::refreshProjectList();
                    pW->hideProgress();
                    cW->showWidget(Screen::Projects);
                    clearGUI();
                } else {
                    QQmlProperty::write(toast, "text.text", "Unfortunately your database is corrupted. 0x01");
                    QQmlProperty::write(toast, "base.width", qFloor(fit(280)));
                    QQmlProperty::write(toast, "base.height", qFloor(fit(65)));
                    QQmlProperty::write(toast, "duration", 5000);
                    QMetaObject::invokeMethod(toast, "show");
                    //FIXME: when sync part done.
                    //FIXME: Show loading indicator when "login" clicked, do not freeze screen
                }
            } else {
                pW->hideProgress();
                QMetaObject::invokeMethod(loginScreen, "animateWrongPass");
            }
        } else {
            auto manager = new QNetworkAccessManager(this);
            auto body = QString("{\"token\" : \"%1\"}").arg(userManager->generateToken(email, password));
            auto url = QUrl("https://165.227.164.200/api/v1/registration/check");
            QNetworkRequest http(url);
            http.setRawHeader("content-type", "application/json");
            QNetworkReply* reply = manager->post(http, QByteArray().insert(0, body));
            connect(reply, &QNetworkReply::finished, this, [=] {
                auto jobj = QJsonDocument::fromJson(reply->readAll()).object();
                if (jobj["result"].toString() == "OK") {
                    pW->showProgress();
                    auto ret = QtConcurrent::run((bool (*)(const QString&,const QString&))(&UserManager::startUserSession), email, password);
                    Delayer::delay(&ret, &QFuture<void>::isRunning);
                    if (ret.result()) {
                        if (autologin) userManager->setAutoLogin(password); else userManager->clearAutoLogin();
                        ProjectsScreen::refreshProjectList();
                        pW->hideProgress();
                        cW->showWidget(Screen::Projects);
                        clearGUI();
                    } else {
                        QQmlProperty::write(toast, "text.text", "Unfortunately your database is corrupted. 0x02");
                        QQmlProperty::write(toast, "base.width", qFloor(fit(280)));
                        QQmlProperty::write(toast, "base.height", qFloor(fit(65)));
                        QQmlProperty::write(toast, "duration", 5000);
                        QMetaObject::invokeMethod(toast, "show");
                        //FIXME: when sync part done.
                    }
                } else if (jobj["result"].toString() == "EMAIL") {
                    QMetaObject::invokeMethod(loginScreen, "animateWrongEmail");
                } else if (jobj["result"].toString() == "PASSWORD") {
                    QMetaObject::invokeMethod(loginScreen, "animateWrongPass");
                }
                reply->deleteLater();
            });
            connect(reply, (void (QNetworkReply::*)(QList<QSslError>))
              &QNetworkReply::sslErrors, this, [=] { reply->ignoreSslErrors(); });
            connect(reply, (void (QNetworkReply::*)(QNetworkReply::NetworkError))
              &QNetworkReply::error, this, [=]
            {
                QQmlProperty::write(toast, "text.text", "Your local data is unencrypted therefore we cannot verify your account locally. Please connect to the internet for login.");
                QQmlProperty::write(toast, "base.width", qFloor(fit(330)));
                QQmlProperty::write(toast, "base.height", qFloor(fit(95)));
                QQmlProperty::write(toast, "duration", 10000);
                QMetaObject::invokeMethod(toast, "show");
            });
        }
    } else {
        auto manager = new QNetworkAccessManager(this);
        auto body = QString("{\"token\" : \"%1\"}").arg(userManager->generateToken(email, password));
        auto url = QUrl("https://165.227.164.200/api/v1/registration/check");
        QNetworkRequest http(url);
        http.setRawHeader("content-type", "application/json");
        QNetworkReply* reply = manager->post(http, QByteArray().insert(0, body));
        connect(reply, &QNetworkReply::finished, this, [=] {
            auto jobj = QJsonDocument::fromJson(reply->readAll()).object();
            if (jobj["result"].toString() == "OK") {
                userManager->buildNewUser(email);
                auto ret = QtConcurrent::run((bool (*)(const QString&,const QString&))(&UserManager::startUserSession), email, password);
                Delayer::delay(&ret, &QFuture<void>::isRunning);
                if (autologin) userManager->setAutoLogin(password); else userManager->clearAutoLogin();
                ProjectsScreen::refreshProjectList();
                pW->hideProgress();
                cW->showWidget(Screen::Projects);
                clearGUI();
            } else if (jobj["result"].toString() == "EMAIL") {
                QMetaObject::invokeMethod(loginScreen, "animateWrongEmail");
            } else if (jobj["result"].toString() == "PASSWORD") {
                QMetaObject::invokeMethod(loginScreen, "animateWrongPass");
            }
            reply->deleteLater();
        });
        connect(reply, (void (QNetworkReply::*)(QList<QSslError>))
          &QNetworkReply::sslErrors, this, [=] { reply->ignoreSslErrors(); });
        connect(reply, (void (QNetworkReply::*)(QNetworkReply::NetworkError))
          &QNetworkReply::error, this, [=]
        {
            QQmlProperty::write(toast, "text.text", "In order to activate your account, internet connection is required for the first time login.");
            QQmlProperty::write(toast, "base.width", qFloor(fit(330)));
            QQmlProperty::write(toast, "base.height", qFloor(fit(80)));
            QQmlProperty::write(toast, "duration", 10000);
            QMetaObject::invokeMethod(toast, "show");
        });
    }
}

void LoginScreen::clearGUI()
{
    autologin = false;
    QQmlProperty::write(autoLoginButton, "checked", false);
    QQmlProperty::write(passwordTextInput, "text", "");
    QQmlProperty::write(emailTextInput, "text", "");
}

