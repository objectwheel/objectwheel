#ifndef SEARCHBOX_H
#define SEARCHBOX_H

#include <QWidget>

class SearchBox : public QWidget
{
        Q_OBJECT
    public:
        explicit SearchBox(QWidget *parent = nullptr);

    public slots:
        void reset();
};

#endif // SEARCHBOX_H