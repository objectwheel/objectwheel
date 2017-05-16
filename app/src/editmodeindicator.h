#ifndef EDITMODEINDICATOR_H
#define EDITMODEINDICATOR_H

#include <QWidget>
#include <QSize>

class EditModeIndicator : public QWidget
{
        Q_OBJECT

    private:
        bool m_on = true;

    protected:
        void paintEvent(QPaintEvent *event);

    public:
        explicit EditModeIndicator(QWidget *parent = 0);

    public slots:
        inline virtual void on() { m_on = true; }
        inline virtual void off() { m_on = false; }
        inline virtual void setOn(bool on) { m_on = on; }
};

#endif // EDITMODEINDICATOR_H
