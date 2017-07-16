#ifndef PARSERCONTROLLER_H
#define PARSERCONTROLLER_H

#include <QObject>
#include <QThread>
#include <QList>
#include <QTimer>

class ParserWorker;

class ParserController : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(ParserController)

    public:
        enum TransactionType {
            VariantPropertyTransaction
        };

        struct Transaction {
                QString fileName;
                QString property;
                QVariant value;
                TransactionType type;

                bool operator== (const Transaction& x) {
                    return (x.fileName == this->fileName &&
                            x.property == this->property &&
                            x.type == this->type);
                }
        };

    public:
        explicit ParserController(QObject *parent = 0);
        static void setVariantProperty(const QString& fileName, const QString& property, const QVariant& value);

    private slots:
        void processWaitingTransactions();

    private:
        static QThread* _workerThread;
        static ParserWorker* _parserWorker;
        static QTimer* _transactionTimer;
        static QList<Transaction> _transactionList;
};

#endif // PARSERCONTROLLER_H
