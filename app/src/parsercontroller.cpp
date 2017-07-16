#include <parsercontroller.h>
#include <parserworker.h>

#define TRANSACTION_INTERVAL 500

ParserWorker* ParserController::_parserWorker = nullptr;
QThread* ParserController::_workerThread = nullptr;
QList<ParserController::Transaction> ParserController::_transactionList;
QTimer* ParserController::_transactionTimer = nullptr;

ParserController::ParserController(QObject *parent)
    : QObject(parent)
{
    if (_parserWorker)
        return;

    _transactionTimer = new QTimer(this);
    _transactionTimer->setInterval(TRANSACTION_INTERVAL);
    connect(_transactionTimer, SIGNAL(timeout()), SLOT(processWaitingTransactions()));

    _workerThread = new QThread(this);
    _parserWorker = new ParserWorker;
    _parserWorker->moveToThread(_workerThread);
    _workerThread->start();
}

void ParserController::setVariantProperty(const QString& fileName, const QString& property, const QVariant& value)
{
    Transaction transaction;
    transaction.fileName = fileName;
    transaction.property = property;
    transaction.value = value;
    transaction.type = VariantPropertyTransaction;

    if (_transactionList.contains(transaction))
        _transactionList.removeAll(transaction);
    _transactionList.append(transaction);

    _transactionTimer->start();
}

void ParserController::processWaitingTransactions()
{
    _transactionTimer->stop();

    if (_transactionList.isEmpty())
        return;
    else if (_transactionList.size() > 1)
        _transactionTimer->start();

    auto transaction = _transactionList.first();
    switch (transaction.type) {
        case VariantPropertyTransaction:
            QMetaObject::invokeMethod(_parserWorker, "setVariantProperty",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, transaction.fileName),
                                      Q_ARG(QString, transaction.property),
                                      Q_ARG(QVariant, transaction.value));
            break;
        default:
            break;
    }
    _transactionList.removeOne(transaction);
}
