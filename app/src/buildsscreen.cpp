#include <buildsscreen.h>
#include <fit.h>
#include <projectmanager.h>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>
#include <usermanager.h>
#include <QDateTime>
#include <QTimer>
#include <QMessageBox>
#include <splashscreen.h>
#include <scenemanager.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <usermanager.h>
#include <toolsmanager.h>
#include <savemanager.h>
#include <splashscreen.h>
#include <scenemanager.h>
#include <QtConcurrent>
#include <QQmlEngine>
#include <projectsscreen.h>
#include <dirlocker.h>
#include <QDebug>
#include <zipper.h>
#include <QStandardPaths>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#define URL QString("https://127.0.0.1/api/v1/build/")

class BuildsScreenPrivate {

    public:
        BuildsScreenPrivate(BuildsScreen* w);
        BuildsScreen* parent;
        QQuickItem* buildPage;
};

BuildsScreenPrivate* BuildsScreen::m_d = nullptr;

BuildsScreenPrivate::BuildsScreenPrivate(BuildsScreen* w) : parent(w)
{

}

BuildsScreen::~BuildsScreen()
{
    delete m_d;
}

BuildsScreen* BuildsScreen::instance()
{
    return m_d->parent;
}

BuildsScreen::BuildsScreen(QWidget *parent) : QQuickWidget(parent)
{
    if (m_d) return;
    m_d = new BuildsScreenPrivate(this);
	rootContext()->setContextProperty("dpi", Fit::ratio());
    setSource(QUrl("qrc:/resources/qmls/buildsScreen/main.qml"));
	setResizeMode(SizeRootObjectToView);

    m_d->buildPage = (QQuickItem*)QQmlProperty::read(rootObject(), "buildPage", engine()).value<QObject*>();
    connect(m_d->buildPage, SIGNAL(btnBuildClicked()), this, SLOT(handleBuildButtonClicked()));
}

void BuildsScreen::handleBuildButtonClicked()
{
    auto buildLabel = QQmlProperty::read(m_d->buildPage, "currentBuildLabel").toString();
    auto savesDir = SaveManager::savesDirectory();
    if (savesDir.isEmpty()) return;
    auto projectFilename = QStandardPaths::standardLocations(QStandardPaths::TempLocation)[0] + separator() + "objectwheel_project.zip";
    Zipper::compressDir(savesDir, projectFilename, "dashboard");
    QByteArray data = rdfile(projectFilename);
    rm(projectFilename);
    QByteArray boundary = "-----------------------------7d935033608e2";
    QByteArray body = "\r\n--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"project\"; filename=\"project.zip\"\r\n";
    body += "Content-Type: application/octet-stream\r\n\r\n";
    body += data;
    body += "\r\n--" + boundary + "--\r\n";

    QNetworkAccessManager* networkAccessManager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl(URL + buildLabel));
    request.setRawHeader("Content-Type","multipart/form-data; boundary=-----------------------------7d935033608e2");
    request.setHeader(QNetworkRequest::ContentLengthHeader,body.size());
    QNetworkReply *reply = networkAccessManager->post(request, body);

    connect(reply, &QNetworkReply::finished, [=] {
//        auto jobj = QJsonDocument::fromJson(reply->readAll()).object();
//        if (jobj["result"].toString() == "OK") {
//            userManager->buildNewUser(email);
//            auto ret = QtConcurrent::run((bool (*)(const QString&,const QString&))(&UserManager::startUserSession), email, password);
//            while(ret.isRunning()) qApp->processEvents(QEventLoop::AllEvents, 20);
//            if (autologin) userManager->setAutoLogin(password); else userManager->clearAutoLogin();
//            ProjectsScreen::refreshProjectList();
//            SplashScreen::hide();
//            SceneManager::show("projectsScene", SceneManager::ToLeft);
//            clearGUI();
//        } else if (jobj["result"].toString() == "EMAIL") {
//            QMetaObject::invokeMethod(loginScreen, "animateWrongEmail");
//        } else if (jobj["result"].toString() == "PASSWORD") {
//            QMetaObject::invokeMethod(loginScreen, "animateWrongPass");
//        }
    });
    connect(reply, (void (QNetworkReply::*)(QList<QSslError>))&QNetworkReply::sslErrors, [=] { reply->ignoreSslErrors(); });
    connect(reply, (void (QNetworkReply::*)(QNetworkReply::NetworkError))&QNetworkReply::error, [=](QNetworkReply::NetworkError e)
    {
        qDebug() << e;
//        QQmlProperty::write(toast, "text.text", "In order to activate your account, internet connection is required for the first time login.");
//        QQmlProperty::write(toast, "base.width", 330);
//        QQmlProperty::write(toast, "base.height", 80);
//        QQmlProperty::write(toast, "duration", 10000);
//        QMetaObject::invokeMethod(toast, "show");
    });
}
