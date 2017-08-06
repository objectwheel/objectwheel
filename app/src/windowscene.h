#ifndef WINDOWSCENE_H
#define WINDOWSCENE_H

#include <controlscene.h>

class WindowScene : public ControlScene
{
        Q_OBJECT

    public:
        explicit WindowScene(QObject *parent = Q_NULLPTR);
        const QList<Window*>& windows() const { return _windows; }
        void addWindow(Window* window);
        void removeWindow(Window* window);
        Window* mainWindow();
        void setMainWindow(Window* mainWindow);
        void setMainControl(Control* mainControl) override;

    private:
        QList<Window*> _windows;
};

#endif // WINDOWSCENE_H
