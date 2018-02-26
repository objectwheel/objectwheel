#include <downloadwidget.h>
#include <fit.h>
#include <projectbackend.h>
#include <userbackend.h>
#include <filemanager.h>
#include <projectbackend.h>
#include <userbackend.h>
#include <saveutils.h>
#include <savebackend.h>
#include <projectswidget.h>
#include <dirlocker.h>
#include <zipper.h>
#include <flatbutton.h>
#include <mainwindow.h>
#include <build.h>

#include <QtWidgets>
#include <QtQuick>
#include <QtQml>

#define URL (QString("https://api.objectwheel.com") + "/api/v1/build/")

class DownloadWidgetPrivate {
    public:
        DownloadWidgetPrivate(DownloadWidget* w);
        QList<qint64> bytes;
        QList<qint64> times;
        QElapsedTimer elapsedTimer;
        DownloadWidget* parent;
        QQuickItem* progressPage;
        QNetworkAccessManager* manager;
        QPointer<QNetworkReply> reply;
        QString bytesString(const qint64 size, bool withExt);
        QString determineBuildExtension(const QString label);
};

DownloadWidgetPrivate* DownloadWidget::_d = nullptr;

DownloadWidgetPrivate::DownloadWidgetPrivate(DownloadWidget* w)
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

    progressPage = (QQuickItem*)QQmlProperty::read(parent->rootObject(),
      "progressPage", parent->engine()).value<QObject*>();

    QObject::connect(progressPage, SIGNAL(btnOkClicked()),
      parent, SLOT(handleBtnOkClicked()));
    QObject::connect(progressPage, SIGNAL(btnCancelClicked()),
      parent, SLOT(handleBtnCancelClicked()));
    QObject::connect(qApp, SIGNAL(aboutToQuit()), parent,
      SLOT(handleBtnCancelClicked()));
}

QString DownloadWidgetPrivate::bytesString(const qint64 size, bool withExt)
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

QString DownloadWidgetPrivate::determineBuildExtension(const QString label)
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

DownloadWidget::~DownloadWidget()
{
    delete _d;
}

DownloadWidget* DownloadWidget::instance()
{
    return _d->parent;
}

DownloadWidget::DownloadWidget(QWidget *parent) : QQuickWidget(parent)
{
    if (_d) return;
    _d = new DownloadWidgetPrivate(this);
}

void DownloadWidget::resizeEvent(QResizeEvent* event)
{
    QQuickWidget::resizeEvent(event);
    emit resized();
}

void DownloadWidget::download(OTargets::Targets target)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<OTargets::Targets>();
    auto buildLabel = QString(metaEnum.valueToKey(target));

    auto pdir = ProjectBackend::instance()->dir();
    if (pdir.isEmpty())
        return;

    QTemporaryDir tdir, tdir2;
    if (!tdir.isValid() || !tdir2.isValid())
        qFatal("Error");

    if (!cp(pdir + separator() + DIR_OWDB, tdir.path()) ||
        !cp(pdir + separator() + DIR_BUILD, tdir.path()))
        qFatal("Error");


    Zipper::compressDir(tdir.path(), tdir2.path() + separator() + "project.zip");
    QByteArray data = rdfile(tdir2.path() + separator() + "project.zip");

    QByteArray boundary = "-----------------------------7d935033608e2";
    QByteArray body = "\r\n--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"project\"; filename=\"project.zip\"\r\n";
    body += "Content-Type: application/octet-stream\r\n\r\n";
    body += data;
    body += "\r\n--" + boundary + "--\r\n";

    QString url = URL;
    if (target == OTargets::android_armeabi_v7a ||
        target == OTargets::android_x86)
        url += "android";

    QNetworkRequest request(QUrl::fromUserInput(url));
    request.setRawHeader("Content-Type","multipart/form-data; boundary=-----------------------------7d935033608e2");
    request.setRawHeader("token", QByteArray().insert(0, QString("{\"value\" : \"%1\"}").arg(UserBackend::instance()->token())));
    request.setRawHeader("x86", QByteArray().insert(0, QString("{\"value\" : %1}").arg(target == OTargets::android_x86 ? "true" : "false")));
    request.setHeader(QNetworkRequest::ContentLengthHeader, body.size());
    _d->reply = _d->manager->post(request, body);

    QQmlProperty::write(_d->progressPage, "informativeText", "Establishing connection");
    QQmlProperty::write(_d->progressPage, "mbText", "-");
    QQmlProperty::write(_d->progressPage, "speedText", "-");
    QQmlProperty::write(_d->progressPage, "progressbarValue", 0.0);
    QMetaObject::invokeMethod(_d->progressPage, "startWaitEffect");
    QMetaObject::invokeMethod(_d->progressPage, "showBtnCancel");

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
            QMetaEnum metaEnum = QMetaEnum::fromType<QNetworkReply::NetworkError>();
            if (e == QNetworkReply::AuthenticationRequiredError) {
                QMessageBox::warning(this, "Authentication Error",
                  "Your email address or password is wrong. Please contact to support.");
            } else if (e == QNetworkReply::ContentOperationNotPermittedError) {
                QMessageBox::warning(this, "Access Denied",
                  "Your account doesn't have any permission to use this feature, please upgrade your subscription plan.");
            }  else if (e == QNetworkReply::ServiceUnavailableError ||
                        e == QNetworkReply::UnknownServerError ||
                        e == QNetworkReply::InternalServerError ) {
                QMessageBox::warning(this, QString(metaEnum.valueToKey(e)),
                  "Server error has occurred, please try again later.");
            } else {
                QMessageBox::warning(this, QString(metaEnum.valueToKey(e)),
                  "Either check your internet connection or contact to support.");
            }
            emit done();
        }
    });
}

void DownloadWidget::handleBtnOkClicked()
{
    _d->elapsedTimer.invalidate();
    _d->times.clear();
    _d->bytes.clear();
    emit done();
}

void DownloadWidget::handleBtnCancelClicked()
{
    if (_d->reply) _d->reply->abort();
    _d->elapsedTimer.invalidate();
    _d->times.clear();
    _d->bytes.clear();
    emit done();
}
