#include <ow.h>
#include <QUrl>

Ow::Ow(QObject* parent) : QObject(parent)
{
}

QString Ow::toLocalFile(const QUrl& url)
{
    return url.toLocalFile();
}
