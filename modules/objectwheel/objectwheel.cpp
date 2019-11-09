#include <objectwheel.h>
#include <QUrl>

Objectwheel::Objectwheel(QObject* parent) : QObject(parent)
{
}

QString Objectwheel::toLocalFile(const QUrl& url)
{
    return url.toLocalFile();
}
