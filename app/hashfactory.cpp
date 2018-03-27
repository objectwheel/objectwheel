#include <hashfactory.h>
#include <QRandomGenerator>
#include <QDateTime>
#include <QCryptographicHash>

typedef QCryptographicHash Hasher;

QString HashFactory::generate()
{
    auto a = QRandomGenerator::global()->generate();
    auto b = QRandomGenerator::global()->generate();
    auto c = QRandomGenerator::global()->generate();
    auto d = QDateTime::currentMSecsSinceEpoch();

    QByteArray data;
    data.insert(0, QString::number(a));
    data.insert(0, QString::number(b));
    data.insert(0, QString::number(c));
    data.insert(0, QString::number(d));

    const auto& hex = Hasher::hash(data, Hasher::Sha256).toHex();

    return hex.left(6) + hex.right(6);
}
