#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <QWidget>

class QTimer;

class Countdown : public QWidget
{
        Q_OBJECT

    public:
        struct Settings {
            /* Colors */
            QColor borderColor;
            QColor backgroundColor;
            QColor textColor;
            QColor leftDigitColor;
            QColor rightDigitColor;
            QColor dotColor;
            QColor screwColor;
            QColor lineColor;

            /* Sizes */
            qreal margins;
            qreal digitRadius;
            qreal borderRadius;
        };

    public:
        explicit Countdown(QWidget *parent = nullptr);
        Settings& settings();

    public slots:
        void stop();
        void decrease();
        void start(int sec = 0);

    protected:
        QSize sizeHint() const override;
        void paintEvent(QPaintEvent *event) override;

    signals:
        void stopped();
        void finished();

    private:
        int _second;
        QTimer* _timer;
        Settings _settings;
};

#endif // COUNTDOWN_H