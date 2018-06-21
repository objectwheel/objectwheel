#ifndef CONTROLMONITORINGMANAGER_H
#define CONTROLMONITORINGMANAGER_H

#include <QObject>

class Form;
class Control;

class ControlMonitoringManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlMonitoringManager)

    friend class ApplicationCore;

public:
    static ControlMonitoringManager* instance();

signals:
    void zValueChanged(Control*);
    void geometryChanged(Control*); // Suppressed
    void previewChanged(Control*);
    void parentChanged(Control*);
    void errorOccurred(Control*);
    void doubleClicked(Control*);
    void idChanged(Control*, const QString& previousId);
    void controlDropped(Control*, const QPointF&, const QString&);

private:
    explicit ControlMonitoringManager(QObject* parent = nullptr);
    ~ControlMonitoringManager();

private:
    static ControlMonitoringManager* s_instance;
};

#endif // CONTROLMONITORINGMANAGER_H
