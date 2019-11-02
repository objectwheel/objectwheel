#ifndef MODEMANAGER_H
#define MODEMANAGER_H

#include <QObject>

class ModeManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ModeManager)

    friend class ApplicationCore;

public:
    enum Mode {
        Designer,
        Editor,
        Split,
        Options,
        Builds,
        Documents
    };

public:
    static ModeManager* instance();
    static Mode mode();
    static void setMode(Mode mode);

signals:
    void modeChanged(Mode mode);

private:
    explicit ModeManager(QObject* parent = nullptr);
    ~ModeManager() override;

private:
    static ModeManager* s_instance;
    static Mode s_mode;
};

#endif // MODEMANAGER_H
