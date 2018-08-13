#ifndef WFW_H
#define WFW_H

#include <QWidget>

inline QWindow* wfw(const QWidget* w) // Window from widget
{
    Q_ASSERT(w);
    QWindow* winHandle = w->windowHandle();
    if (!winHandle) {
        if (const QWidget* nativeParent = w->nativeParentWidget())
            winHandle = nativeParent->windowHandle();
    }
    Q_ASSERT(winHandle);
    return winHandle;
}

#endif // WFW_H
