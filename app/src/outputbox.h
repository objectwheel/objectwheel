#ifndef OUTPUTBOX_H
#define OUTPUTBOX_H

#include <QWidget>

class QSplitter;
class OutputBoxPrivate;

class OutputBox : public QWidget
{
        Q_OBJECT
    public:
        explicit OutputBox(QSplitter* splitter, QWidget *parent = nullptr);

    protected:
        virtual QSize sizeHint() const override;

    private:
        QSplitter* _splitter;
        OutputBoxPrivate* _d;

};

#endif // OUTPUTBOX_H
