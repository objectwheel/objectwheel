#ifndef OW_H
#define OW_H

#include <QObject>

class Objectwheel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Objectwheel)

public:
    explicit Objectwheel(QObject* parent = nullptr);

public slots:
    QString toLocalFile(const QUrl& url);
};

#endif // OW_H
