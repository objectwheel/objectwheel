#include <hashfactory.h>
#include <QRandomGenerator>
#include <QDateTime>
#include <QCryptographicHash>

namespace HashFactory {

QString generate()
{
    QByteArray data;
    data.append(QString::number(QDateTime::currentMSecsSinceEpoch()));
    data.append(QString::number(QRandomGenerator::global()->generate()));
    const QByteArray& hex = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
    return hex.left(6) + hex.right(6);
}

}