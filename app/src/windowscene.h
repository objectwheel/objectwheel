#ifndef WINDOWSCENE_H
#define WINDOWSCENE_H

#include <control.h>
#include <controlsscrollpanel.h>
#include <QGraphicsScene>
#include <QPointer>

class WindowScenePrivate;

class WindowScene : public QGraphicsScene
{
        Q_OBJECT
        Q_DISABLE_COPY(WindowScene)
        friend class WindowScenePrivate;

    public:
        explicit WindowScene(QObject *parent = Q_NULLPTR);
        static WindowScene* instance();

        static const QList<Window*>& windows();
        static void addWindow(Window* window);
        static void removeWindow(Window* window);

        static Window* currentWindow();
        static void setCurrentWindow(Window* currentWindow);

        static void removeControl(Control* control);
        static void removeChildControlsOnly(Control* parent);
        static QList<Control*> controls(Qt::SortOrder order = Qt::DescendingOrder);
        static QList<Control*> selectedControls();

        static bool showOutlines();
        static void setShowOutlines(bool value);

        static bool snapping();
        static void setSnapping(bool snapping);

        static QPointF lastMousePos();

        static ControlsScrollPanel* nonGuiControlsPanel();

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
        void drawForeground(QPainter *painter, const QRectF &rect) override;

    private:
        static WindowScenePrivate* _d;
        static QList<Window*> _windows;
        static QPointer<Window> _currentWindow;
        static bool _snapping;
        static QPointF _lastMousePos;
        static ControlsScrollPanel _nonGuiControlsPanel;

};

#endif // WINDOWSCENE_H
