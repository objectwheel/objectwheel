#ifndef LOADINGINDICATOR_H
#define LOADINGINDICATOR_H

#include <QLabel>

class LoadingIndicator : public QWidget
{
        Q_OBJECT
    public:
        explicit LoadingIndicator(QWidget *parent = 0);
        static bool running();
        static void setRunning(bool running);

    protected:
        virtual QSize sizeHint() const override;
        virtual void paintEvent(QPaintEvent *event) override;

    private:
        static bool _running;
};

#endif // LOADINGINDICATOR_H
