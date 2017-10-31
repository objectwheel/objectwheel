#include <parsercontroller.h>
#include <parserworker.h>

#define TRANSACTION_INTERVAL 500

ParserWorker* ParserController::_parserWorker = nullptr;
QThread* ParserController::_workerThread = nullptr;
QList<Transaction> ParserController::_transactionList;
QTimer* ParserController::_transactionTimer = nullptr;
bool ParserController::_running = false;

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
    connect(_parserWorker, SIGNAL(done()), SLOT(checkRunning()));
    _workerThread->start();
}

ParserController::~ParserController()
{
    _workerThread->quit();
    _workerThread->wait();
}

void ParserController::setVariantProperty(const QString& url, const QString& property, const QVariant& value)
{
    Transaction transaction;
    transaction.url = url;
    transaction.property = property;
    transaction.value = value;
    transaction.type = VariantProperty;

    if (_transactionList.contains(transaction))
        _transactionList.removeAll(transaction);
    _transactionList.append(transaction);

    _transactionTimer->start();
}

void ParserController::removeVariantProperty(const QString& url, const QString& property)
{
    Transaction transaction;
    transaction.url = url;
    transaction.property = property;
    transaction.type = RemoveVariantProperty;

    if (_transactionList.contains(transaction))
        _transactionList.removeAll(transaction);
    _transactionList.append(transaction);

    _transactionTimer->start();
}

bool ParserController::running()
{
    return _transactionList.size() > 0;
}

void ParserController::processWaitingTransactions()
{
    _transactionTimer->stop();

    if (_transactionList.isEmpty())
        return;

    _running = true;

    auto transaction = _transactionList.first();
    switch (transaction.type) {
        case VariantProperty:
            QMetaObject::invokeMethod(_parserWorker, "setVariantProperty",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, transaction.url),
                                      Q_ARG(QString, transaction.property),
                                      Q_ARG(QVariant, transaction.value));
            break;
        case RemoveVariantProperty:
            QMetaObject::invokeMethod(_parserWorker, "removeVariantProperty",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, transaction.url),
                                      Q_ARG(QString, transaction.property));
            break;
        default:
            break;
    }

    if (_transactionList.size() > 1)
        _transactionTimer->start();

    _transactionList.removeOne(transaction);
    emit runningChanged(_running);
}

void ParserController::checkRunning()
{
    if (_transactionList.isEmpty()) {
        _running = false;
        emit runningChanged(_running);
    }
}
