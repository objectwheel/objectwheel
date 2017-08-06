#ifndef DESIGNMANAGER_H
#define DESIGNMANAGER_H

#include <windowscene.h>
#include <QObject>

class QWidget;
class DesignManagerPrivate;

class DesignManager : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(DesignManager)
        friend class DesignManagerPrivate;

    public:
        enum Mode {
            ControlGUI,
            WindowGUI,
            CodeEdit
        };

    public:
        explicit DesignManager(QObject *parent = 0);
        static DesignManager* instance();
        static void setSettleWidget(QWidget* widget);
        static const Mode& mode();
        static void setMode(const Mode& mode);
        static ControlScene* currentScene();
        static ControlScene* controlScene();
        static WindowScene* windowScene();

    signals:
        void modeChanged();

    private:
        static DesignManagerPrivate* _d;
        static Mode _mode;
        static ControlScene* _currentScene;
};

#endif // DESIGNMANAGER_H
