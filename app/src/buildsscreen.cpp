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
#include <QList>
#include <QElapsedTimer>
#include <QString>
#include <QApplication>

#define URL QString("https://127.0.0.1/api/v1/build/")

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
        QNetworkReply* reply;
        QString bytesString(const qint64 size, bool withExt);
        QString determineBuildExtension(const QString label);
};

BuildsScreenPrivate* BuildsScreen::m_d = nullptr;

BuildsScreenPrivate::BuildsScreenPrivate(BuildsScreen* w)
    : parent(w)
    , manager(new QNetworkAccessManager(parent))
{
    parent->rootContext()->setContextProperty("dpi", Fit::ratio());
    parent->setSource(QUrl("qrc:/resources/qmls/buildsScreen/main.qml"));
    parent->setResizeMode(QQuickWidget::SizeRootObjectToView);

    buildPage = (QQuickItem*)QQmlProperty::read(parent->rootObject(), "buildPage", parent->engine()).value<QObject*>();
    progressPage = (QQuickItem*)QQmlProperty::read(parent->rootObject(), "progressPage", parent->engine()).value<QObject*>();
    toast = (QQuickItem*)QQmlProperty::read(parent->rootObject(), "toast",parent-> engine()).value<QObject*>();
    swipeView = (QQuickItem*)QQmlProperty::read(parent->rootObject(), "swipeView", parent->engine()).value<QObject*>();

    QObject::connect(buildPage, SIGNAL(btnBuildClicked()), parent, SLOT(handleBuildButtonClicked()));
    QObject::connect(progressPage, SIGNAL(btnOkClicked()), parent, SLOT(handleBtnOkClicked()));
    QObject::connect(progressPage, SIGNAL(btnCancelClicked()), parent, SLOT(handleBtnCancelClicked()));
    QObject::connect(qApp, SIGNAL(aboutToQuit()), parent, SLOT(handleBtnCancelClicked()));
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
        return QString(".dmg");
    } else {
        return QString();
    }
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

    QNetworkRequest request(QUrl(URL + buildLabel));
    request.setRawHeader("Content-Type","multipart/form-data; boundary=-----------------------------7d935033608e2");
    request.setHeader(QNetworkRequest::ContentLengthHeader,body.size());
    m_d->reply = m_d->manager->post(request, body);

    QQmlProperty::write(m_d->swipeView, "currentIndex", 1);
    QQmlProperty::write(m_d->progressPage, "informativeText", "Establishing connection");
    QQmlProperty::write(m_d->progressPage, "mbText", "-");
    QQmlProperty::write(m_d->progressPage, "speedText", "-");
    QQmlProperty::write(m_d->progressPage, "progressbarValue", 0.0);
    QMetaObject::invokeMethod(m_d->progressPage, "startWaitEffect");
    QMetaObject::invokeMethod(m_d->progressPage, "showBtnCancel");

    connect(m_d->reply, &QNetworkReply::finished, [=] {
        if (!m_d->reply->isOpen() || !m_d->reply->isReadable() || m_d->reply->error() != QNetworkReply::NoError) {
            m_d->elapsedTimer.invalidate();
            m_d->times.clear();
            m_d->bytes.clear();
            m_d->reply->deleteLater();
            return;
        }

        QQmlProperty::write(m_d->progressPage, "btnCancelEnabled", false);
        auto response = m_d->reply->readAll();
        QString name = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0] + separator() + buildLabel;

        QString nname;
        int count = 0;
        do {
            if (count == 0) {
                nname = name + m_d->determineBuildExtension(buildLabel);
            } else {
                nname = name + "_" + QString::number(count) + m_d->determineBuildExtension(buildLabel);
            }
            count++;
        } while (exists(nname));

        wrfile(nname, response);

        QQmlProperty::write(m_d->progressPage, "informativeText", "Done");
        QMetaObject::invokeMethod(m_d->progressPage, "stopWaitEffect");
        QMetaObject::invokeMethod(m_d->progressPage, "showBtnOk");
        QQmlProperty::write(m_d->progressPage, "btnCancelEnabled", true);

        m_d->elapsedTimer.invalidate();
        m_d->times.clear();
        m_d->bytes.clear();
        m_d->reply->deleteLater();
    });
    connect(m_d->reply, (void (QNetworkReply::*)(qint64, qint64))&QNetworkReply::downloadProgress, [=](qint64 bytesSent, qint64 bytesTotal) {
        QQmlProperty::write(m_d->progressPage, "informativeText", "Downloading your build");
        QString sentStr = m_d->bytesString(bytesSent, false);
        if (bytesTotal == -1) {
            QQmlProperty::write(m_d->progressPage, "mbText", QString("%1/?").arg(sentStr));
            QQmlProperty::write(m_d->progressPage, "progressbarValue", 0.6);
            QQmlProperty::write(m_d->progressPage, "progressbarValue2", 0.75);
        } else if (bytesTotal == 0) {
            Q_UNUSED(bytesTotal);
        } else {
            QString totalStr = m_d->bytesString(bytesTotal, true);
            QQmlProperty::write(m_d->progressPage, "mbText", QString("%1/%2").arg(sentStr).arg(totalStr));
            QQmlProperty::write(m_d->progressPage, "progressbarValue", (qreal)bytesSent/bytesTotal);
            QQmlProperty::write(m_d->progressPage, "progressbarValue2", 0.3 + ((qreal)bytesSent/bytesTotal) * 0.7);
        }

        if (bytesSent > 0 && bytesSent != bytesTotal)  {
            if (m_d->elapsedTimer.isValid()) {
                m_d->times.append(m_d->elapsedTimer.elapsed());
            }
            m_d->bytes.append(bytesSent);
            m_d->elapsedTimer.restart();

            if (m_d->bytes.size() > 5) {
                m_d->bytes.removeFirst();
                m_d->times.removeFirst();
            }

            qreal collection = 0;
            for (int i = 0; i < m_d->times.size(); i++) {
                collection += (m_d->bytes[i + 1] - m_d->bytes[i])/((qreal)m_d->times[i]);
            }
            if (collection > 0) {
                QQmlProperty::write(m_d->progressPage, "speedText", m_d->bytesString((collection / m_d->times.size()) * 1000, true) + "/sec");
            } else {
                QQmlProperty::write(m_d->progressPage, "speedText", m_d->bytesString(bytesSent, true) + "/sec");
            }
        } else {
            m_d->elapsedTimer.invalidate();
            m_d->times.clear();
            m_d->bytes.clear();
        }
    });
    connect(m_d->reply, (void (QNetworkReply::*)(qint64, qint64))&QNetworkReply::uploadProgress, [=](qint64 bytesSent, qint64 bytesTotal) {
        QQmlProperty::write(m_d->progressPage, "informativeText", "Uploading your project");
        QString sentStr = m_d->bytesString(bytesSent, false);
        if (bytesTotal == -1) {
            QQmlProperty::write(m_d->progressPage, "mbText", QString("%1/?").arg(sentStr));
            QQmlProperty::write(m_d->progressPage, "progressbarValue", 0.6);
            QQmlProperty::write(m_d->progressPage, "progressbarValue2", 0.3);
        } else if (bytesTotal == 0) {
            Q_UNUSED(bytesTotal);
        } else {
            QString totalStr = m_d->bytesString(bytesTotal, true);
            QQmlProperty::write(m_d->progressPage, "mbText", QString("%1/%2").arg(sentStr).arg(totalStr));
            QQmlProperty::write(m_d->progressPage, "progressbarValue", (qreal)bytesSent/bytesTotal);
            QQmlProperty::write(m_d->progressPage, "progressbarValue2", (((qreal)bytesSent / bytesTotal) / 2) * 0.3);
        }

        if (bytesSent > 0 && bytesSent != bytesTotal)  {
            if (m_d->elapsedTimer.isValid()) {
                m_d->times.append(m_d->elapsedTimer.elapsed());
            }
            m_d->bytes.append(bytesSent);
            m_d->elapsedTimer.restart();

            if (m_d->bytes.size() > 5) {
                m_d->bytes.removeFirst();
                m_d->times.removeFirst();
            }

            qreal collection = 0;
            for (int i = 0; i < m_d->times.size(); i++) {
                collection += (m_d->bytes[i + 1] - m_d->bytes[i])/((qreal)m_d->times[i]);
            }
            if (collection > 0) {
                QQmlProperty::write(m_d->progressPage, "speedText", m_d->bytesString((collection / m_d->times.size()) * 1000, true) + "/sec");
            } else {
                QQmlProperty::write(m_d->progressPage, "speedText", m_d->bytesString(bytesSent, true) + "/sec");
            }
        } else {
            m_d->elapsedTimer.invalidate();
            m_d->times.clear();
            m_d->bytes.clear();
        }
        if (bytesSent == bytesTotal) {
            QQmlProperty::write(m_d->progressPage, "informativeText", "Your build getting prepared");
            m_d->elapsedTimer.invalidate();
            m_d->times.clear();
            m_d->bytes.clear();
        }
    });
    connect(m_d->reply, (void (QNetworkReply::*)(QList<QSslError>))&QNetworkReply::sslErrors, [=] { m_d->reply->ignoreSslErrors(); });
    connect(m_d->reply, (void (QNetworkReply::*)(QNetworkReply::NetworkError))&QNetworkReply::error, [=](QNetworkReply::NetworkError e)
    {
        if (e != QNetworkReply::OperationCanceledError) {
            QQmlProperty::write(m_d->swipeView, "currentIndex", 0);
            QMetaEnum metaEnum = QMetaEnum::fromType<QNetworkReply::NetworkError>();
            QQmlProperty::write(m_d->toast, "text.text", "<p><b>" + QString(metaEnum.valueToKey(e)) + "</b></p>" +
                                "<p>Either check your internet connection or contact to support.</p>");
            QQmlProperty::write(m_d->toast, "base.width", 330);
            QQmlProperty::write(m_d->toast, "base.height", 95);
            QQmlProperty::write(m_d->toast, "duration", 10000);
            QMetaObject::invokeMethod(m_d->toast, "show");
        }
    });
}

void BuildsScreen::handleBtnOkClicked()
{
    m_d->elapsedTimer.invalidate();
    m_d->times.clear();
    m_d->bytes.clear();
    QQmlProperty::write(m_d->swipeView, "currentIndex", 0);
}

void BuildsScreen::handleBtnCancelClicked()
{
    m_d->reply->abort();
    m_d->elapsedTimer.invalidate();
    m_d->times.clear();
    m_d->bytes.clear();
    QQmlProperty::write(m_d->swipeView, "currentIndex", 0);
}
