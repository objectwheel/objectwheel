#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <QWidget>

class QTimer;
class Countdown final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(Countdown)

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
    explicit Countdown(QWidget* parent = nullptr);
    bool hasExpired() const;
    Settings& settings();

public slots:
    void stop();
    void decrease();
    void start(int sec = 0);

private:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent* event) override;

signals:
    void stopped();
    void finished();

private:
    int m_seconds;
    QTimer* m_timer;
    Settings m_settings;
};

#endif // COUNTDOWN_H
