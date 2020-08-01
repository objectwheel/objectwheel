#ifndef INACTIVITYWATCHER_H
#define INACTIVITYWATCHER_H

#include <QTimer>

class InactivityWatcher final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(InactivityWatcher)

    friend class ApplicationCore;

public:
    static InactivityWatcher* instance();
    static int secsLimit();
    static void activate(bool active = true);

private slots:
    void onTimeout();

signals:
    void activated();
    void deactivated();

private:
    explicit InactivityWatcher(int secsLimit, QObject* parent = nullptr);
    ~InactivityWatcher() override;

private:
    static InactivityWatcher* s_instance;
    static bool s_deactivated;
    static int s_secsLimit;
    static QTimer s_inactivityTimer;
};

#endif // INACTIVITYWATCHER_H
