#ifndef CONTROLWATCHER_H
#define CONTROLWATCHER_H

#include <QObject>

class Control;

class ControlWatcher : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(ControlWatcher)

    public:
        static ControlWatcher* instance();

    signals:
        void zValueChanged(Control*);
        void geometryChanged(Control*);
        void previewChanged(Control*);
        void errorOccurred(Control*);
        void doubleClicked(Control*);

    private:
        ControlWatcher() {}
};

#endif // CONTROLWATCHER_H
