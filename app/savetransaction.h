#ifndef SAVETRANSACTION_H
#define SAVETRANSACTION_H

#include <QObject>

class Control;

class SaveTransaction final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SaveTransaction)

    friend class BackendManager;

private slots:
    void processZ(Control*);
    void processParent(Control*);
    void processGeometry(Control*);

private:
    explicit SaveTransaction(QObject* parent = nullptr);
};

#endif // SAVETRANSACTION_H
