#ifndef SWITCH_H
#define SWITCH_H

#include <QEasingCurve>
#include <QAbstractButton>

class QVariantAnimation;

class Switch : public QAbstractButton
{
        Q_OBJECT

    public:
        struct Settings {
            /* Colors */
            QColor activeBorderColor;
            QColor activeBackgroundColor;
            QColor inactiveBorderColor;
            QColor inactiveBackgroundColor;
            QColor handleBorderColor;
            QColor handleColor;

            /* Animation settings */
            int animationDuration;
            QEasingCurve::Type animationType;
        };

    public:
        explicit Switch(QWidget* parent = nullptr);
        Settings& settings();

    public slots:
        void triggerSettings();

    protected:
        QSize sizeHint() const override;
        void paintEvent(QPaintEvent* event) override;

    private slots:
        void handleStateChange();

    private:
        Settings _settings;
        QVariantAnimation* _animation;
};

#endif // SWITCH_H