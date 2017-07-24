#ifndef CONTROLTRANSACTION_H
#define CONTROLTRANSACTION_H

#include <QObject>

class Control;

class ControlTransaction : public QObject
{
        Q_OBJECT

    public:
        explicit ControlTransaction(Control* watched, QObject *parent = 0);

    public slots:
        void flushGeometryChange();
        void flushParentChange();
        void flushZChange();
        void flushIdChange(const QString& prevId);

    private:
        Control* _watched;
};

#endif // CONTROLTRANSACTION_H
