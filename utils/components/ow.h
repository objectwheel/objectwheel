#ifndef OW_H
#define OW_H

#include <QObject>

class Ow : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Ow)

public:
    explicit Ow(QObject* parent = nullptr);

public slots:
    QString toLocalFile(const QUrl& url);
};

#endif // OW_H
