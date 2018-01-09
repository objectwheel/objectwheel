#ifndef SWITCH_H
#define SWITCH_H

#include <QAbstractButton>

class QVariantAnimation;

class Switch : public QAbstractButton
{
        Q_OBJECT

    public:
        explicit Switch(QWidget *parent = nullptr);

    protected:
        virtual void paintEvent(QPaintEvent* event) override;

    private slots:
        void handleStateChange();

    private:
        QVariantAnimation* _animation;
};

#endif // SWITCH_H