#ifndef SAVETRANSACTION_H
#define SAVETRANSACTION_H

#include <QObject>

class Control;

class SaveTransaction : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SaveTransaction)

public:
    static SaveTransaction* instance();
    void disable();
    void enable();

public slots:
    void processZ(Control*);
    void processParent(Control*);
    void processGeometry(Control*);

private:
    SaveTransaction();
};

#endif // SAVETRANSACTION_H
