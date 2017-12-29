#ifndef LOADINGINDICATOR_H
#define LOADINGINDICATOR_H

#include <QWidget>

class LoadingIndicator : public QWidget
{
        Q_OBJECT

    public:
        explicit LoadingIndicator(QWidget *parent = 0);

        bool running() const;
        void setRunning(bool running);
        void setImage(const QImage& image);

    public slots:
        void start();
        void stop();

    protected:
        void paintEvent(QPaintEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;

    private:
        bool _running;
        QTimer* _timer;
        QImage _image, _scaled;
};

#endif // LOADINGINDICATOR_H
