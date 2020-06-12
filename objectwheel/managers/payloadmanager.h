#ifndef PAYLOADMANAGER_H
#define PAYLOADMANAGER_H

#include <QObject>

class PayloadManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PayloadManager)

public:
    explicit PayloadManager(QObject* parent = nullptr);

};

#endif // PAYLOADMANAGER_H
