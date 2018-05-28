#ifndef CONTROLWATCHER_H
#define CONTROLWATCHER_H

#include <QObject>

class Form;
class Control;

class ControlWatcher final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlWatcher)

    friend class BackendManager;

public:
    static ControlWatcher* instance();

signals:
    void zValueChanged(Control*);
    void geometryChanged(Control*); // Suppressed
    void previewChanged(Control*);
    void parentChanged(Control*);
    void errorOccurred(Control*);
    void doubleClicked(Control*);
    void controlDropped(Control*, const QPointF&, const QString&);

private:
    explicit ControlWatcher(QObject* parent = nullptr);
    ~ControlWatcher();

private:
    static ControlWatcher* s_instance;
};

#define cW ControlWatcher::instance()

#endif // CONTROLWATCHER_H
