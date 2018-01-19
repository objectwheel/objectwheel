#include <internetaccess.h>
#include <delayer.h>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#define TIMEOUT 5000

bool InternetAccess::available()
{
    QNetworkAccessManager nam;
    QNetworkRequest req(QUrl("http://google.com"));
    QNetworkReply* reply = nam.get(req);
    Delayer::delay(std::bind(&QNetworkReply::isRunning, reply), false, TIMEOUT);
    reply->deleteLater();
    return reply->bytesAvailable();
}
