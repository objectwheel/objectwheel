#ifndef CONTROLWATCHER_H
#define CONTROLWATCHER_H

#include <QObject>

class Form;
class Control;

class ControlWatcher : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(ControlWatcher)

    public:
        static ControlWatcher* instance();

    signals:
        void skinChanged(Form*);
        void zValueChanged(Control*);
        void geometryChanged(Control*);
        void previewChanged(Control*);
        void parentChanged(Control*);
        void errorOccurred(Control*);
        void doubleClicked(Control*);
        void controlDropped(Control*, const QPointF&, const QString&);

    private:
        ControlWatcher() {}
};

#endif // CONTROLWATCHER_H
