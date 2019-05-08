#include <hashfactory.h>
#include <QRandomGenerator>
#include <QDateTime>
#include <QCryptographicHash>

namespace HashFactory {

QByteArray generate()
{
    QByteArray data;
    data.append(QByteArray::number(QDateTime::currentMSecsSinceEpoch()));
    data.append(QByteArray::number(QRandomGenerator::global()->generate()));
    const QByteArray& hex = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
    return hex.left(6) + hex.right(6);
}

QByteArray generateSalt()
{
    QByteArray data;
    data.append(QByteArray::number(QDateTime::currentMSecsSinceEpoch()));
    data.append(QByteArray::number(QRandomGenerator::global()->generate()));
    data.append(QByteArray::number(QRandomGenerator::global()->generate64()));
    return QCryptographicHash::hash(data, QCryptographicHash::Sha3_512).toHex();
}

} // HashFactory