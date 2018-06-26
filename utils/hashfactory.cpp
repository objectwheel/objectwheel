#include <hashfactory.h>

#include <QRandomGenerator>
#include <QDateTime>
#include <QCryptographicHash>

QString HashFactory::generate()
{
    typedef QCryptographicHash Hasher;

    const quint32 a = QRandomGenerator::global()->generate();
    const quint32 b = QRandomGenerator::global()->generate();
    const quint32 c = QRandomGenerator::global()->generate();
    const quint32 d = QDateTime::currentMSecsSinceEpoch();

    QByteArray data;
    data.insert(0, QString::number(a));
    data.insert(0, QString::number(b));
    data.insert(0, QString::number(c));
    data.insert(0, QString::number(d));

    const QByteArray& hex = Hasher::hash(data, Hasher::Sha256).toHex();

    return hex.left(6) + hex.right(6);
}
