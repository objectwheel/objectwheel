#include <buildsscreen.h>
#include <fit.h>
#include <projectmanager.h>
#include <usermanager.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <usermanager.h>
#include <savemanager.h>
#include <projectsscreen.h>
#include <dirlocker.h>
#include <zipper.h>
#include <flatbutton.h>
#include <global.h>
#include <mainwindow.h>
#include <global.h>

#include <QStandardPaths>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QList>
#include <QElapsedTimer>
#include <QString>
#include <QApplication>
#include <QPointer>
#include <QDebug>
#include <QtMath>
#include <QQmlEngine>
#include <QDateTime>
#include <QTimer>
#include <QMessageBox>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>

#define URL QString(SERVER + "/api/v1/build/")



class BuildsScreenPrivate {

    public:
        BuildsScreenPrivate(BuildsScreen* w);
        QList<qint64> bytes;
        QList<qint64> times;
        QElapsedTimer elapsedTimer;
        BuildsScreen* parent;
        QQuickItem* buildPage;
        QQuickItem* progressPage;
        QQuickItem* toast;
        QQuickItem* swipeView;
        QNetworkAccessManager* manager;
        QPointer<QNetworkReply> reply;
        FlatButton exitButton;
        QString bytesString(const qint64 size, bool withExt);
        QString determineBuildExtension(const QString label);
};

BuildsScreenPrivate* BuildsScreen::_d = nullptr;

BuildsScreenPrivate::BuildsScreenPrivate(BuildsScreen* w)
    : parent(w)
    , manager(new QNetworkAccessManager(parent))
{
    parent->rootContext()->setContextProperty("dpi", fit::ratio());
    parent->setSource(QUrl("qrc:/resources/qmls/buildsScreen/main.qml"));
    parent->setResizeMode(QQuickWidget::SizeRootObjectToView);
    parent->setWindowTitle("Objectwheel Builds");
    parent->setWindowModality(Qt::ApplicationModal);
    parent->setWindowFlags(Qt::CustomizeWindowHint |
      Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint);

    exitButton.setParent(parent);
    exitButton.setIconButton(true);
    exitButton.setIcon(QIcon(":/resources/images/delete-icon.png"));
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE)
    exitButton.setGeometry(parent->width() - fit::fx(26), fit::fx(8), fit::fx(18), fit::fx(18));
#else
    exitButton.setGeometry(parent->width() - fit::fx(15), fit::fx(5), fit::fx(8), fit::fx(8));
#endif
    QObject::connect((BuildsScreen*)parent,  &BuildsScreen::resized, [=]{
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE)
        exitButton.setGeometry(parent->width() - fit::fx(26), fit::fx(8), fit::fx(18), fit::fx(18));
#else
        exitButton.setGeometry(parent->width() - fit::fx(15), fit::fx(5), fit::fx(8), fit::fx(8));
#endif
    });
    fit::fx(&exitButton, fit::both);
    exitButton.show();

    QObject::connect(&exitButton, &FlatButton::clicked, [=]{
        parent->hide();
    });

    buildPage = (QQuickItem*)QQmlProperty::read(parent->rootObject(),
      "buildPage", parent->engine()).value<QObject*>();
    progressPage = (QQuickItem*)QQmlProperty::read(parent->rootObject(),
      "progressPage", parent->engine()).value<QObject*>();
    toast = (QQuickItem*)QQmlProperty::read(parent->rootObject(),
      "toast",parent-> engine()).value<QObject*>();
    swipeView = (QQuickItem*)QQmlProperty::read(parent->rootObject(),
      "swipeView", parent->engine()).value<QObject*>();

    QObject::connect(buildPage, SIGNAL(btnBuildClicked()),
      parent, SLOT(handleBuildButtonClicked()));
    QObject::connect(progressPage, SIGNAL(btnOkClicked()),
      parent, SLOT(handleBtnOkClicked()));
    QObject::connect(progressPage, SIGNAL(btnCancelClicked()),
      parent, SLOT(handleBtnCancelClicked()));
    QObject::connect(qApp, SIGNAL(aboutToQuit()), parent,
      SLOT(handleBtnCancelClicked()));
}

QString BuildsScreenPrivate::bytesString(const qint64 size, bool withExt)
{
    QString ret;
    float kb = 1024.0f;
    float mb = 1048576.0f;
    float gb = 1073741824.0f;

    if (size < kb) {
        ret = QString::number(size);
        if (withExt) ret += " Bytes";
    } else if (size < mb) {
        ret = QString::number(size / kb, 'f', 1);
        if (withExt) ret += " Kb";
    } else if (size < gb) {
        ret = QString::number(size / mb, 'f', 1);
        if (withExt) ret += " Mb";
    } else {
        ret = QString::number(size / gb, 'f', 2);
        if (withExt) ret += " Gb";
    }

    return ret;
}

QString BuildsScreenPrivate::determineBuildExtension(const QString label)
{
    if (label.contains("android")) {
        return QString(".apk");
    } else if (label.contains("ios")) {
        return QString(".ipa");
    } else if (label.contains("windows")) {
        return QString(".exe");
    } else if (label.contains("linux")) {
        return QString(".run");
    } else if (label.contains("raspi")) {
        return QString(".run");
    }  else if (label.contains("macos")) {
        return QString(".tar.gz");
    } else {
        return QString();
    }
}

BuildsScreen::~BuildsScreen()
{
    delete _d;
}

BuildsScreen* BuildsScreen::instance()
{
    return _d->parent;
}

BuildsScreen::BuildsScreen(QWidget *parent) : QQuickWidget(parent)
{
    if (_d) return;
    _d = new BuildsScreenPrivate(this);
}

void BuildsScreen::resizeEvent(QResizeEvent* event)
{
    QQuickWidget::resizeEvent(event);
    emit resized();
}

void BuildsScreen::handleBuildButtonClicked()
{
    auto buildLabel = QQmlProperty::read(_d->buildPage, "currentBuildLabel").toString();
    auto savesDir = SaveManager::basePath();
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

    QNetworkRequest request(QUrl(URL + buildLabel));
    request.setRawHeader("Content-Type","multipart/form-data; boundary=-----------------------------7d935033608e2");
    request.setRawHeader("token", QByteArray().insert(0, QString("{\"token\" : \"%1\"}").arg(UserManager::currentSessionsToken())));
    request.setHeader(QNetworkRequest::ContentLengthHeader,body.size());
    _d->reply = _d->manager->post(request, body);

    QQmlProperty::write(_d->swipeView, "currentIndex", 1);
    QQmlProperty::write(_d->progressPage, "informativeText", "Establishing connection");
    QQmlProperty::write(_d->progressPage, "mbText", "-");
    QQmlProperty::write(_d->progressPage, "speedText", "-");
    QQmlProperty::write(_d->progressPage, "progressbarValue", 0.0);
    QMetaObject::invokeMethod(_d->progressPage, "startWaitEffect");
    QMetaObject::invokeMethod(_d->progressPage, "showBtnCancel");
    _d->exitButton.hide();

    connect(_d->reply, &QNetworkReply::finished, this, [=] {
        if (!_d->reply->isOpen() || !_d->reply->isReadable() || _d->reply->error() != QNetworkReply::NoError) {
            _d->elapsedTimer.invalidate();
            _d->times.clear();
            _d->bytes.clear();
            if (_d->reply) _d->reply->deleteLater();
            return;
        }

        QQmlProperty::write(_d->progressPage, "btnCancelEnabled", false);
        auto response = _d->reply->readAll();
        QString name = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0] + separator() + buildLabel;

        QString nname;
        int count = 0;
        do {
            if (count == 0) {
                nname = name + _d->determineBuildExtension(buildLabel);
            } else {
                nname = name + "_" + QString::number(count) + _d->determineBuildExtension(buildLabel);
            }
            count++;
        } while (exists(nname));

        wrfile(nname, response);

        QQmlProperty::write(_d->progressPage, "informativeText", "Done");
        QMetaObject::invokeMethod(_d->progressPage, "stopWaitEffect");
        QMetaObject::invokeMethod(_d->progressPage, "showBtnOk");
        QQmlProperty::write(_d->progressPage, "btnCancelEnabled", true);

        _d->elapsedTimer.invalidate();
        _d->times.clear();
        _d->bytes.clear();
        if (_d->reply) _d->reply->deleteLater();
    });
    connect(_d->reply, (void (QNetworkReply::*)(qint64, qint64))
      &QNetworkReply::downloadProgress, this, [=](qint64 bytesSent, qint64 bytesTotal) {
        QQmlProperty::write(_d->progressPage, "informativeText", "Downloading your build");
        QString sentStr = _d->bytesString(bytesSent, false);
        if (bytesTotal == -1) {
            QQmlProperty::write(_d->progressPage, "mbText", QString("%1/?").arg(sentStr));
            QQmlProperty::write(_d->progressPage, "progressbarValue", 0.6);
            QQmlProperty::write(_d->progressPage, "progressbarValue2", 0.75);
        } else if (bytesTotal == 0) {
            Q_UNUSED(bytesTotal);
        } else {
            QString totalStr = _d->bytesString(bytesTotal, true);
            QQmlProperty::write(_d->progressPage, "mbText", QString("%1/%2").arg(sentStr).arg(totalStr));
            QQmlProperty::write(_d->progressPage, "progressbarValue", (qreal)bytesSent/bytesTotal);
            QQmlProperty::write(_d->progressPage, "progressbarValue2", 0.3 + ((qreal)bytesSent/bytesTotal) * 0.7);
        }

        if (bytesSent > 0 && bytesSent != bytesTotal)  {
            if (_d->elapsedTimer.isValid()) {
                _d->times.append(_d->elapsedTimer.elapsed());
            }
            _d->bytes.append(bytesSent);
            _d->elapsedTimer.restart();

            if (_d->bytes.size() > 5) {
                _d->bytes.removeFirst();
                _d->times.removeFirst();
            }

            qreal collection = 0;
            for (int i = 0; i < _d->times.size(); i++) {
                collection += (_d->bytes[i + 1] - _d->bytes[i])/((qreal)_d->times[i]);
            }
            if (collection > 0) {
                QQmlProperty::write(_d->progressPage, "speedText", _d->bytesString((collection / _d->times.size()) * 1000, true) + "/sec");
            } else {
                QQmlProperty::write(_d->progressPage, "speedText", _d->bytesString(bytesSent, true) + "/sec");
            }
        } else {
            _d->elapsedTimer.invalidate();
            _d->times.clear();
            _d->bytes.clear();
        }
    });
    connect(_d->reply, (void (QNetworkReply::*)(qint64, qint64))
      &QNetworkReply::uploadProgress, this, [=](qint64 bytesSent, qint64 bytesTotal) {
        QQmlProperty::write(_d->progressPage, "informativeText", "Uploading your project");
        QString sentStr = _d->bytesString(bytesSent, false);
        if (bytesTotal == -1) {
            QQmlProperty::write(_d->progressPage, "mbText", QString("%1/?").arg(sentStr));
            QQmlProperty::write(_d->progressPage, "progressbarValue", 0.6);
            QQmlProperty::write(_d->progressPage, "progressbarValue2", 0.3);
        } else if (bytesTotal == 0) {
            Q_UNUSED(bytesTotal);
        } else {
            QString totalStr = _d->bytesString(bytesTotal, true);
            QQmlProperty::write(_d->progressPage, "mbText", QString("%1/%2").arg(sentStr).arg(totalStr));
            QQmlProperty::write(_d->progressPage, "progressbarValue", (qreal)bytesSent/bytesTotal);
            QQmlProperty::write(_d->progressPage, "progressbarValue2", (((qreal)bytesSent / bytesTotal) / 2) * 0.3);
        }

        if (bytesSent > 0 && bytesSent != bytesTotal)  {
            if (_d->elapsedTimer.isValid()) {
                _d->times.append(_d->elapsedTimer.elapsed());
            }
            _d->bytes.append(bytesSent);
            _d->elapsedTimer.restart();

            if (_d->bytes.size() > 5) {
                _d->bytes.removeFirst();
                _d->times.removeFirst();
            }

            qreal collection = 0;
            for (int i = 0; i < _d->times.size(); i++) {
                collection += (_d->bytes[i + 1] - _d->bytes[i])/((qreal)_d->times[i]);
            }
            if (collection > 0) {
                QQmlProperty::write(_d->progressPage, "speedText", _d->bytesString((collection / _d->times.size()) * 1000, true) + "/sec");
            } else {
                QQmlProperty::write(_d->progressPage, "speedText", _d->bytesString(bytesSent, true) + "/sec");
            }
        } else {
            _d->elapsedTimer.invalidate();
            _d->times.clear();
            _d->bytes.clear();
        }
        if (bytesSent == bytesTotal) {
            QQmlProperty::write(_d->progressPage, "informativeText", "Your build getting prepared");
            _d->elapsedTimer.invalidate();
            _d->times.clear();
            _d->bytes.clear();
        }
    });
    connect(_d->reply, (void (QNetworkReply::*)(QList<QSslError>))
      &QNetworkReply::sslErrors, this, [=] { _d->reply->ignoreSslErrors(); });
    connect(_d->reply, (void (QNetworkReply::*)(QNetworkReply::NetworkError))
      &QNetworkReply::error, this, [=](QNetworkReply::NetworkError e)
    {
        if (e != QNetworkReply::OperationCanceledError) {
            QQmlProperty::write(_d->swipeView, "currentIndex", 0);
            QMetaEnum metaEnum = QMetaEnum::fromType<QNetworkReply::NetworkError>();

            if (e == QNetworkReply::AuthenticationRequiredError) {
                QQmlProperty::write(_d->toast, "text.text", "<p><b>Authentication Error</b></p>"
                                    "<p>Your email address or password is wrong. Please contact to support.</p>");
                QQmlProperty::write(_d->toast, "base.width", qFloor(fit::fx(330)));
                QQmlProperty::write(_d->toast, "base.height", qFloor(fit::fx(95)));
                QQmlProperty::write(_d->toast, "duration", 10000);
            } else if (e == QNetworkReply::ContentOperationNotPermittedError) {
                QQmlProperty::write(_d->toast, "text.text", "<p><b>Access Denied</b></p>"
                                    "<p>Your account doesn't have any permission to use this feature, please upgrade your subscription plan.</p>");
                QQmlProperty::write(_d->toast, "base.width", qFloor(fit::fx(330)));
                QQmlProperty::write(_d->toast, "base.height", qFloor(fit::fx(110)));
                QQmlProperty::write(_d->toast, "duration", 10000);
            }  else if (e == QNetworkReply::ServiceUnavailableError ||
                        e == QNetworkReply::UnknownServerError ||
                        e == QNetworkReply::InternalServerError ) {
                QQmlProperty::write(_d->toast, "text.text", "<p><b>" + QString(metaEnum.valueToKey(e)) + "</b></p>" +
                                    "<p>Server error has occurred, please try again later.</p>");
                QQmlProperty::write(_d->toast, "base.width", qFloor(fit::fx(330)));
                QQmlProperty::write(_d->toast, "base.height",qFloor(fit::fx(95)));
                QQmlProperty::write(_d->toast, "duration", 10000);
            } else {
                QQmlProperty::write(_d->toast, "text.text", "<p><b>" + QString(metaEnum.valueToKey(e)) + "</b></p>" +
                                    "<p>Either check your internet connection or contact to support.</p>");
                QQmlProperty::write(_d->toast, "base.width", qFloor(fit::fx(330)));
                QQmlProperty::write(_d->toast, "base.height", qFloor(fit::fx(95)));
                QQmlProperty::write(_d->toast, "duration", 10000);
            }
            QMetaObject::invokeMethod(_d->toast, "show");
            _d->exitButton.show();
        }
    });
}

void BuildsScreen::handleBtnOkClicked()
{
    _d->elapsedTimer.invalidate();
    _d->times.clear();
    _d->bytes.clear();
    QQmlProperty::write(_d->swipeView, "currentIndex", 0);
    _d->exitButton.show();
}

void BuildsScreen::handleBtnCancelClicked()
{
    if (_d->reply) _d->reply->abort();
    _d->elapsedTimer.invalidate();
    _d->times.clear();
    _d->bytes.clear();
    QQmlProperty::write(_d->swipeView, "currentIndex", 0);
    _d->exitButton.show();
}
