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
        void handleGeometryChange();
        void handleParentChange();

    private:
        Control* _watched;
};

#endif // CONTROLTRANSACTION_H
