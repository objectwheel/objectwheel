#ifndef PARSERCONTROLLER_H
#define PARSERCONTROLLER_H

#include <QObject>
#include <QThread>
#include <QList>
#include <QTimer>

class ParserWorker;


enum TransactionType {
    VariantProperty,
    RemoveVariantProperty
};

struct Transaction {
        QString url;
        QString property;
        QVariant value;
        TransactionType type;

        bool operator== (const Transaction& x) const {
            return (x.url == this->url &&
                    x.property == this->property &&
                    x.type == this->type);
        }
};


class ParserController : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(ParserController)

    public:
        explicit ParserController(QObject *parent = 0);
        ~ParserController();
        static void setVariantProperty(const QString& url, const QString& property, const QVariant& value);
        static void removeVariantProperty(const QString& url, const QString& property);
        static bool running();
        static void removeTransactionsFor(const QString& url);

    private slots:
        void processWaitingTransactions();
        void checkRunning();

    signals:
        void runningChanged(bool running);

    private:
        static QThread* _workerThread;
        static ParserWorker* _parserWorker;
        static QTimer* _transactionTimer;
        static QList<Transaction> _transactionList;
        static bool _running;
};

#endif // PARSERCONTROLLER_H
