#ifndef CONSOLEBOX_H
#define CONSOLEBOX_H

#include <QWidget>

class ConsoleBox : public QWidget
{
        Q_OBJECT
    public:
        explicit ConsoleBox(QWidget *parent = nullptr);

    public slots:
        void clear();
};

#endif // CONSOLEBOX_H