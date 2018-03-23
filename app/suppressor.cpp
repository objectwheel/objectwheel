#include <suppressor.h>
#include <QTimer>
#include <QMap>
#include <QPair>

void Suppressor::suppress(int msec, const QString& key, const std::function<void()>& function)
{
    static QMap<QString, QPair<QTimer*, std::function<void()>>> map;
    if (map.contains(key)) {
        map[key].second = function;
        map.value(key).first->start();
        return;
    } else {
        auto timer = new QTimer;
        timer->setInterval(msec);
        QObject::connect(timer, &QTimer::timeout, [=] {
            timer->deleteLater();
            map.value(key).second();
            map.remove(key);
        });
        map[key].first = timer;
        map[key].second = function;
        timer->start();
    }
}
