#include <suppressor.h>
#include <QTimer>
#include <QMap>

using namespace std;

void Suppressor::suppress(int msec, const QString& key, const function<void()>& function)
{
    static QMap<QString, QTimer*> map;
    if (map.contains(key)) {
        map.value(key)->start();
        return;
    } else {
        auto timer = new QTimer;
        timer->setInterval(msec);
        QObject::connect(timer, &QTimer::timeout, [=] {
            map.remove(key);
            timer->deleteLater();
            function();
        });
        map[key] = timer;
    }
}
