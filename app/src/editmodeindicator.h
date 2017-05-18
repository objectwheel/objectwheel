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
        inline virtual void on() { m_on = true; update(); }
        inline virtual void off() { m_on = false; update(); }
        inline virtual void setOn(bool on) { m_on = on; update(); }
};

#endif // EDITMODEINDICATOR_H
