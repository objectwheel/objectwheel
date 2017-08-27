#ifndef CONTROLTRANSACTION_H
#define CONTROLTRANSACTION_H

#include <QObject>

class Control;

class ControlTransaction : public QObject
{
        Q_OBJECT

    public:
        explicit ControlTransaction(Control* watched, QObject *parent = 0);

        void setGeometryTransactionsEnabled(bool value);
        void setParentTransactionsEnabled(bool value);
        void setZTransactionsEnabled(bool value);
        void setIdTransactionsEnabled(bool value);
        void setTransactionsEnabled(bool value);

    public slots:
        void flushGeometryChange();
        void flushParentChange();
        void flushZChange();
        void flushIdChange(const QString& prevId);

    private:
        Control* _watched;
        bool _transactionsEnabled;
        bool _geometryTransactionsEnabled;
        bool _parentTransactionsEnabled;
        bool _zTransactionsEnabled;
        bool _idTransactionsEnabled;

};

#endif // CONTROLTRANSACTION_H
