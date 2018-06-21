#ifndef CONTROLTRANSACTIONMANAGER_H
#define CONTROLTRANSACTIONMANAGER_H

#include <QObject>

class Control;

class ControlTransactionManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlTransactionManager)

    friend class ApplicationCore;

private slots:
    void processZ(Control*);
    void processParent(Control*);
    void processGeometry(Control*);

private:
    explicit ControlTransactionManager(QObject* parent = nullptr);
};

#endif // CONTROLTRANSACTIONMANAGER_H
